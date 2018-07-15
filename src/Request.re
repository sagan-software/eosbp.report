type t;

[@bs.get] external statusCode : t => int = "";
[@bs.get] external statusText : t => string = "statusMessage";
[@bs.get] external body : t => string = "";
[@bs.get] external headers : t => Js.Dict.t(string) = "";
let header = (t, key) => t |> headers |. Js.Dict.get(key);
[@bs.get] external httpVersionMajor : t => int = "";
[@bs.get] external httpVersionMinor : t => int = "";
[@bs.get] external httpVersionMinor : t => int = "";
[@bs.get] [@bs.scope "request"] external url : t => URL.t = "uri";

module Options = {
  [@bs.deriving abstract]
  type t = {
    url: string,
    [@bs.as "method"]
    method_: string,
    json: bool,
    resolveWithFullResponse: bool,
    [@bs.optional]
    body: option(string),
    simple: bool,
    time: bool,
    timeout: int,
    headers: Js.Dict.t(string),
    encoding: Js.Null_undefined.t(string),
  };
};

[@bs.module]
external make_ : Options.t => Js.Promise.t(t) = "request-promise";

exception TimedOut(string);
exception UnknownError(Js.Promise.error);

module Error = {
  type t;
  [@bs.get] external options : t => Options.t = "";
  [@bs.get] external name : t => string = "";
  [@bs.get] external message : t => string = "";
  [@bs.get] external error : t => Js.Exn.t = "";
  [@bs.get] [@bs.scope "error"] external code : t => string = "";
  external fromPromiseError : Js.Promise.error => t = "%identity";
};

let make =
    (
      ~url,
      ~method_="GET",
      ~json=false,
      ~body=?,
      ~timeout=0,
      ~headers=?,
      ~encoding=Js.Null_undefined.undefined,
      (),
    ) =>
  Options.t(
    ~url,
    ~method_,
    ~json,
    ~resolveWithFullResponse=true,
    ~body,
    ~simple=false,
    ~time=true,
    ~timeout,
    ~headers=headers |> Js.Option.getWithDefault([||]) |> Js.Dict.fromArray,
    ~encoding,
    (),
  )
  |> make_
  |> Js.Promise.catch(e =>
       (
         switch (e |> Error.fromPromiseError |> Error.code) {
         | "ETIMEDOUT" => TimedOut(url)
         | _ => UnknownError(e)
         }
       )
       |> Js.Promise.reject
     );
/* make_({
     "url": url,
     "method": method_,
     "json": json,
     "body": Js.Nullable.fromOption(body),
     "resolveWithFullResponse": true,
     "simple": false,
     "time": true,
     "timeout": timeout,
     "headers": ,
   }); */
