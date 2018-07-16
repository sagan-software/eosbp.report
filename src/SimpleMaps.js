// Generated by BUCKLESCRIPT VERSION 4.0.0, PLEASE EDIT WITH CARE
'use strict';

var ReasonReact = require("reason-react/src/ReasonReact.js");
var Js_primitive = require("bs-platform/lib/js/js_primitive.js");
var ReactSimpleMaps = require("react-simple-maps");

function make(width, height, children) {
  var tmp = { };
  if (width) {
    tmp.width = Js_primitive.valFromOption(width);
  }
  if (height) {
    tmp.height = Js_primitive.valFromOption(height);
  }
  return ReasonReact.wrapJsForReason(ReactSimpleMaps.ComposableMap, tmp, children);
}

var ComposableMap = /* module */[/* make */make];

function make$1(zoom, children) {
  var tmp = { };
  if (zoom) {
    tmp.zoom = Js_primitive.valFromOption(zoom);
  }
  return ReasonReact.wrapJsForReason(ReactSimpleMaps.ZoomableGroup, tmp, children);
}

var ZoomableGroup = /* module */[/* make */make$1];

function make$2(geography, children) {
  return ReasonReact.wrapJsForReason(ReactSimpleMaps.Geographies, {
              geography: geography
            }, children);
}

var Geographies = /* module */[/* make */make$2];

function make$3(geography, projection, cacheId, style, children) {
  var tmp = {
    geography: geography,
    projection: projection
  };
  if (cacheId) {
    tmp.cacheId = Js_primitive.valFromOption(cacheId);
  }
  if (style) {
    tmp.style = Js_primitive.valFromOption(style);
  }
  return ReasonReact.wrapJsForReason(ReactSimpleMaps.Geography, tmp, children);
}

var Geography = /* module */[/* make */make$3];

exports.ComposableMap = ComposableMap;
exports.ZoomableGroup = ZoomableGroup;
exports.Geographies = Geographies;
exports.Geography = Geography;
/* ReasonReact Not a pure module */
