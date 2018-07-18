ReactDOMRe.hydrateToElementWithId(<App />, "app");

ReasonReact.Router.push("");

/* Temporary start */

let eos = Eos.make(~httpEndpoint="http://node2.liquideos.com", ());

let _ = [%bs.raw {|window.eos = eos|}];

let _ = [%bs.raw {|window.Eos = require('eosjs')|}];

/* Temporary end */
