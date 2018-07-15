let component = ReasonReact.statelessComponent(__MODULE__);

let make = (~producer, _children) => {
  ...component,
  render: _self =>
    <div>
      <h1> (producer |> ReasonReact.string) </h1>
      <p>
        <Link route=Route.Home> ("Back to home" |> ReasonReact.string) </Link>
      </p>
    </div>,
};
