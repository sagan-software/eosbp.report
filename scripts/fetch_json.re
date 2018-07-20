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

let httpEndpoint = "http://api.eosnewyork.io";
let eos = Eos.make(~httpEndpoint, ());

let rec tableRows = (~lowerBound=?, ()) =>
  eos
  |. Eosio.getProducers(~lowerBound?, ~limit=100, ())
  |> Js.Promise.then_((table: Eos.TableRows.t(Eosio.System.ProducerInfo.t)) => {
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
               |. Eos.AccountName.toString
               |. encodeName(false)
               |> BigNumber.fromString
               |. BigNumber.plusInt(1)
               |> BigNumber.toString;
             tableRows(~lowerBound=nextLowerBound, ())
             |> Js.Promise.then_(
                  (t2: Eos.TableRows.t(Eosio.System.ProducerInfo.t)) => {
                  let newTable: Eos.TableRows.t(Eosio.System.ProducerInfo.t) = {
                    rows: Js.Array.concat(table.rows, t2.rows),
                    more: t2.more,
                  };
                  Js.Promise.resolve(newTable);
                });
           | None => Js.Promise.resolve(table)
           };
         } :
         Js.Promise.resolve(table);
     });

let bpJsonRaw = (row: Eosio.System.ProducerInfo.t) =>
  row.url
  |> Js.String.replaceByRe([%bs.re "/\\W/g"], "")
  |> Js.String.trim
  |> Js.String.length > 0 ?
    {
      let url = row.url |> EosBp_Json.normalizeBpJsonUrl;
      fetch(~url, ());
    } :
    Js.Promise.reject(BadUrl(row.url));

let bpJson = row => row |> bpJsonRaw |> thenDecode(EosBp_Json.decode);

[@bs.module "mkdirp"] external mkdirpSync : string => unit = "sync";

let producerDir = (row: Eosio.System.ProducerInfo.t) =>
  Node.Path.join([|Env.buildDir, row.owner |. Eos.AccountName.toString|]);

let writeProducerFile =
    (~row: Eosio.System.ProducerInfo.t, ~filename, ~contents, ~mode) => {
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

let writeBpJson = ((response, data, row: Eosio.System.ProducerInfo.t)) => {
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
         Log.info(
           "bp.json",
           row.owner |. Eos.AccountName.toString,
           response.responseUrl,
         );
         Js.Promise.resolve(Some((response, data, row)));
       } else {
         Js.Promise.reject(BadStatus(response));
       }
     )
  |> Js.Promise.catch(error => {
       switch (handleBpJsonError(error)) {
       | Some(message) =>
         Log.error("bp.json", row.owner |. Eos.AccountName.toString, message)
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

let fetchBpJsonFiles = (table: Eos.TableRows.t(Eosio.System.ProducerInfo.t)) => {
  Log.info("regproducer", "total", table.rows |> Js.Array.length);
  table.rows
  |. allChunked(fetchBpJson, 25)
  |> Js.Promise.then_(responses =>
       responses |> withoutNone |> Js.Promise.resolve
     )
  |> Js.Promise.then_(responses =>
       Js.Promise.resolve((table.rows, responses))
     );
};

let writeBpJsonFiles = ((rows, responses)) => {
  let numRows = rows |> Js.Array.length;
  let numResponses = responses |> Js.Array.length;
  Log.info(
    "fetch done",
    {j|Got $numResponses OK responses of $numRows producers|j},
    "",
  );
  responses
  |. allChunked(writeBpJson, 10)
  |> Js.Promise.then_(_ => Js.Promise.resolve((rows, responses)));
};

let renderHtml = (~element) => {
  let content = ReactDOMServerRe.renderToString(element);
  let helmet = Helmet.renderStatic();
  let bodyAttributes = helmet |. Helmet.bodyAttributesGet |> Helmet.toString;
  let htmlAttributes = helmet |. Helmet.htmlAttributesGet |> Helmet.toString;
  let style = helmet |. Helmet.styleGet |> Helmet.toString;
  let title = helmet |. Helmet.titleGet |> Helmet.toString;
  let meta = helmet |. Helmet.metaGet |> Helmet.toString;
  let script = helmet |. Helmet.scriptGet |> Helmet.toString;
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
    (row: Eosio.System.ProducerInfo.t, basename: string, url: option(string)) => {
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
         | Some(message) =>
           Log.error("image", row.owner |. Eos.AccountName.toString, message)
         | None => ()
         };
         Js.Promise.resolve();
       });
  };
};

