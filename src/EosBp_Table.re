module Row = {
  type t = {
    owner: string,
    totalVotes: string,
    producerKey: string,
    isActive: bool,
    url: string,
    unpaidBlocks: int,
    lastClaimTime: Js.Date.t,
    location: int,
  };
  let decode = j =>
    Json.Decode.{
      owner: j |> field("owner", string),
      totalVotes: j |> field("total_votes", string),
      producerKey: j |> field("producer_key", string),
      isActive: j |> field("is_active", int |> map(x => x === 1)),
      url: j |> field("url", string),
      unpaidBlocks: j |> field("unpaid_blocks", int),
      lastClaimTime:
        j
        |> field(
             "last_claim_time",
             either(Json.Decode.float, string |> map(float_of_string))
             |> map(us => us /. 1000. |> Js.Date.fromFloat),
           ),
      location: j |> field("location", int),
    };
  let encode = d =>
    Json.Encode.(
      object_([
        ("owner", d.owner |> string),
        ("total_votes", d.totalVotes |> string),
        ("producer_key", d.producerKey |> string),
        ("is_active", int(d.isActive ? 1 : 0)),
        ("url", d.url |> string),
        ("unpaid_blocks", d.unpaidBlocks |> int),
        (
          "last_claim_time",
          (d.lastClaimTime |> Js.Date.getTime) *. 1000. |> int_of_float |> int,
        ),
        ("location", d.location |> int),
      ])
    );

  let jsonUrl = t => {
    let url = t.url;
    /* Remove invalid unicode characters from beginning and end */
    let url =
      url
      |> Js.String.replaceByRe([%bs.re "/^[^\\x00-\\x7F]/g"], "")
      |> Js.String.replaceByRe([%bs.re "/[^\\x00-\\x7F]$/g"], "")
      |> Js.String.replaceByRe([%bs.re "/\\/$/g"], "");

    /* Add protocol if missing */
    let url =
      Js.String.startsWith("http", url) || Js.String.startsWith("https", url) ?
        url : "http://" ++ url;
    /* Add /bp.json if necessary */
    let url = Js.String.endsWith(".json", url) ? url : url ++ "/bp.json";
    url;
  };
};

type t = {
  rows: array(Row.t),
  more: bool,
};

let decode = j =>
  Json.Decode.{
    rows: j |> field("rows", array(Row.decode)),
    more: j |> field("more", bool),
  };

let headers =
  Fetch.HeadersInit.make({
    "Content-Type": "application/json",
    "Origin": "http://eosbpdotjson.io",
    "Referer": "http://eosbpdotjson.io",
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36",
  });

let fetch = (~httpEndpoint, ~headers=headers, ()) => {
  let payload =
    Json.Encode.(
      [
        ("scope", string("eosio")),
        ("code", string("eosio")),
        ("table", string("producers")),
        ("json", bool(true)),
        ("limit", int(5000)),
      ]
      |> object_
      |> Json.stringify
    );
  let options =
    Fetch.RequestInit.make(
      ~method_=Fetch.Post,
      ~headers,
      ~body=payload |> Fetch.BodyInit.make,
      (),
    );
  let url = {j|$httpEndpoint/v1/chain/get_table_rows|j};
  Js.Promise.(
    Fetch.fetchWithInit(url, options)
    |> then_(Fetch.Response.json)
    |> then_(j => {
         Js.log2("got JSON", j);
         Json.Decode.(j |> field("rows", array(Row.decode))) |> resolve;
       })
    |> catch(e => {
         Js.log2("error fetching producers table JSON:", e);
         resolve([||]);
       })
  );
};
