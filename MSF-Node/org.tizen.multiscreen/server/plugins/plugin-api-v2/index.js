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
var util = require('util');
var url = require('url');
var Channel = require('./channels/index').Channel;
var request = require('request');
var createError = require('http-errors');
var MSFPlugin = require('../plugin');
var service = require('../../lib/multiscreen-service');
var utils = service.utils;
var serviceJson = require('../service.json');

/*
 Constants
 */
var URI_PATH = '/api/v2/';
var REMOTE_CHANNEL = '/api/v2/channels/samsung.remote.control';

/*
 Utility function for create REST callback responders from route handlers
 */
function createCallbackResponder(req, res, next){

    return function(err, result){

        if(err){
            return next(err);
        }else{
            // ensure standardized json
            if(_.isBoolean(result)) result = {ok:result};
            res.send(result);
        }
    };
}

function createCallbackPartyMode(req, res, next){

    return function(err, result){
        if(err){
            return next(err);
        }else{
            // ensure standardized json
            if(_.isBoolean(result.partyMode) && result.partyMode === true) result = {running : result.running, partymode_enable : true};
            else{
                result = {running : result.running, partymode_enable : false};
            }
            res.send(result);
        }
    };
}


function ApiV2(config){

    /* Call the super constructor */
    MSFPlugin.apply(this, arguments);

    this.channels = {};

    // REST API : Service
    this.app.get(URI_PATH,this.getServiceInfo.bind(this));
    
    // REST API : Party Mode

    this.app.get(URI_PATH + 'applications/org.volt.mycontents',this.getPartyMode.bind(this));
    

    // REST API : Application
    this.app.get(URI_PATH + 'applications/:id',this.getApplication.bind(this));
    this.app.post(URI_PATH + 'applications/:id',this.launchApplication.bind(this));
    this.app.delete(URI_PATH + 'applications/:id',this.terminateApplication.bind(this));
    this.app.put(URI_PATH + 'applications/:id',this.installApplication.bind(this));

    // REST API : Web Application
    this.app.get(URI_PATH + 'webapplication/',this.getWebApplication.bind(this));
    this.app.post(URI_PATH + 'webapplication/',this.launchWebApplication.bind(this));
    this.app.delete(URI_PATH + 'webapplication/',this.terminateWebApplication.bind(this));

    // TODO : Remove this once we can pass parameters to all devices including Tizen
    this.app.get(URI_PATH + 'webapplication/data',function(req, res, next){
        this.device.getApplicationData(null, createCallbackResponder(req, res, next));
    }.bind(this));

    this.configureChannels();

}


MSFPlugin.extend(ApiV2);

/*
 Sets up the channel container and creates channels on the fly for upgrade requests
 */
ApiV2.prototype.configureChannels = function(){
    var self = this;

    this.server.on('upgrade',function(req, socket, upgradeHead){

        var u = url.parse(req.url);

        /*
         * Check is it is a channel request
         */

        // Verify that the url path matches this api path plus `channels`
        if(u && u.pathname.indexOf(URI_PATH+'channels') === 0){

            var path = u.pathname;

            if(!self.channels[path] && path!=REMOTE_CHANNEL){

                var channel = self.channels[path] = new Channel(path, self.server);

                if(channel){

                    channel.server._onHandleUpgrade(req, socket, upgradeHead);
                    // If the server shutsDown remove it from the mapped channels
                    channel.on("shutDown", function(){
                        delete self.channels[path];
                        self.channels[path] = void 0;
                    });

                }else{
                    socket.end();
                }
            }
        }

    });

};



ApiV2.prototype.getServiceInfo = function(req, res, next){

    this.logger.verbose("getServiceInfo");

    var self = this;

    var infoHandler = function(err, callback){
        var serviceType = JSON.stringify(serviceJson);
        var info = {
            id      : self.device.attributes.id,
            name    : self.device.attributes.name,
            version : self.service.version,
            device  : self.device.attributes,
            type    : self.device.attributes.type,
            uri     : "http://"+ req.get('host') + URI_PATH,
            remote  : self.service.remoteVersion,
            isSupport : serviceType
        };
        callback(null,info);
    };

    infoHandler({}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.getApplication = function getApplication(req, res, next){
    this.logger.verbose("getApplication : ", req.params.id);
    this.device.getApplication( {id:req.params.id}, createCallbackResponder(req, res, next));
};


ApiV2.prototype.getPartyMode = function getPartyMode(req, res, next){
    this.logger.verbose("getPartyMode");
    this.device.getPartyMode( {id: "org.volt.mycontents"}, createCallbackPartyMode(req, res, next));
};

ApiV2.prototype.launchApplication = function launchApplication(req, res, next){
    this.logger.verbose("launchApplication : ", req.params.id);
    var data = req.body;
    var clientIp = utils.checkWhetherIpv6(req.headers['x-forwarded-for'] || req.connection.remoteAddress);
    this.device.launchApplication({id:req.params.id, clientIp:clientIp, data:data}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.terminateApplication = function stopApplication(req, res, next){
    this.logger.verbose("stopApplication : ", req.params.id);
    this.device.terminateApplication({id:req.params.id}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.installApplication = function installApplication(req, res, next){
    this.logger.verbose("installApplication : ", req.params.id);
    this.device.installApplication({id:req.params.id}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.getWebApplication = function getWebApplication(req, res, next){
    this.logger.verbose("getWebApplication");
    this.device.getWebApplication({url:req.body.url}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.launchWebApplication = function launchWebApplication(req, res, next){
    this.logger.verbose("launchWebApplication", req.body);
    var clientIp = utils.checkWhetherIpv6(req.headers['x-forwarded-for'] || req.connection.remoteAddress);
    this.device.launchWebApplication({url:req.body.url, clientIp:clientIp}, createCallbackResponder(req, res, next));
};

ApiV2.prototype.terminateWebApplication = function stopWebApplication(req, res, next){
    this.logger.verbose("stopWebApplication", req.body);
    this.device.terminateWebApplication({url:req.body.url}, createCallbackResponder(req, res, next));
};




module.exports = ApiV2;
