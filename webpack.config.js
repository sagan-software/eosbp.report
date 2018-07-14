const CleanWebpackPlugin = require("clean-webpack-plugin");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const webpack = require("webpack");
const path = require("path");

const isProd = process.env.NODE_ENV === "production";

module.exports = {
	entry: "./src/Index.js",
	mode: isProd ? "production" : "development",
	output: {
		path: path.resolve(__dirname, "build"),
		filename: "index.js",
		publicPath: "/",
	},
	performance: {
		hints: isProd ? "error" : false,
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
		new CleanWebpackPlugin(["build"], {
			verbose: true,
		}),
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
