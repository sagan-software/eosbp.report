module Location = {
  type t = {
    name: string,
    country: string,
    latitude: float,
    longitude: float,
  };
  let decode = j =>
    Json.Decode.{
      name: j |> field("name", string),
      country: j |> field("country", string),
      latitude: j |> field("latitude", Json.Decode.float),
      longitude: j |> field("longitude", Json.Decode.float),
    };
  let encode = d =>
    Json.Encode.(
      object_([
        ("name", d.name |> string),
        ("country", d.country |> string),
        ("latitude", d.latitude |> Json.Encode.float),
        ("longitude", d.longitude |> Json.Encode.float),
      ])
    );
};

let encodeObjectWithOptions = fields =>
  fields
  |> Js.List.foldLeft(
       (. fields, field) => {
         switch (field) {
         | Some(field) => Js.Array.push(field, fields) |> ignore
         | None => ()
         };
         fields;
       },
       [||],
     )
  |> Belt.List.fromArray
  |> Json.Encode.object_;

module Node = {
  type t = {
    location: Location.t,
    isProducer: option(bool),
    nodeType: option(array(string)),
    p2pEndpoint: option(string),
    bnetEndpoint: option(string),
    apiEndpoint: option(string),
    sslEndpoint: option(string),
  };
  let decode = j =>
    Json.Decode.{
      location: j |> field("location", Location.decode),
      isProducer: j |> optional(field("is_producer", bool)),
      nodeType:
        j
        |> optional(
             field(
               "node_type",
               either(array(string), string |> map(s => [|s|])),
             ),
           ),
      p2pEndpoint: j |> optional(field("p2p_endpoint", string)),
      bnetEndpoint: j |> optional(field("bnet_endpoint", string)),
      apiEndpoint: j |> optional(field("api_endpoint", string)),
      sslEndpoint: j |> optional(field("ssl_endpoint", string)),
    };
  let encode = d =>
    Json.Encode.(
      Js.Option.(
        encodeObjectWithOptions([
          Some(("location", Location.encode(d.location))),
          d.isProducer |> map((. x) => ("is_producer", x |> bool)),
          d.nodeType |> map((. x) => ("node_type", x |> stringArray)),
          d.p2pEndpoint |> map((. x) => ("p2p_endpoint", x |> string)),
          d.bnetEndpoint |> map((. x) => ("bnet_endpoint", x |> string)),
          d.apiEndpoint |> map((. x) => ("api_endpoint", x |> string)),
          d.sslEndpoint |> map((. x) => ("ssl_endpoint", x |> string)),
        ])
      )
    );
};

module Org = {
  module Branding = {
    type t = {
      logo256: option(string),
      logo1024: option(string),
      logoSvg: option(string),
    };
    let decode = j =>
      Json.Decode.{
        logo256: j |> optional(field("logo_256", string)),
        logo1024: j |> optional(field("logo_1024", string)),
        logoSvg: j |> optional(field("logo_svg", string)),
      };

    let encode = d =>
      Json.Encode.(
        Js.Option.(
          encodeObjectWithOptions([
            d.logo256 |> map((. x) => ("logo_256", x |> string)),
            d.logo1024 |> map((. x) => ("logo_1024", x |> string)),
            d.logoSvg |> map((. x) => ("logo_svg", x |> string)),
          ])
        )
      );
  };

