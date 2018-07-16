module ComposableMap = {
  [@bs.module "react-simple-maps"]
  external reactClass : ReasonReact.reactClass = "ComposableMap";

  [@bs.deriving abstract]
  type jsProps = {
    [@bs.optional]
    width: int,
    [@bs.optional]
    height: int,
  };

  let make = (~width=?, ~height=?, children) =>
    ReasonReact.wrapJsForReason(
      ~reactClass,
      ~props=jsProps(~width?, ~height?, ()),
      children,
    );
};
module ZoomableGroup = {
  [@bs.module "react-simple-maps"]
  external reactClass : ReasonReact.reactClass = "ZoomableGroup";

  [@bs.deriving abstract]
  type jsProps = {
    [@bs.optional]
    zoom: int,
  };

  let make = (~zoom=?, children) =>
    ReasonReact.wrapJsForReason(
      ~reactClass,
      ~props=jsProps(~zoom?, ()),
      children,
    );
};

module Geographies = {
  [@bs.module "react-simple-maps"]
  external reactClass : ReasonReact.reactClass = "Geographies";

  [@bs.deriving abstract]
  type jsProps = {geography: Js.Json.t};

  let make = (~geography, children) =>
    ReasonReact.wrapJsForReason(
      ~reactClass,
      ~props=jsProps(~geography),
      children,
    );
};

module Geography = {
  [@bs.module "react-simple-maps"]
  external reactClass : ReasonReact.reactClass = "Geography";

  [@bs.deriving abstract]
  type jsProps('style) = {
    geography: Js.Json.t,
    projection: Js.Json.t,
    [@bs.optional]
    cacheId: string,
    [@bs.optional]
    style: 'style,
  };

  let make = (~geography, ~projection, ~cacheId=?, ~style=?, children) =>
    ReasonReact.wrapJsForReason(
      ~reactClass,
      ~props=jsProps(~geography, ~projection, ~cacheId?, ~style?, ()),
      children,
    );
};
