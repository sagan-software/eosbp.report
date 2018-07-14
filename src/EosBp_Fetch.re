module Response = {
  type t('data, 'error) =
    | ValidJson4(string, Xhr.t, 'data)
    | ValidJson5(string, Xhr.t, 'data)
    | InvalidJson(string, Xhr.t, string)
    | BadResponse(string, Xhr.t)
    | BadUrl(string)
    | TimedOut(string, Xhr.t)
    | Unreachable(string, Xhr.t)
    | UnknownError(string, 'error);

  [@bs.module "json5"] external parseJson5Exn : string => Js.Json.t = "parse";

  let parseJson = (url, res, text) =>
    switch (text |> Js.Json.parseExn) {
    | json => ValidJson4(url, res, json)
    | exception _error =>
      switch (text |> parseJson5Exn) {
      | json => ValidJson5(url, res, json)
      | exception _error => InvalidJson(url, res, text)
      }
    };

  let mapJson = (fn, response) =>
    switch (response) {
    | ValidJson4(url, xhr, data) => ValidJson4(url, xhr, data |> fn)
    | ValidJson5(url, xhr, data) => ValidJson5(url, xhr, data |> fn)
    | InvalidJson(url, xhr, text) => InvalidJson(url, xhr, text)
    | BadResponse(url, xhr) => BadResponse(url, xhr)
    | BadUrl(str) => BadUrl(str)
    | TimedOut(url, xhr) => TimedOut(url, xhr)
    | Unreachable(url, xhr) => Unreachable(url, xhr)
    | UnknownError(url, e) => UnknownError(url, e)
    };

  let getData = response =>
    switch (response) {
    | ValidJson4(_url, _res, data) => Some(data)
    | ValidJson5(_url, _res, data) => Some(data)
    | _ => None
    };
};

let headers = [("Content-Type", "application/json")];

let fetchWithUrl =
    (~url, ~method_=`GET, ~headers=headers, ~body=None, ~timeout=15000, ()) =>
  Js.Promise.make((~resolve, ~reject as _) => {
    open Response;
    let urlStr = url |. URL.toString;
    let xhr = Xhr.make(method_, urlStr);
    xhr |. Xhr.setRequestHeaders(headers);
    xhr |. Xhr.setTimeout(timeout);

    let timeoutId =
      Js.Global.setTimeout(() => resolve(. TimedOut(urlStr, xhr)), timeout);
    let clearAndResolve = response => {
      Js.Global.clearTimeout(timeoutId);
      resolve(. response);
    };

    xhr
    |. Xhr.onLoad(_event => {
         let result =
           switch (xhr |. Xhr.getResponseText) {
           | Some(text) => parseJson(urlStr, xhr, text)
           | None => UnknownError(urlStr, "")
           };
         clearAndResolve(result);
       });
    xhr |. Xhr.onTimeout(_event => clearAndResolve(TimedOut(urlStr, xhr)));
    xhr |. Xhr.onAbort(_event => clearAndResolve(TimedOut(urlStr, xhr)));
    xhr |. Xhr.onError(error => clearAndResolve(UnknownError(urlStr, error)));
    switch (body) {
    | Some(json) => xhr |. Xhr.sendJson(json)
    | None => xhr |. Xhr.send
    };
  });

let fetchWithBase =
    (~baseUrl, ~path, ~method_=?, ~headers=?, ~body=?, ~timeout=?, ()) =>
  Js.Promise.(
    switch (URL.makeWithBase(path, baseUrl)) {
    | url => fetchWithUrl(~url, ~method_?, ~headers?, ~body?, ~timeout?, ())
    | exception _error => Response.BadUrl(baseUrl) |> resolve
    }
  );

let fetch = (~url, ~method_=?, ~headers=?, ~body=?, ~timeout=?, ()) =>
  Js.Promise.(
    switch (URL.make(url)) {
    | url => fetchWithUrl(~url, ~method_?, ~headers?, ~body?, ~timeout?, ())
    | exception _error => Response.BadUrl(url) |> resolve
    }
  );

let thenDecode = (decoder, promise) =>
  promise
  |> Js.Promise.then_(response =>
       response |> Response.mapJson(j => j |> decoder) |> Js.Promise.resolve
     )
  |> Js.Promise.catch(e => {
       Js.Console.error2("error decoding JSON", e);
       Response.UnknownError("", "error decoding JSON") |> Js.Promise.resolve;
     });

let tableRowsRaw = (~httpEndpoint, ~headers=?, ()) => {
  let method_ = `POST;
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
      |. Some
    );
  fetchWithBase(~baseUrl, ~path, ~method_, ~body, ~headers?, ());
};

let tableRows = (~httpEndpoint, ~headers=?, ()) =>
  tableRowsRaw(~httpEndpoint, ~headers?, ())
  |> thenDecode(EosBp_Table.decode);

let bpJsonRaw = (~row: EosBp_Table.Row.t, ~headers=?, ()) =>
  row.url
  |> Js.String.replaceByRe([%bs.re "/\\W/g"], "")
  |> Js.String.trim
  |> Js.String.length > 0 ?
    {
      let url = row |> EosBp_Table.Row.jsonUrl;
      fetch(~url, ~headers?, ());
    } :
    Js.Promise.resolve(Response.BadUrl(row.url));

let bpJson = (~row, ~headers=?, ()) =>
  bpJsonRaw(~row, ~headers?, ()) |> thenDecode(EosBp_Json.decode);
