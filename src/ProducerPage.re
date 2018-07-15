open MaterialUi;

let component = ReasonReact.statelessComponent(__MODULE__);

let make = (~producer, _children) => {
  ...component,
  render: _self =>
    <div>
      <Typography variant=`Headline>
        (producer |> ReasonReact.string)
      </Typography>
      <Typography variant=`Body1>
        <Link route=Route.Home> ("Back to home" |> ReasonReact.string) </Link>
      </Typography>
    </div>,
};