  module Social = {
    type t = {
      facebook: option(string),
      github: option(string),
      keybase: option(string),
      reddit: option(string),
      steemit: option(string),
      telegram: option(string),
      twitter: option(string),
      wechat: option(string),
      youtube: option(string),
    };
    let decode = j =>
      Json.Decode.{
        facebook: j |> optional(field("facebook", string)),
        github: j |> optional(field("github", string)),
        keybase: j |> optional(field("keybase", string)),
        reddit: j |> optional(field("reddit", string)),
        steemit: j |> optional(field("steemit", string)),
        telegram: j |> optional(field("telegram", string)),
        twitter: j |> optional(field("twitter", string)),
        wechat: j |> optional(field("wechat", string)),
        youtube: j |> optional(field("youtube", string)),
      };
    let encode = d =>
      Json.Encode.(
        Js.Option.(
          encodeObjectWithOptions([
            d.facebook |> map((. x) => ("facebook", x |> string)),
            d.github |> map((. x) => ("github", x |> string)),
            d.keybase |> map((. x) => ("keybase", x |> string)),
            d.reddit |> map((. x) => ("reddit", x |> string)),
            d.steemit |> map((. x) => ("steemit", x |> string)),
            d.telegram |> map((. x) => ("telegram", x |> string)),
            d.twitter |> map((. x) => ("twitter", x |> string)),
            d.wechat |> map((. x) => ("wechat", x |> string)),
            d.youtube |> map((. x) => ("youtube", x |> string)),
          ])
        )
      );
  };
  type t = {
    candidateName: string,
    location: Location.t,
    website: string,
    codeOfConduct: option(string),
    ownershipDisclosure: option(string),
    email: option(string),
    branding: option(Branding.t),
    social: option(Social.t),
  };
  let decode = j =>
    Json.Decode.{
      candidateName: j |> field("candidate_name", string),
      location: j |> field("location", Location.decode),
      website: j |> field("website", string),
      codeOfConduct: j |> optional(field("code_of_conduct", string)),
      ownershipDisclosure:
        j |> optional(field("ownership_disclosure", string)),
      email: j |> optional(field("email", string)),
      branding: j |> optional(field("branding", Branding.decode)),
      social: j |> optional(field("social", Social.decode)),
    };
  let encode = d =>
    Json.Encode.(
      Js.Option.(
        encodeObjectWithOptions([
          ("candidate_name", d.candidateName |> string) |. Some,
          ("location", d.location |> Location.encode) |. Some,
          ("website", d.website |> string) |. Some,
          d.codeOfConduct |> map((. x) => ("code_of_conduct", x |> string)),
          d.ownershipDisclosure
          |> map((. x) => ("ownership_disclosure", x |> string)),
          d.email |> map((. x) => ("email", x |> string)),
          d.branding |> map((. x) => ("branding", x |> Branding.encode)),
          d.social |> map((. x) => ("social", x |> Social.encode)),
        ])
      )
    );
};

type t = {
  producerAccountName: string,
  producerPublicKey: string,
  org: Org.t,
  nodes: Js.Array.t(Node.t),
};

let decode = j =>
  Json.Decode.{
    producerAccountName: j |> field("producer_account_name", string),
    producerPublicKey: j |> field("producer_public_key", string),
    org: j |> field("org", Org.decode),
    nodes: j |> field("nodes", array(Node.decode)),
  };

let encode = d =>
  Json.Encode.(
    object_([
      ("producer_account_name", d.producerAccountName |> string),
      ("producer_public_key", d.producerPublicKey |> string),
      ("org", d.org |> Org.encode),
      ("nodes", d.nodes |> array(Node.encode)),
    ])
  );

[@bs.module] [@bs.val]
external schema : Js.Json.t = "bp-info-standard/schema.json";

module Ajv = {
  type t;
  type error = Js.Json.t;
  [@bs.new] [@bs.module] external make : {. "allErrors": bool} => t = "ajv";
  [@bs.send] external validate : (t, Js.Json.t, Js.Json.t) => bool = "";
  [@bs.get] external errors : t => array(error) = "";
};

let validate = json => {
  let ajv = Ajv.make({"allErrors": true});
  ajv |. Ajv.validate(schema, json) |> ignore;
  ajv |. Ajv.errors;
};

let normalizeUrl = baseUrl => {
  /* Remove invalid unicode characters from beginning and end */
  let url =
    baseUrl
    |> Js.String.replaceByRe([%bs.re "/^[^\\x00-\\x7F]/g"], "")
    |> Js.String.replaceByRe([%bs.re "/[^\\x00-\\x7F]$/g"], "")
    |> Js.String.replaceByRe([%bs.re "/\\/$/g"], "");

  /* Add protocol if missing */
  let url =
    Js.String.startsWith("http", url) || Js.String.startsWith("https", url) ?
      url : "http://" ++ url;
  url;
};

let normalizeBpJsonUrl = baseUrl => {
  let url = baseUrl |. normalizeUrl;
  let url = Js.String.endsWith(".json", url) ? url : url ++ "/bp.json";
  url;
};
