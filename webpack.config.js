const HtmlWebpackPlugin = require("html-webpack-plugin");
const webpack = require("webpack");
const path = require("path");

const isProd = process.env.NODE_ENV === "production";

const webConfig = {
	entry: "./src/Index.js",
	mode: isProd ? "production" : "development",
	output: {
		path: path.resolve(__dirname, "build"),
		filename: "index.js",
		publicPath: "/",
	},
	performance: {
		hints: isProd ? "warning" : false,
	},
	devServer: {
		contentBase: false,
		compress: true,
		port: 9001,
		hot: true,
		inline: true,
		historyApiFallback: true,
	},
	plugins: [
		new HtmlWebpackPlugin({
			inject: true,
			showErrors: true,
			template: path.resolve(__dirname, "src/index.html"),
			filename: "index.html",
		}),
		new webpack.EnvironmentPlugin([
			"NODE_ENV",
			"STATIC_URL",
			"CONTRACT_ACCOUNT",
		]),
	],
};

const nodeConfig = {
	target: "node",
	mode: isProd ? "production" : "development",
	resolve: {
		mainFields: ["main", "module"],
	},
	entry: "./scripts/fetch_json.js",
	output: {
		path: path.resolve(__dirname, "scripts"),
		filename: "fetch_json.bundle.js",
	},
};

const TARGET =
	process.env.TARGET === "web"
		? "web"
		: process.env.TARGET === "node"
			? "node"
			: "all";

module.exports = () => {
	console.log(`Targeting ${TARGET} files`);
	switch (TARGET) {
		case "web":
			return webConfig;
		case "node":
			return nodeConfig;
		default:
			[webConfig, nodeConfig];
	}
};
