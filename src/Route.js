// Generated by BUCKLESCRIPT VERSION 4.0.0, PLEASE EDIT WITH CARE
'use strict';

var Block = require("bs-platform/lib/js/block.js");
var Js_list = require("bs-platform/lib/js/js_list.js");
var Belt_List = require("bs-platform/lib/js/belt_List.js");
var ReasonReact = require("reason-react/src/ReasonReact.js");

function fromList(parts) {
  if (parts) {
    var match = parts[1];
    var producer = parts[0];
    if (match) {
      if (match[1]) {
        return /* NotFound */1;
      } else {
        return /* ProducerHash */Block.__(1, [
                  producer,
                  match[0]
                ]);
      }
    } else {
      return /* Producer */Block.__(0, [producer]);
    }
  } else {
    return /* Home */0;
  }
}

function fromUrl(url) {
  return fromList(url[/* path */0]);
}

function fromString(str) {
  return fromList(Js_list.filter((function (str) {
                    return str !== "";
                  }), Belt_List.fromArray(str.split("/"))));
}

function toString(route) {
  if (typeof route === "number") {
    if (route === 0) {
      return "/";
    } else {
      return "/?404";
    }
  } else if (route.tag) {
    return "/" + (route[0] + ("/" + route[1]));
  } else {
    return "/" + route[0];
  }
}

function redirect(route) {
  return ReasonReact.Router[/* push */0](toString(route));
}

exports.fromList = fromList;
exports.fromUrl = fromUrl;
exports.fromString = fromString;
exports.toString = toString;
exports.redirect = redirect;
/* ReasonReact Not a pure module */
