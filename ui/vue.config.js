const CompressionPlugin = require("compression-webpack-plugin");
const HtmlWebpackPlugin = require('html-webpack-plugin')
const InlineChunkHtmlPlugin = require('inline-chunk-html-plugin');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');

module.exports = {
    css: {
        extract: false,
    },
    configureWebpack: config => {
        config.output.filename = '[name].js';
        config.output.chunkFilename = '[name].js';
        if (process.env.VUE_APP_EMBEDDED === 'true') {
            config.plugins.push(new HtmlWebpackPlugin({
                filename: 'index.html', // the output file name that will be created
                template: 'public/index.html', // this is important - a template file to use for insertion
                inlineSource: '.(js|css)$' // embed all javascript and css inline
            }));
            config.plugins.push(new InlineChunkHtmlPlugin(HtmlWebpackPlugin, [/app.js/]));
            config.plugins.push(new CompressionPlugin({}));
            config.optimization = {
                splitChunks: false,
                minimizer: [new UglifyJsPlugin({
                    cache: true,
                    parallel: true,
                    uglifyOptions: {
                        output: {
                            comments: false,
                        },
                    },
                })],
            };
        }
    },
    productionSourceMap: false
}