[@bs.val] [@bs.scope "process.env"] external nodeEnv : string = "NODE_ENV";

[@bs.val] [@bs.scope "process.env"] external metadata : string = "APP_LABEL";

[@bs.val] [@bs.scope "process.env"]
external twitterName : string = "TWITTER_NAME";

[@bs.val] [@bs.scope "process.env"]
external githubName : string = "GITHUB_NAME";

[@bs.val] [@bs.scope "process.env"] external siteUrl : string = "SITE_URL";

[@bs.val] [@bs.scope "process.env"] external sitePort : string = "SITE_PORT";

[@bs.val] [@bs.scope "process.env"]
external siteWsUrl : string = "SITE_WS_URL";

[@bs.val] [@bs.scope "process.env"] external staticUrl : string = "STATIC_URL";

[@bs.val] [@bs.scope "process.env"] external eosUrl : string = "EOS_URL";

let isDev = nodeEnv == "development";

let buildDir = Node.Path.join([|Node.Process.cwd(), "build"|]);
