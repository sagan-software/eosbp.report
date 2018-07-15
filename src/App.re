open MaterialUi;

type state = {route: Route.t};

type action =
  | ChangeRoute(Route.t);

let reducer = (action, state) =>
  switch (action) {
  | ChangeRoute(route) => ReasonReact.Update({...state, route})
  };

let component = ReasonReact.reducerComponent(__MODULE__);

let make = (~route=Route.Home, _children) => {
  ...component,
  reducer,
  initialState: () => {route: route},
  subscriptions: self => [
    Sub(
      () =>
        ReasonReact.Router.watchUrl(url =>
          self.send(ChangeRoute(url |> Route.fromUrl))
        ),
      ReasonReact.Router.unwatchUrl,
    ),
  ],
  render: self =>
    <div>
      <Helmet>
        <html prefix="og: http://ogp.me/ns#" />
        <meta name="description" content="" />
        <meta
          name="viewport"
          content="width=device-width, initial-scale=1, shrink-to-fit=no"
        />
        <link
          rel="stylesheet"
          href="https://fonts.googleapis.com/css?family=Roboto:300,400,500"
        />
      </Helmet>
      <CssBaseline />
      (
        switch (self.state.route) {
        | Route.Home => <Home />
        | Route.Producer(producer) => <ProducerPage producer />
        | Route.NotFound => "Error: Not found" |> ReasonReact.string
        }
      )
    </div>,
};
