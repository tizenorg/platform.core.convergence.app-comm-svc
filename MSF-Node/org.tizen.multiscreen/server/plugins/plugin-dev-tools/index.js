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
var _ = require('lodash');
var fs = require('fs');
var os = require('os');
var path = require('path');
var MSFPlugin = require('../plugin');
var url = require('url');

var captcha = require('captchapng');

// TODO : Remove this path dependency
var WebSocketServer = require('../../lib/ws').Server;

/*
 Constants
 */
var URI_ROOT = '/';
var URI_LOGS = '/logs';
var URI_SETTINGS_UPDATE = '/settings';
var URI_CAPTCHA = '/captcha.png';
var URI_TEST_TOOL = '/msftest';


/*
 HTML Templates
 */
var indexTemplate = require.resolve('./templates/index.tpl');


/*
 The websocket log server is only created if someone views the logging page to reduce resources
 */
var wsLogServer;



function DevTools(config){

    /*
     Call the BaseModule constructor
     */
    MSFPlugin.apply(this, arguments);

    /*
     Route Handlers
     */
    this.app.get(URI_ROOT, this.renderIndex.bind(this));
    this.app.post(URI_SETTINGS_UPDATE, this.updateSettings.bind(this));
    this.app.get(URI_LOGS, this.renderLogs.bind(this));
    this.app.get(URI_CAPTCHA, this.renderCaptcha.bind(this));
	this.app.get(URI_TEST_TOOL, this.renderTool.bind(this));


}

MSFPlugin.extend(DevTools);

/*
 Creates and sets up the websocket server to broadcast logging messages
 */
DevTools.prototype.createWebSocketServer = function(){

    if(!wsLogServer){

        var self = this;

        this.logger.info('Starting websocket logging server');

        wsLogServer = new WebSocketServer({server:this.server, path:'/logger'});

        wsLogServer.broadcast = function(data) {
            for(var i=0; i<this.clients.length; i++){
                this.clients[i].send(data);
            }
        };

        wsLogServer.on('connection', function(ws) {
            self.logger.info('Client connected to log server....');
        });

        this.service.logger.addListener('logging', function(transport, level, msg, meta){

            // Only log the console transport to keep from broadcasting multiple times for a single message
            if(transport.name === 'file'){
                var entry = { level : level, msg : msg, meta : meta};
                // Wrap in a try catch in case the meta has a circular reference
                try{
                    entry = JSON.stringify(entry);
                    wsLogServer.broadcast(entry);
                }catch(e){
                    console.error(e);
                }

            }

        });

    }

};



DevTools.prototype.renderNotDevMode = function(req,res,next){	
	next();
};


DevTools.prototype.renderIndex = function(req, res, next){

    if(this.utils.isSupportDevelopement(req.ip))
    {
	    var tplIndex = _.template(fs.readFileSync(indexTemplate,'utf8'));

	    var content = tplIndex({
	        mem     : process.memoryUsage(),
	        service : this.service.config.service,
	        logging : {
	            level : this.service.logger.level,
	            levels : ["error","warn","info","verbose","debug", "silly"]
	        },
	        allowAllContent : this.device.allowAllContent,
	        device  : this.service.device.attributes
	    });


	    res.set('Content-Type', 'text/html');
	    res.end(content);
 	}
   else
   	{
		this.renderNotDevMode(req,res,next);
   	}
   
};


DevTools.prototype.updateSettings = function(req, res, next){
    this.logger.verbose('captcha sent : '+req.body.captchaAnswer + ', correct : '+this.captchaAnswer);
    if(parseInt(req.body.captchaAnswer) === this.captchaAnswer){
        var message = 'Settings updated!';
        if(req.body.logLevel){
            this.service.logger.setLogLevel(req.body.logLevel);
            message += '\n - log level set to '+ req.body.logLevel;
        }
        if(req.body.allowAllContent !== null){
            this.device.allowAllContent = req.body.allowAllContent === "true";
            message += '\n - allow all content set to '+ this.device.allowAllContent;
        }
        this.logger.info(message);
        res.send(message);

    }else{
        res.send(403,'Not Allowed');
    }

};

DevTools.prototype.renderLogs = function(req, res, next){

	if(this.utils.isSupportDevelopement(req.ip))
	{
	    if(!wsLogServer) this.createWebSocketServer();
	    res.sendfile(path.join(__dirname,'./templates/logs.html'));
	}
	else
	{
		this.renderNotDevMode(req,res,next);
	}
};

DevTools.prototype.renderTool = function(req, res, next){
	if(this.utils.isSupportDevelopement(req.ip))
	{
	res.sendfile(path.join(__dirname,'./templates/tool.html'));
	}
	else
	{
		this.renderNotDevMode(req,res,next);
	}

};


DevTools.prototype.renderCaptcha = function(req, res, next){
    this.captchaAnswer = parseInt(Math.random()*9000+1000);
    var p = new captcha(80,30,this.captchaAnswer);
    p.color(0, 0, 0, 0);
    p.color(80, 80, 80, 255);

    var img = p.getBase64();
    var imgBase64 = new Buffer(img,'base64');
    res.set('Content-Type', 'image/png');
    res.end(imgBase64);

};


module.exports = DevTools;
