// Generated by BUCKLESCRIPT VERSION 4.0.0, PLEASE EDIT WITH CARE
'use strict';

var Path = require("path");
var Process = require("process");

var isDev = process.env.NODE_ENV === "development";

var buildDir = Path.join(Process.cwd(), "build");

exports.isDev = isDev;
exports.buildDir = buildDir;
/* isDev Not a pure module */
