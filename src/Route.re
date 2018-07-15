type t =
  | Home
  | Producer(string)
  | NotFound;

let fromList = parts =>
  switch (parts) {
  | [] => Home
  | [producer] => Producer(producer)
  | _ => NotFound
  };

let fromUrl = (url: ReasonReact.Router.url) => fromList(url.path);

let fromString = str =>
  str
  |> Js.String.split("/")
  |> Belt.List.fromArray
  |> Js.List.filter((. str) => str != "")
  |> fromList;

let toString = route =>
  switch (route) {
  | Home => "/"
  | Producer(producer) => "/" ++ producer ++ "/"
  | NotFound => "/?404"
  };

let redirect = route => route |. toString |. ReasonReact.Router.push;
