type t =
  | Home
  | Producer(string)
  | ProducerHash(string, string)
  | NotFound;

let fromList = parts =>
  switch (parts) {
  | [] => Home
  | [producer] => Producer(producer)
  | [producer, hash] => ProducerHash(producer, hash)
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
  | Producer(producer) => "/" ++ producer
  | ProducerHash(producer, hash) => "/" ++ producer ++ "/" ++ hash
  | NotFound => "/?404"
  };

let redirect = route => route |. toString |. ReasonReact.Router.push;
