/* https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods */
[@bs.deriving jsConverter]
type method = [
  | [@bs.as "GET"] `GET
  | [@bs.as "HEAD"] `HEAD
  | [@bs.as "POST"] `POST
  | [@bs.as "PUT"] `PUT
  | [@bs.as "DELETE"] `DELETE
  | [@bs.as "CONNECT"] `CONNECT
  | [@bs.as "OPTIONS"] `OPTIONS
  | [@bs.as "TRACE"] `TRACE
  | [@bs.as "PATCH"] `PATCH
];

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState */
[@bs.deriving jsConverter]
type readyState =
  | [@bs.as 0] UNSENT
  | [@bs.as 1] OPENED
  | [@bs.as 2] HEADERS_RECEIVED
  | [@bs.as 3] LOADING
  | [@bs.as 4] DONE;

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/responseType */
[@bs.deriving jsConverter]
type responseType = [
  | [@bs.as "arraybuffer"] `arraybuffer
  | [@bs.as "blob"] `blob
  | [@bs.as "document"] `document
  | [@bs.as "json"] `json
  | [@bs.as "text"] `text
];

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest */
type t;

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/XMLHttpRequest */
[@bs.new] external make_ : unit => t = "XMLHttpRequest";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/open */
[@bs.send] external open_ : (t, string, string) => unit = "open";

let make = (method, url) => {
  let xhr = make_();
  xhr |. open_(methodToJs(method), url);
  xhr;
};

[@bs.send] external send : t => unit = "";

[@bs.send] external sendString : (t, string) => unit = "send";

let sendJson = (t, json) => t |. sendString(json |> Js.Json.stringify);

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/setRequestHeader */
[@bs.send] external setRequestHeader : (t, string, string) => unit = "";

let setRequestHeaders = (t, headers) =>
  headers
  |> Js.List.iter((. (key, value)) => t |. setRequestHeader(key, value));

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/timeout */
[@bs.set] external setTimeout : (t, int) => unit = "timeout";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/abort */
[@bs.send] external abort : t => unit = "";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/getResponseHeader */
[@bs.send] [@bs.return nullable]
external getResponseHeader : (t, string) => option(string) = "";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/getAllResponseHeaders */
[@bs.send] [@bs.return nullable]
external getAllResponseHeaders : (t, string) => option(string) = "";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState */
[@bs.get] external getReadyState_ : t => int = "readyState";

let getReadyState = t => t |. getReadyState_ |. readyStateFromJs;

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/responseText */
[@bs.get] [@bs.return nullable]
external getResponseText : t => option(string) = "responseText";

[@bs.set] external setResponseType_ : (t, string) => unit = "responseType";

let setResponseType = (t, r) => t |. setResponseType_(r |> responseTypeToJs);

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/responseURL */
[@bs.get] [@bs.return nullable]
external getResponseUrl : t => option(string) = "responseURL";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/status */
[@bs.get] external status : t => int = "";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/statusText */
[@bs.get] external statusText : t => string = "";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/withCredentials */
[@bs.get] external getWithCredentials : t => bool = "withCredentials";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/withCredentials */
[@bs.set] external setWithCredentials : (t, bool) => unit = "withCredentials";

/* https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/onreadystatechange */
[@bs.set]
external onReadyStateChange : (t, unit => unit) => unit = "onreadystatechange";

/* https://developer.mozilla.org/en-US/docs/Web/Events/abort */
[@bs.set] external onAbort : (t, 'abortEvent => unit) => unit = "onabort";

/* https://developer.mozilla.org/en-US/docs/Web/Events/error */
[@bs.set] external onError : (t, 'error => unit) => unit = "onerror";

/* https://developer.mozilla.org/en-US/docs/Web/Events/load */
[@bs.set] external onLoad : (t, 'progressEvent => unit) => unit = "onload";

/* https://developer.mozilla.org/en-US/docs/Web/Events/timeout */
[@bs.set]
external onLoadEnd : (t, 'progressEvent => unit) => unit = "onloadend";

/* https://developer.mozilla.org/en-US/docs/Web/Events/loadstart */
[@bs.set]
external onLoadStart : (t, 'progressEvent => unit) => unit = "onloadstart";

/* https://developer.mozilla.org/en-US/docs/Web/Events/progress */

[@bs.set]
external onProgress : (t, 'progressEvent => unit) => unit = "onprogress";

/* https://developer.mozilla.org/en-US/docs/Web/Events/timeout */
[@bs.set]
external onTimeout : (t, 'progressEvent => unit) => unit = "ontimeout";
