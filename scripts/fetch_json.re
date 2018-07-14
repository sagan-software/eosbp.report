[%raw "global.XMLHttpRequest = require('xhr2').XMLHttpRequest"];
[%raw "process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0'"];

module Log = NpmLog;

[@bs.module "mkdirp"]
external mkdirpSync : string => unit = "sync";

/* let consoleTransport =
     Log.(console(~format=Format.(combine([|metadata(), cli(), simple()|]))));

   let log = Log.(make(~level="debug", ~transports=[|consoleTransport|])); */

let httpEndpoint = "http://node2.liquideos.com";

let writeBpJson = (row: EosBp_Table.Row.t, xhr, json) => {
  let dirname = Node.Path.join([|Env.buildDir, row.owner|]);
  let text = xhr |. Xhr.getResponseText |> Js.Option.getWithDefault("");
  mkdirpSync(dirname);
  Node.Fs.writeFileAsUtf8Sync(
    Node.Path.join([|dirname, "bp-raw.json"|]),
    text,
  );
  Node.Fs.writeFileAsUtf8Sync(
    Node.Path.join([|dirname, "bp.json"|]),
    json |. Js.Json.stringifyWithSpace(2),
  );
  Log.info(
    "write bp.json",
    "wrote files",
    Node.Path.relative(~from=Node.Process.cwd(), ~to_=dirname, ()),
  );
};

let fetchBpJson = row =>
  Js.Promise.(
    EosBp.Fetch.Response.(
      EosBp.Fetch.bpJsonRaw(~row, ())
      |> then_(result => {
           let label = "fetch bp.json";
           let metadata = (url, xhr) => {
             "producer": row.owner,
             "row_url": row.url,
             "request_url": url,
             "response_url": xhr |. Xhr.getResponseUrl,
             "response_type": xhr |. Xhr.getResponseHeader("Content-Type"),
           };
           switch (result) {
           | ValidJson4(url, xhr, json) =>
             Log.info(label, "Valid JSON4", metadata(url, xhr));
             resolve(Some((row, xhr, json)));
           | ValidJson5(url, xhr, json) =>
             Log.warn(label, "Valid JSON5", metadata(url, xhr));
             resolve(Some((row, xhr, json)));
           | InvalidJson(url, xhr, _text) =>
             Log.error(label, "Invalid JSON", metadata(url, xhr));
             resolve(None);
           | BadResponse(url, xhr) =>
             Log.error(label, "Bad response", metadata(url, xhr));
             resolve(None);
           | BadUrl(url) =>
             Log.error(
               label,
               "Bad URL",
               {"producer": row.owner, "url": url},
             );
             resolve(None);
           | TimedOut(url, xhr) =>
             Log.error(label, "Timed out", metadata(url, xhr));
             resolve(None);
           | Unreachable(url, xhr) =>
             Log.error(label, "Unreachable", metadata(url, xhr));
             resolve(None);
           | UnknownError(url, e) =>
             Log.error(
               label,
               "Unknown error",
               {"producer": row.owner, "url": url, "error": e},
             );
             resolve(None);
           };
         })
    )
  );

Js.Promise.(
  EosBp.Fetch.tableRows(~httpEndpoint, ())
  |> then_(response => {
       let rows =
         response
         |> EosBp.Fetch.Response.getData
         |> Js.Option.getWithDefault([||]);
       Js.log2("got table rows", rows |> Js.Array.length);
       rows |> Js.Array.map(fetchBpJson) |> all;
     })
  |> then_(results =>
       results
       |> Js.Array.forEach(result =>
            switch (result) {
            | Some((row, xhr, json)) => writeBpJson(row, xhr, json)
            | None => ()
            }
          )
       |> resolve
     )
  |> then_(_results => {
       Log.info("", "Done", Env.buildDir);
       /* Node.Process.exit(0); */
       resolve();
     })
);
