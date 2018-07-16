open MaterialUi;
open SimpleMaps;

let world50m = [%bs.raw {|require("./world-50m.json")|}];

let component = ReasonReact.statelessComponent(__MODULE__);

Js.log2("!!!!!!!!!", world50m);

let make = _children => {
  ...component,
  render: _self =>
    <div>
      <AppBar position=`Static color=`Default elevation=(`Int(0))>
        <Grid
          container=true
          spacing=Grid.V16
          alignItems=`Center
          justify=`Space_Between>
          <Grid item=true>
            <Typography variant=`Title color=`Inherit>
              <strong> ("EOS" |> ReasonReact.string) </strong>
              (" BP" |> ReasonReact.string)
              <strong> ("." |> ReasonReact.string) </strong>
              ("Report" |> ReasonReact.string)
            </Typography>
          </Grid>
          <Grid item=true>
            <Tabs value=0>
              <Tab label=("Producers" |> ReasonReact.string) />
              <Tab label=("Proxies" |> ReasonReact.string) />
              <Tab label=("Nodes" |> ReasonReact.string) />
              <Tab label=("Schema" |> ReasonReact.string) />
            </Tabs>
          </Grid>
        </Grid>
      </AppBar>
      <div>
        <ComposableMap height=600>
          <ZoomableGroup>
            <Geographies geography=world50m>
              (
                (geographies, projection) =>
                  geographies
                  |> Js.Array.mapi((geography, i) =>
                       <Geography
                         key=(i |> string_of_int)
                         geography
                         projection
                         style={
                           "default": {
                             "fill": "#ECEFF1",
                             "stroke": "#607D8B",
                             "strokeWidth": 0.75,
                             "outline": "none",
                           },
                           "hover": {
                             "fill": "#ECEFF1",
                             "stroke": "#607D8B",
                             "strokeWidth": 0.75,
                             "outline": "none",
                           },
                           "pressed": {
                             "fill": "#ECEFF1",
                             "stroke": "#607D8B",
                             "strokeWidth": 0.75,
                             "outline": "none",
                           },
                         }
                       />
                     )
                  |> ReasonReact.array
              )
            </Geographies>
          </ZoomableGroup>
        </ComposableMap>
        <Grid container=true alignItems=`Flex_End>
          <Grid item=true />
          <Grid item=true>
            <TextField
              label=("Search" |> ReasonReact.string)
              type_="search"
              fullWidth=true
            />
          </Grid>
        </Grid>
        <Paper>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell numeric=true>
                  ("Rank" |> ReasonReact.string)
                </TableCell>
                <TableCell> ("Name" |> ReasonReact.string) </TableCell>
                <TableCell> ("bp.json URL" |> ReasonReact.string) </TableCell>
                <TableCell numeric=true>
                  ("Last Change" |> ReasonReact.string)
                </TableCell>
                <TableCell> ("Votes" |> ReasonReact.string) </TableCell>
                <TableCell />
              </TableRow>
            </TableHead>
            <TableBody>
              <TableRow>
                <TableCell numeric=true>
                  ("1" |> ReasonReact.string)
                </TableCell>
                <TableCell>
                  <Link route=(Route.Producer("eosnewyorkio"))>
                    ("eosnewyorkio" |> ReasonReact.string)
                  </Link>
                </TableCell>
                <TableCell>
                  <a href="https://bp.eosnewyork.io/bp.json">
                    <code>
                      (
                        "https://bp.eosnewyork.io/bp.json" |> ReasonReact.string
                      )
                    </code>
                  </a>
                </TableCell>
                <TableCell numeric=true>
                  ("5 days ago" |> ReasonReact.string)
                </TableCell>
                <TableCell>
                  <LinearProgress variant=`Determinate value=(`Float(100.)) />
                </TableCell>
                <TableCell>
                  <code> ("68,956,425" |> ReasonReact.string) </code>
                </TableCell>
              </TableRow>
              <TableRow>
                <TableCell numeric=true>
                  ("2" |> ReasonReact.string)
                </TableCell>
                <TableCell>
                  <Link route=(Route.Producer("eoscanadacom"))>
                    ("eoscanadacom" |> ReasonReact.string)
                  </Link>
                </TableCell>
                <TableCell>
                  <a href="https://www.eoscanada.com/hubfs/bp_info.json">
                    <code>
                      (
                        "https://www.eoscanada.com/hubfs/bp_info.json"
                        |> ReasonReact.string
                      )
                    </code>
                  </a>
                </TableCell>
                <TableCell numeric=true>
                  ("5 days ago" |> ReasonReact.string)
                </TableCell>
                <TableCell>
                  <LinearProgress variant=`Determinate value=(`Float(80.)) />
                </TableCell>
                <TableCell>
                  <code> ("68,956,425" |> ReasonReact.string) </code>
                </TableCell>
              </TableRow>
            </TableBody>
          </Table>
        </Paper>
      </div>
    </div>,
};
