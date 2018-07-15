// var fetch = require("node-fetch");

// fetch("http://can-play.ca/bp.json").then(console.log.bind(console));

var request = require("request-promise");
request("http://eosbp.report/123singapore/bp.json").then(
	console.log.bind(console),
);