let fetchImages = (row: Eosio.System.ProducerInfo.t, json: EosBp.Json.t) =>
  json.org.branding
  |> Js.Option.map((. branding: EosBp.Json.Org.Branding.t) =>
       Js.Promise.all([|
         fetchImage(row, "logo_256", branding.logo256),
         fetchImage(row, "logo_1024", branding.logo1024),
         fetchImage(row, "logo", branding.logoSvg),
       |])
     )
  |> Js.Option.getWithDefault(Js.Promise.resolve([||]));

let fetchAllImages = ((rows, responses)) =>
  responses
  |> Js.Array.map(((_response, data, row)) =>
       switch (data.decoded) {
       | Belt.Result.Ok(json) => fetchImages(row, json)
       | _ => Js.Promise.resolve([||])
       }
     )
  |. allChunked(p => p, 5)
  |> Js.Promise.then_(_ => Js.Promise.resolve((rows, responses)));

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

let generateProducerHtmlFile = (row: Eosio.System.ProducerInfo.t) =>
  generateHtmlFile(
    ~route=Route.Producer(row.owner |. Eos.AccountName.toString),
    ~dirname=producerDir(row),
  );

let generateProducerHtmlFiles = ((rows, responses)) =>
  rows
  |. allChunked(generateProducerHtmlFile, 10)
  |> Js.Promise.then_(_ => Js.Promise.resolve((rows, responses)));

let generateProducerReport =
    (
      producer: Eosio.System.ProducerInfo.t,
      globalState: Eosio.System.GlobalState.t,
    ) => {
  let estimatedRewards =
    Eosio.System.estimateProducerPay(producer, globalState);
  Js.Promise.resolve((producer, estimatedRewards));
};

let generateProducerReports = ((rows, responses)) =>
  eos
  |. Eosio.getGlobalState()
  |> Js.Promise.then_((globalState: option(Eosio.System.GlobalState.t)) =>
       switch (globalState) {
       | Some(globalState) =>
         rows
         |. Belt.Array.map((r: Eosio.System.ProducerInfo.t) =>
              generateProducerReport(r, globalState)
            )
         |. Js.Promise.all
         |> Js.Promise.then_(results =>
              results
              |> Js.Array.sortInPlaceWith(((_, a), (_, b)) =>
                   a |. BigNumber.gt(b) ? 1 : (-1)
                 )
              |. Belt.Array.forEach(
                   (
                     (
                       producer: Eosio.System.ProducerInfo.t,
                       estimatedRewards: BigNumber.t,
                     ),
                   ) =>
                   Log.info(
                     "rewards",
                     producer.owner |. Eos.AccountName.toString,
                     estimatedRewards
                     |. Eos.Asset.fromBigNumber(~precision=4, ~symbol="EOS")
                     |. Eos.Asset.toString,
                   )
                 )
              |. Js.Promise.resolve
            )
       | None =>
         Log.error(
           "reports",
           "No global state found in the eosio.system contract",
           "",
         );
         Js.Promise.resolve();
       }
     )
  |> Js.Promise.then_(_results => Js.Promise.resolve((rows, responses)));

