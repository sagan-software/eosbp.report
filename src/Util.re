let formatNumberString =
  Js.String.replaceByRe([%bs.re "/\\B(?=(\\d{3})+(?!\\d))/g"], ",");

let formatInt = int => int |. Js.Int.toString |> formatNumberString;

let decodeJsonAsResult = (decoder, json) =>
  switch (decoder(json)) {
  | decoded => Belt.Result.Ok(decoded)
  | exception (Json.Decode.DecodeError(msg)) => Belt.Result.Error(msg)
  };

let parseJsonAsPromise = text =>
  switch (text |. Js.Json.parseExn) {
  | parsed => Js.Promise.resolve(parsed)
  | exception error => Js.Promise.reject(error)
  };

let decodeJsonAsPromise = (decode, json) =>
  switch (json |. decode) {
  | decoded => Js.Promise.resolve(decoded)
  | exception error => Js.Promise.reject(error)
  };

let parseAndDecodeAsPromise = (decode, text) =>
  text |. parseJsonAsPromise |> Js.Promise.then_(decodeJsonAsPromise(decode));

let promiseToOption = promise =>
  promise
  |> Js.Promise.then_(result => result |. Some |. Js.Promise.resolve)
  |> Js.Promise.catch(_error => None |. Js.Promise.resolve);

let promiseToResult = promise =>
  promise
  |> Js.Promise.then_(result =>
       result |. Belt.Result.Ok |. Js.Promise.resolve
     )
  |> Js.Promise.catch(error =>
       error |. Belt.Result.Error |. Js.Promise.resolve
     );

let onlySome = array =>
  Belt.Array.reduce(
    array,
    [||],
    (results, item) => {
      switch (item) {
      | Some(item) => Js.Array.push(item, results) |> ignore
      | _ => ()
      };
      results;
    },
  );

let onlyOk = array =>
  Belt.Array.reduce(
    array,
    [||],
    (results, item) => {
      switch (item) {
      | Belt.Result.Ok(item) => Js.Array.push(item, results) |> ignore
      | _ => ()
      };
      results;
    },
  );

let chunkArray = (originalArr, chunkSize) => {
  let results = [||];
  let arr = Belt.Array.copy(originalArr);
  while (arr |. Belt.Array.length > 0) {
    let chunk =
      arr |> Js.Array.spliceInPlace(~pos=0, ~remove=chunkSize, ~add=[||]);
    results |> Js.Array.push(chunk) |> ignore;
  };
  results;
};

let chunkArrayPromises = (arr: array('a), fn: 'a => 'b, chunkSize) =>
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
    chunkArray(arr, chunkSize),
  );

let doNothing = thing => thing;

let timeoutPromise = ms =>
  Js.Promise.make((~resolve, ~reject as _) =>
    Js.Global.setTimeout(() => resolve(. 1), ms) |> ignore
  );
