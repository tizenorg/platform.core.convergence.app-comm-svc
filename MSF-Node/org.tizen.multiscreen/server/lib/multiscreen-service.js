/*
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
var pkg = require("../package.json");
var _ = require('lodash');
var express = require('express');
var createError = require('http-errors');
var EventEmitter = require('eventemitter2').EventEmitter2;

var config = require("./config");
var routes = require('./routes');
var logger = require('./logging');
var events = require('./events').EventBus;
var device = require('./device');
var utils  = require('./utils');
var settings  = require('./settings');

var service = new EventEmitter();
var running = false;

/*
 Set the public properties for the service
 */
service.plugins = {};
service.config = config;
service.port = config.service.port;
service.app = null;
service.server = null;
service.logger = logger;
service.version = pkg.version;
service.events = events;
service.device = device;
service.utils  = utils;
service.settings  = settings;
service.remoteVersion = '0';
/*
 Loads a plugin by name
 */
service.loadPlugin = function loadPlugin(name, path, config){
    // TODO : Add some validation here before this method is exported
    var PluginClass = require(path);
    this.plugins[name] = new PluginClass(config);
};

/*
 Starts the service
 */
service.start = function(){

    if(running) return logger.warn('MULTISCREEN SERVICE IS ALREADY RUNNING');

    logger.info('STARTING MULTISCREEN SERVICE');


    /*
     Set allowing of launching unverified urls
     */
    if(config.allowAllContent){
        device.allowAllContent = config.allowAllContent;
    }


    /*
     Store a public reference to the express application
     */
    this.app = express();


    /*
     Subscribe to all global events for logging
     */
    this.events.onAny(function(value){
        logger.silly('eventbus published : ', this.event);
    });


    /*
     Store the app configuration
     */
    this.app.set('config', this.config);


    /*
     Configure routes
     */
    routes.configure(this.app);


    /*
     Store a reference to the http server
     */
    this.server = this.app.listen(this.config.service.port);


    /*
     Load Plugins from the config
     */
    _.each(this.config.plugins,function(config, key, list){
        this.loadPlugin(key, '../plugins/'+key, config);
    },this);


    /*
     404 Handler (this needs to be the last `app.use` request handler)
     */
    this.app.use(function(req, res, next){

        res.status(404);

        // respond with JSON
        if(req.accepts('json') && (req.url.indexOf('/api') === 0 || req.method === 'POST' )){
            next(createError(404));
            return;
        }

        // respond with html 404
        if (req.accepts('html')) {
            res.sendfile(require('path').join(__dirname,'resources/public/404.html'));
            return;
        }

        // default to plain-text. send()
        res.type('txt').send('Not found');
    });

    /*
     Application level Error Handler (this needs to be the last `app.use` error handler)
     */
    this.app.use(function(err, req, res, next) {
        //logger.error(err.stack);
        var e = {};
        if(err.status && err.status < 500){
            e.status    = err.status || err.statusCode || 500;
            e.message   = err.message || 'Error '+ e.message;
            e.code      = e.code || e.status;
            e.details   = err.details || void 0;
        }else{
            e.status    = 500;
            e.message   = 'Internal Server Error';
            e.code      = e.status;
        }
        res.send(e.status,e);
    });

    running = true;

};


service.stop = function(){
    this.logger.info('STOPPING MULTISCREEN SERVICE');
    if(this.server){
        this.server.close();
    }
    // TODO : Stop any necessary plugins and remove any unused resources
    running = false;
};

module.exports = service;