let generateNodesJson = ((rows, responses)) =>
  responses
  |. Belt.Array.reduce(
       [||], (result, (_, data, row: Eosio.System.ProducerInfo.t)) =>
       switch (data.decoded) {
       | Ok((bpJson: EosBp_Json.t)) =>
         bpJson.nodes
         |. Belt.Array.reduce(
              [||],
              (result, node) => {
                let apiEndpoint =
                  node.apiEndpoint
                  |. Belt.Option.getWithDefault("")
                  |. String.trim;
                if (apiEndpoint |. String.length > 0) {
                  result
                  |> Js.Array.push((
                       row.owner,
                       node,
                       apiEndpoint |. EosBp_Json.normalizeUrl,
                     ))
                  |> ignore;
                };
                let sslEndpoint =
                  node.sslEndpoint
                  |. Belt.Option.getWithDefault("")
                  |. String.trim;
                if (sslEndpoint |. String.length > 0) {
                  result
                  |> Js.Array.push((
                       row.owner,
                       node,
                       sslEndpoint |. EosBp_Json.normalizeUrl,
                     ))
                  |> ignore;
                };
                result;
              },
            )
         |. Belt.Array.reduce(
              [||],
              (result, (producer, node: EosBp_Json.Node.t, url)) => {
                switch (URL.make(url)) {
                | url =>
                  let report: EosBp_Report.Node.t = {
                    producer,
                    url,
                    latitude: node.location.latitude,
                    longitude: node.location.longitude,
                    nodeType:
                      node.nodeType |. Belt.Option.getWithDefault([||]),
                    infoStatus: None,
                    serverVersion: None,
                    chainId: None,
                    headBlockNum: None,
                    headBlockTime: None,
                    lastIrreversibleBlockNum: None,
                  };
                  result |> Js.Array.push(report) |> ignore;
                | exception _error => ()
                };
                result;
              },
            )
         |. Belt.Array.map((report: EosBp_Report.Node.t) =>
              Request.make(
                ~url=
                  URL.makeWithBase(
                    "/v1/chain/get_info",
                    report.url |. URL.origin,
                  )
                  |. URL.toString,
                ~json=false,
                ~timeout=10000,
                (),
              )
              |> Js.Promise.then_(response =>
                   response
                   |. Request.body
                   |> Util.parseAndDecodeAsPromise(Eos.Info.decode)
                   |> Js.Promise.then_((info: Eos.Info.t) =>
                        {
                          ...report,
                          infoStatus: Some(response |. Request.statusCode),
                          serverVersion: Some(info.serverVersion),
                          chainId: Some(info.chainId),
                          headBlockNum: Some(info.headBlockNum),
                          headBlockTime: Some(info.headBlockTime),
                          lastIrreversibleBlockNum:
                            Some(info.lastIrreversibleBlockNum),
                        }
                        |. Js.Promise.resolve
                      )
                   |> Js.Promise.catch(_ =>
                        {
                          ...report,
                          infoStatus: Some(response |. Request.statusCode),
                        }
                        |. Js.Promise.resolve
                      )
                 )
              |> Js.Promise.catch(_ => Js.Promise.resolve(report))
            )
         |. Belt.Array.concat(result)
       | Error(_error) => result
       }
     )
  |> Js.Promise.all
  |> Js.Promise.then_(reports => {
       let dirname = Env.buildDir;
       let fullpath = Node.Path.join([|dirname, "nodes.json"|]);
       let contents =
         reports
         |> Js.Array.sortInPlaceWith(
              (a: EosBp_Report.Node.t, b: EosBp_Report.Node.t) =>
              compare(
                (a.producer |. Eos.AccountName.toString)
                ++ (a.url |. URL.toString),
                (b.producer |. Eos.AccountName.toString)
                ++ (b.url |. URL.toString),
              )
            )
         |. Belt.Array.map(EosBp_Report.Node.encode)
         |. Json.Encode.jsonArray
         |. Js.Json.stringifyWithSpace(2);
       let mode = `utf8;
       Node.Fs.writeFileSync(fullpath, contents, mode);
       Log.info(
         "write",
         "nodes.json",
         Node.Path.relative(~from=Node.Process.cwd(), ~to_=fullpath, ()),
       );
       Js.Promise.resolve();
     })
  |> Js.Promise.then_(_results => Js.Promise.resolve((rows, responses)));

Js.Promise.(
  tableRows()
  |> then_(fetchBpJsonFiles)
  |> then_(writeBpJsonFiles)
  |> then_(generateProducerHtmlFiles)
  |> then_(fetchAllImages)
  |> then_(generateProducerReports)
  |> then_(generateNodesJson)
  |> then_(_results => {
       Log.info("", "Done", Env.buildDir);
       resolve();
     })
  |> catch(error => {
       Log.error("err", "", error);
       resolve();
     })
);
