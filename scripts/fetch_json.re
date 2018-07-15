[%raw "process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0'"];

module Log = NpmLog;

type data('data) = {
  json: Js.Json.t,
  isJson5: bool,
  decoded: Belt.Result.t('data, string),
};

type response = {
  requestUrl: string,
  responseUrl: string,
  statusCode: int,
  statusText: string,
  contentType: option(string),
  text: string,
};

exception BadUrl(string);

let fetchWithUrl = (~url, ~method_=?, ~body=?, ~timeout=5000, ()) => {
  let urlStr = url |. URL.toString;

  Request.make(~url=urlStr, ~method_?, ~body?, ~json=false, ~timeout, ())
  |> Js.Promise.then_((res: Request.t) =>
       {
         requestUrl: urlStr,
         responseUrl: res |. Request.url |. URL.href,
         statusCode: res |. Request.statusCode,
         statusText: res |. Request.statusText,
         contentType: res |. Request.header("Content-Type"),
         text: res |. Request.body,
       }
       |> Js.Promise.resolve
     );
};

let fetchWithBase = (~baseUrl, ~path, ~method_=?, ~body=?, ~timeout=?, ()) =>
  switch (URL.makeWithBase(path, baseUrl)) {
  | url => fetchWithUrl(~url, ~method_?, ~body?, ~timeout?, ())
  | exception _error => Js.Promise.reject(BadUrl(baseUrl))
  };

let fetch = (~url, ~method_=?, ~body=?, ~timeout=?, ()) =>
  switch (URL.make(url)) {
  | url => fetchWithUrl(~url, ~method_?, ~body?, ~timeout?, ())
  | exception _error => Js.Promise.reject(BadUrl(url))
  };

[@bs.module "json5"] external parseJson5Exn : string => Js.Json.t = "parse";

let decodeAsResult = (decoder, json) =>
  switch (decoder(json)) {
  | decoded => Belt.Result.Ok(decoded)
  | exception (Json.Decode.DecodeError(msg)) => Belt.Result.Error(msg)
  };

let parseData = (decoder, text) =>
  switch (text |> Js.Json.parseExn) {
  | json => {json, isJson5: false, decoded: decodeAsResult(decoder, json)}
  | exception _error =>
    let json = text |> parseJson5Exn;
    {json, isJson5: false, decoded: decodeAsResult(decoder, json)};
  };

exception InvalidJson(response);

let thenDecode = (decoder, promise) =>
  promise
  |> Js.Promise.then_(response =>
       switch (parseData(decoder, response.text)) {
       | data => Js.Promise.resolve((response, data))
       | exception _ => Js.Promise.reject(InvalidJson(response))
       }
     );

exception DecodeError(response, string);

let requireDecoded = promise =>
  promise
  |> Js.Promise.then_(((response, data)) =>
       switch (data.decoded) {
       | Belt.Result.Ok(decoded) =>
         Js.Promise.resolve((response, data, decoded))
       | Belt.Result.Error(message) =>
         Js.Promise.reject(DecodeError(response, message))
       }
     );

[@bs.module "eosjs"] [@bs.scope ("modules", "format")]
external encodeName : (string, bool) => string = "";

module BigNumber = {
  type t;
  [@bs.module] [@bs.new] external fromString : string => t = "bignumber.js";
  [@bs.send] external plusInt : (t, int) => t = "plus";
  [@bs.send] external toString : t => string = "";
};

let tableRowsRaw = (httpEndpoint, ~lowerBound=?, ()) => {
  let method_ = "POST";
  let baseUrl = httpEndpoint;
  let path = "/v1/chain/get_table_rows";
  let body =
    Json.Encode.(
      [
        ("scope", string("eosio")),
        ("code", string("eosio")),
        ("table", string("producers")),
        ("json", bool(true)),
        ("limit", int(100)),
        (
          "lower_bound",
          lowerBound |> Js.Option.getWithDefault("") |> string,
        ),
      ]
      |> object_
      |> Js.Json.stringify
    );
  fetchWithBase(~baseUrl, ~path, ~method_, ~body, ());
};

let rec tableRows = (httpEndpoint, ~lowerBound=?, ()) =>
  tableRowsRaw(httpEndpoint, ~lowerBound?, ())
  |> thenDecode(EosBp_Table.decode)
  |> requireDecoded
  |> Js.Promise.then_(((response, data, table: EosBp.Table.t)) => {
       let total = table.rows |> Js.Array.length;
       let more = table.more;
       Log.info("regproducer", {j|total=$total more=$more|j}, "");
       more ?
         {
           let lastIndex = Js.Array.length(table.rows) - 1;
           let lastRow = Belt.Array.get(table.rows, lastIndex);
           switch (lastRow) {
           | Some(row) =>
             let nextLowerBound =
               row.owner
               |. encodeName(false)
               |> BigNumber.fromString
               |. BigNumber.plusInt(1)
               |> BigNumber.toString;
             tableRows(httpEndpoint, ~lowerBound=nextLowerBound, ())
             |> Js.Promise.then_(((r2, d2, t2: EosBp.Table.t)) => {
                  let newTable: EosBp.Table.t = {
                    rows: Js.Array.concat(table.rows, t2.rows),
                    more: t2.more,
                  };
                  Js.Promise.resolve((r2, d2, newTable));
                });
           | None => Js.Promise.resolve((response, data, table))
           };
         } :
         Js.Promise.resolve((response, data, table));
     });

let bpJsonRaw = (row: EosBp_Table.Row.t) =>
  row.url
  |> Js.String.replaceByRe([%bs.re "/\\W/g"], "")
  |> Js.String.trim
  |> Js.String.length > 0 ?
    {
      let url = row |> EosBp_Table.Row.jsonUrl;
      fetch(~url, ());
    } :
    Js.Promise.reject(BadUrl(row.url));

let bpJson = row => row |> bpJsonRaw |> thenDecode(EosBp_Json.decode);

[@bs.module "mkdirp"] external mkdirpSync : string => unit = "sync";

let httpEndpoint = "http://node2.liquideos.com";

let producerDir = (row: EosBp.Table.Row.t) =>
  Node.Path.join([|Env.buildDir, row.owner|]);

let writeProducerFile = (~row: EosBp.Table.Row.t, ~filename, ~contents, ~mode) => {
  let dirname = producerDir(row);
  let fullpath = Node.Path.join([|dirname, filename|]);
  mkdirpSync(dirname);
  Node.Fs.writeFileSync(fullpath, contents, mode);
  Log.info(
    "write",
    filename,
    Node.Path.relative(~from=Node.Process.cwd(), ~to_=fullpath, ()),
  );
  Js.Promise.resolve();
};

let writeBpJson = ((response, data, row: EosBp.Table.Row.t)) => {
  let writeBpRawJson =
    writeProducerFile(
      ~row,
      ~filename="bp-raw.json",
      ~contents=response.text,
      ~mode=`utf8,
    );
  let writeBpJson =
    writeProducerFile(
      ~row,
      ~filename="bp.json",
      ~contents=data.json |. Js.Json.stringifyWithSpace(2),
      ~mode=`utf8,
    );
  Js.Promise.all2((writeBpRawJson, writeBpJson));
};

let chunks = (size, originalArr) => {
  let results = [||];
  let arr = Js.Array.copy(originalArr);
  while (arr |> Js.Array.length > 0) {
    let chunk =
      arr |> Js.Array.spliceInPlace(~pos=0, ~remove=size, ~add=[||]);
    results |> Js.Array.push(chunk) |> ignore;
  };
  results;
};

let allChunked = (arr: array('a), fn: 'a => 'b, chunkSize) =>
  Js.Array.reduce(
    (promise, chunk) =>
      promise
      |> Js.Promise.then_(allResults
           /* Log.info("chunk", "processing chunk...", chunk |> Js.Array.length); */
           =>
             chunk
             |> Js.Array.map(fn)
             |> Js.Promise.all
             |> Js.Promise.then_(results =>
                  Js.Promise.resolve(Js.Array.concat(allResults, results))
                )
           ),
    Js.Promise.resolve([||]),
    chunks(chunkSize, arr),
  );

exception Unreachable(string);
exception BadResponse(response);
exception BadStatus(response);

let handleBpJsonError =
  [@bs.open]
  (
    fun
    | BadUrl(url) => {j|Bad URL: "$url"|j}
    | BadStatus({responseUrl, statusCode, statusText}) => {j|Bad status: $statusCode ($statusText) at $responseUrl|j}
    | Request.TimedOut(url) => {j|Timed out: $url |j}
  );

let fetchBpJson = row =>
  row
  |> bpJson
  |> Js.Promise.then_(((response, data)) =>
       if (200 <= response.statusCode && response.statusCode < 400) {
         Log.info("bp.json", row.owner, response.responseUrl);
         Js.Promise.resolve(Some((response, data, row)));
       } else {
         Js.Promise.reject(BadStatus(response));
       }
     )
  |> Js.Promise.catch(error => {
       switch (handleBpJsonError(error)) {
       | Some(message) => Log.error("bp.json", row.owner, message)
       | None => ()
       };
       Js.Promise.resolve(None);
     });

let withoutNone = optsArray =>
  Js.Array.reduce(
    (results, item) => {
      switch (item) {
      | Some(item) => Js.Array.push(item, results) |> ignore
      | None => ()
      };
      results;
    },
    [||],
    optsArray,
  );

let renderHtml = (~element) => {
  let content = ReactDOMServerRe.renderToString(element);
  let helmet = Helmet.renderStatic();
  let bodyAttributes = helmet |. Helmet.bodyAttributesGet |> Helmet.toString;
  let htmlAttributes = helmet |. Helmet.htmlAttributesGet |> Helmet.toString;
  let style = helmet |. Helmet.styleGet |> Helmet.toString;
  let title = helmet |. Helmet.titleGet |> Helmet.toString;
  let meta = helmet |. Helmet.metaGet |> Helmet.toString;
  let script = helmet |. Helmet.scriptGet |> Helmet.toString;
  let staticUrl = Env.staticUrl;
  {j|<!DOCTYPE html>
    <html $htmlAttributes>
      <head>
        <meta charset="utf-8">
        <meta http-equiv="x-ua-compatible" content="ie=edge">
        $title
        $meta
        $style
      </head>
    <body $bodyAttributes>
      <div id="app">$content</div>
      <script src="/index.js"></script>
      $script
    </body>
    </html>
  |j};
};

let generateHtmlFile = (~route, ~dirname) => {
  let element = <App route />;
  let html = renderHtml(~element);
  let fullpath = Node.Path.join([|dirname, "index.html"|]);
  mkdirpSync(dirname);
  Node.Fs.writeFileAsUtf8Sync(fullpath, html);
  Log.info(
    "write",
    "html",
    Node.Path.relative(~from=Node.Process.cwd(), ~to_=fullpath, ()),
  );
  Js.Promise.resolve();
};

let generateProducerHtmlFile = (row: EosBp.Table.Row.t) =>
  generateHtmlFile(
    ~route=Route.Producer(row.owner),
    ~dirname=producerDir(row),
  );

let imageExtension = contentType =>
  if (contentType |> Js.String.startsWith("image/svg")) {
    Some(".svg");
  } else if (contentType |> Js.String.startsWith("image/png")) {
    Some(".png");
  } else if (contentType |> Js.String.startsWith("image/jpeg")) {
    Some(".jpg");
  } else {
    None;
  };

let fetchImage =
    (row: EosBp.Table.Row.t, basename: string, url: option(string)) => {
  let url =
    url
    |> Js.Option.getWithDefault("")
    |> Js.String.trim
    |> Js.String.replaceByRe([%bs.re "/^[^\\x00-\\x7F]/g"], "")
    |> Js.String.replaceByRe([%bs.re "/[^\\x00-\\x7F]$/g"], "")
    |> Js.String.replaceByRe([%bs.re "/\\/$/g"], "");

  if (url |> Js.String.length === 0) {
    Js.Promise.resolve();
  } else {
    /* Add protocol if missing */
    let url =
      Js.String.startsWith("http", url) || Js.String.startsWith("https", url) ?
        url : "http://" ++ url;

    Request.make(~url, ~timeout=15000, ~encoding=Js.Null_undefined.null, ())
    |> Js.Promise.then_(res => {
         let contentType =
           res
           |. Request.header("content-type")
           |> Js.Option.getWithDefault("?");
         switch (imageExtension(contentType)) {
         | Some(ext) =>
           let filename = basename ++ ext;
           writeProducerFile(
             ~row,
             ~filename,
             ~contents=res |. Request.body,
             ~mode=`binary,
           );
         | None => Js.Promise.resolve()
         };
       })
    |> Js.Promise.catch(error => {
         switch (handleBpJsonError(error)) {
         | Some(message) => Log.error("image", row.owner, message)
         | None => ()
         };
         Js.Promise.resolve();
       });
  };
};

let fetchImages = (row: EosBp.Table.Row.t, json: EosBp.Json.t) =>
  json.org.branding
  |> Js.Option.map((. branding: EosBp.Json.Org.Branding.t) =>
       Js.Promise.all([|
         fetchImage(row, "logo_256", branding.logo256),
         fetchImage(row, "logo_1024", branding.logo1024),
         fetchImage(row, "logo", branding.logoSvg),
       |])
     )
  |> Js.Option.getWithDefault(Js.Promise.resolve([||]));

Js.Promise.(
  httpEndpoint
  |. tableRows()
  |> then_(((_response, _data, table: EosBp.Table.t)) => {
       Log.info("regproducer", "total", table.rows |> Js.Array.length);
       table.rows
       |. allChunked(fetchBpJson, 25)
       |> then_(responses => responses |> withoutNone |> resolve)
       |> then_(responses => resolve((table.rows, responses)));
     })
  |> then_(((rows, responses)) => {
       let numRows = rows |> Js.Array.length;
       let numResponses = responses |> Js.Array.length;
       Log.info(
         "fetch done",
         {j|Got $numResponses OK responses of $numRows producers|j},
         "",
       );
       responses
       |. allChunked(writeBpJson, 10)
       |> then_(_ => resolve((rows, responses)));
     })
  |> then_(((rows, responses)) =>
       rows
       |. allChunked(generateProducerHtmlFile, 10)
       |> then_(_ => resolve((rows, responses)))
     )
  |> then_(((rows, responses)) =>
       responses
       |> Js.Array.map(((_response, data, row)) =>
            switch (data.decoded) {
            | Belt.Result.Ok(json) => fetchImages(row, json)
            | _ => Js.Promise.resolve([||])
            }
          )
       |. allChunked(p => p, 5)
       |> then_(_ => resolve((rows, responses)))
     )
  |> then_(_results => {
       Log.info("", "Done", Env.buildDir);
       resolve();
     })
  |> catch(error => {
       Log.error("err", "", error);
       resolve();
     })
);
