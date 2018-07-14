open MaterialUi;
module Icon = MaterialUIIcons;

let component = ReasonReact.statelessComponent(__MODULE__);

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
              ("JSON" |> ReasonReact.string)
            </Typography>
          </Grid>
          <Grid item=true>
            <Tabs value=0>
              <Tab label=("Producers" |> ReasonReact.string) />
              <Tab label=("Nodes" |> ReasonReact.string) />
              <Tab label=("Peers" |> ReasonReact.string) />
              <Tab label=("Schema" |> ReasonReact.string) />
            </Tabs>
          </Grid>
        </Grid>
      </AppBar>
      <div>
        <Grid container=true alignItems=`Flex_End>
          <Grid item=true> <Icon.Search /> </Grid>
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
              <TableCell> "eosnewyork.io" </TableCell>
              <TableCell>
                <a href="#">
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
              <TableCell> "eosnewyork.io" </TableCell>
              <TableCell>
                <a href="#">
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
