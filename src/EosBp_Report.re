type t = {
  producer: Eos.AccountName.t,
  rank: int,
  ramBytes: BigNumber.t,
  netWeight: Eos.Asset.t,
  cpuWeight: Eos.Asset.t,
  votedFor: array(Eos.AccountName.t),
  totalVotes: BigNumber.t,
  coreLiquidBalance: Eos.Asset.t,
  created: Eos.BlockTimestamp.t,
  estimatedRewards: Eos.Asset.t,
  hasBpJson: bool,
  websiteUrl: string,
  websiteUrlReachable: bool,
  bpJsonUrl: string,
  bpJsonValid: bool,
  bpJsonUrlReachable: bool,
  unpaidBlocks: BigNumber.t,
  location: int,
  isActive: string,
  producerKey: string,
  reachableNodes: int,
  unreachableNodes: int,
  lastBpJsonChange: Js.Date.t,
};

module Node = {
  type t = {
    producer: Eos.AccountName.t,
    url: URL.t,
    latitude: float,
    longitude: float,
    nodeType: array(string),
    infoStatus: int,
    serverVersion: option(string),
    chainId: option(string),
    headBlockNum: option(int),
    headBlockTime: option(Eos.BlockTimestamp.t),
    lastIrreversibleBlockNum: option(int),
  };

  let encode = t =>
    Json.Encode.(
      object_([
        ("producer", t.producer |. Eos.AccountName.encode),
        ("url", t.url |. URL.toString |. string),
        ("latitude", t.latitude |. Json.Encode.float),
        ("longitude", t.longitude |. Json.Encode.float),
        ("node_type", t.nodeType |. stringArray),
        ("info_status", t.infoStatus |. int),
        ("server_version", t.serverVersion |> nullable(string)),
        ("chain_id", t.chainId |> nullable(string)),
        ("head_block_num", t.headBlockNum |> nullable(int)),
        (
          "head_block_time",
          t.headBlockTime |> nullable(Eos.BlockTimestamp.encode),
        ),
        (
          "last_irreversible_block_num",
          t.lastIrreversibleBlockNum |> nullable(int),
        ),
      ])
    );
};
