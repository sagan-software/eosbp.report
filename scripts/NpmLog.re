module Tracker = {
  type t;

  [@bs.send] external info : (t, string, string, 'data) => unit = "";

  [@bs.send] external warn : (t, string, string, 'data) => unit = "";

  [@bs.send] external error : (t, string, string, 'data) => unit = "";
};

[@bs.module "npmlog"] external debug : (string, string, 'data) => unit = "";

[@bs.module "npmlog"] external info : (string, string, 'data) => unit = "";

[@bs.module "npmlog"] external warn : (string, string, 'data) => unit = "";

[@bs.module "npmlog"] external error : (string, string, 'data) => unit = "";

[@bs.module "npmlog"] external newItem : (string, int, int) => Tracker.t = "";

[@bs.module "npmlog"] external enableProgress : unit => unit = "";

[@bs.module "npmlog"] external enableColor : unit => unit = "";
