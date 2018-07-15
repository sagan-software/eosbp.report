[%raw "process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0'"];

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

let tableRowsRaw = httpEndpoint => {
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
        ("limit", int(5000)),
      ]
      |> object_
      |> Js.Json.stringify
    );
  fetchWithBase(~baseUrl, ~path, ~method_, ~body, ());
};

let tableRows = httpEndpoint =>
  httpEndpoint |> tableRowsRaw |> thenDecode(EosBp_Table.decode);

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

module Log = NpmLog;

[@bs.module "mkdirp"] external mkdirpSync : string => unit = "sync";

let httpEndpoint = "http://node2.liquideos.com";

let writeBpJson = ((response, data, row: EosBp.Table.Row.t)) => {
  let dirname = Node.Path.join([|Env.buildDir, row.owner|]);
  mkdirpSync(dirname);
  Node.Fs.writeFileAsUtf8Sync(
    Node.Path.join([|dirname, "bp-raw.json"|]),
    response.text,
  );
  Node.Fs.writeFileAsUtf8Sync(
    Node.Path.join([|dirname, "bp.json"|]),
    data.json |. Js.Json.stringifyWithSpace(2),
  );
  Log.info(
    "write",
    row.owner,
    Node.Path.relative(~from=Node.Process.cwd(), ~to_=dirname, ()),
  );
  Js.Promise.resolve();
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
  );

let fetchBpJson = row =>
  row
  |> bpJson
  |> Js.Promise.then_(((response, data)) =>
       if (200 <= response.statusCode && response.statusCode < 400) {
         Log.info("fetch", row.owner, response.responseUrl);
         Js.Promise.resolve(Some((response, data, row)));
       } else {
         Js.Promise.reject(BadStatus(response));
       }
     )
  |> Js.Promise.catch(error => {
       switch (handleBpJsonError(error)) {
       | Some(message) => Log.error("fetch", row.owner, message)
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

Js.Promise.(
  httpEndpoint
  |> tableRows
  |> requireDecoded
  |> then_(((_response, _data, table: EosBp.Table.t)) => {
       let total = table.rows |> Js.Array.length;
       let more = table.more;
       Log.info("regproducer", {j|total=$total more=$more|j}, "");
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
       responses |. allChunked(writeBpJson, 10);
     })
  |> then_(_results => {
       Log.info("", "Done", Env.buildDir);
       resolve();
     })
  |> catch(error => {
       Log.error("err", "", error);
       resolve();
     })
);
