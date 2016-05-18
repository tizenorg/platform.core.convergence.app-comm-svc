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
var IpcClient = require('./lib/IpcClient');
var MSFDevicePlugin = require('../device-plugin');
var async = require('async');
var createError = require('http-errors');
var config = require("../../lib/config");

/*
 Constants
 */

var WEB_LAUNCHER_ID = 'org.tizen.browser';
var IPC_PCS_PATH    = '/tmp/pcs';
var IPC_MSF_PATH    = '/tmp/msf';


var deviceAttributeMapping = {
	DeviceName: 'name',
	DUID: 'duid',
    WifiMac: 'wifiMac',
	Model: 'model',
	NetworkType: 'networkType',
	SSID: 'ssid',
	IP: 'ip',
	FirmwareVersion: 'firmwareVersion',
	CountryCode: 'countryCode',
	DeviceID: 'id',
	ModelDescription: 'description',
	ModelName: 'modelName',
	UDN: 'udn',
	Resolution: 'resolution',
	DeveloperMode: 'developerMode',
	DeveloperIP: 'developerIP',
	SmartHubAgreement: 'smartHubAgreement'
};
var player = false;

function Tizen(config){

    /*
    Call the super constructor
    */
    MSFDevicePlugin.apply(this, arguments);

    this.currentWebAppUrl = null;
    this.isContents = null;
    this.attributes.type = 'Samsung SmartTV';

    /*
     IPC client interfaces
     TODO : migrate device info
     */
    this.deviceClient = new IpcClient(IPC_PCS_PATH);
    this.ipcClient = new IpcClient(IPC_MSF_PATH);

    //this.tizenAddonManager = new TizenAddon.TizenAddonManager();
    //this.tizenAddonManager.InitializeAddon();

    /*
     * Setup route handlers
     */
    
    // This is called on network/name changes from Tizen firmware
    this.service.app.post('/ms/1.0/device/info/update',function(req, res){
        this.logger.debug('DeviceInfoUpdate::');
        this.refreshAttributes();
        res.send('Device info update request received');
    }.bind(this));

    /*
     Set the initial attributes from IPC
     */
    this.getDeviceAttributes(function(err, attributes){
        _.assign(this.attributes,attributes);
		this.setDevelopementMode();	
    }.bind(this));

    /*
     Delay the device registration by 10 seconds
     */
    setTimeout(this.register.bind(this), 5000);
    
}

MSFDevicePlugin.extend(Tizen);

Tizen.prototype.getDeviceAttributes = function(callback){
    this.logger.debug('GetDeviceAttributes::start');
    this.deviceClient.transceive('getinfo', function(err, result){
        if (!err){
            try{
                result = JSON.parse(result);
                var data = {};
                _.each(result, function(val, key){
                    if (_.has(deviceAttributeMapping, key)){
                        data[deviceAttributeMapping[key]] = val;
                    } else {
                        data[key] = val;
                    }
                });
                this.logger.debug('GetDeviceAttributes::complete');
                callback(null,data);
            }catch(err){
                this.logger.error(err);
                callback(err);
            }
        }
    }.bind(this));
};


Tizen.prototype.refreshAttributes = function(){
    this.logger.debug('RefreshAttributes::');
    this.getDeviceAttributes(function(err, attributes){
        if(err) {
            return this.logger.error('unable to refresh attributes', err.message);
        }
        this.updateAttributes(attributes);
    }.bind(this));

};

Tizen.prototype.registerTizenLog = function(){

    this.service.logger.on('logging', function(transport, level, msg, meta)
    {
	    if(transport.name === 'file')
        {
            TizenAddon.TizenLog("[" + level + "] " + msg + JSON.stringify(meta));
	    }
    });
};

Tizen.prototype.setDevelopementMode = function(){

	config.developement.os = "Tizen";
	config.developement.developerMode = this.attributes.developerMode;
	config.developement.developerIP = this.attributes.developerIP;
	this.service.remoteVersion = config.service.remoteVersion;
    this.attributes.OS = config.developement.os;
};

Tizen.prototype.getApplication = function(options, cb){

    var params = {  appId : options.id , url:options.url};

	this.ipcClient.callRPC( 'application.getApplication', params, function(err, result){

        // intercept the callback to convert appId to id and running string to bool

        if(!err && result){
            result.id = result.appId;
		if(result.id === "3201412000694")
          {
				result.media_player = player;
          }
         result.running = (result.running === "true" || result.running === true);
         delete result.appId;
        }

        cb(err,result);
    });
};



Tizen.prototype.getPartyMode = function(options, cb){

    var params = {  appId : options.id };
    this.logger.verbose("getPartyMode");

	this.ipcClient.callRPC( 'application.partyMode', params, function(err, result){

        // intercept the callback to convert appId to id and running string to bool

        if(!err && result){
//            result.id = result.appId;
//            result.running = (result.running === "true" || result.running === true);
//            delete result.appId;
            result.id = result.appId;
            result.running = (result.running === "true" || result.running === true);
			result.partyMode = (result.partyMode === "true" || result.partyMode === true);
            delete result.appId;
        }

        cb(err,result);
    });
};

Tizen.prototype.getApplicationData = function(options, cb){

    cb(null,{url:this.currentWebAppUrl, isContents:this.isContents});
};

Tizen.prototype.launchApplication = function(options, cb){

    this.logger.info("launchApplication : ", options);

    var params = { appId : options.id, clientIp : options.clientIp, deviceName : options.deviceName, data : options.data, url : options.url, isContents : options.isContents, testmode:options.testmode};

    this.ipcClient.callRPC( 'application.launchApplication', params, function(err, result){

        // intercept the callback to convert string to bool

        if(!err && result != null){
            result = (result === "true" || result === true);
        }

        cb(err,result);

    }.bind(this));

};

Tizen.prototype.terminateApplication = function(options, cb){

    var params = { appId : options.id, url : options.url, isContents : options.isContents};

    this.ipcClient.callRPC( 'application.stopApplication', params, function(err, result){

        // intercept the callback to convert string to bool

        if(!err && result != null){
            result = (result === "true" || result === true);
        }

        cb(err,result);
    });

};

Tizen.prototype.installApplication = function(options, cb){

    var params = { appId : options.id };

    this.ipcClient.callRPC( 'application.installApplication', params, function(err, result){

        // intercept the callback to convert string to bool

        if(!err && result != null){
            result = (result === "true" || result === true);
        }

        cb(err,result);
    });
};


Tizen.prototype.getWebApplication = function(options, cb){
    this.getApplication({id:WEB_LAUNCHER_ID, url:options.url}, cb);
};

Tizen.prototype.launchWebApplication = function(options, cb){

    this.logger.info('launchWebApplicaiton', options);

    var self = this;
    var params = {id : WEB_LAUNCHER_ID, url:options.url, clientIp : options.clientIp,deviceName : options.deviceName, isContents:options.isContents, testmode : options.testmode};
    
    this.getWebApplication(params, function(err, appInfo){
    	if(err) return cb(err);
   
	    if(appInfo.running && (options.url != self.currentWebAppUrl)) {
				self.terminateWebApplication({id:WEB_LAUNCHER_ID, url:self.currentWebAppUrl, isContents:player}, function(err, result){});
			}
			
			// We have to delay the launch request or IPC will still think it is running
	    setTimeout(function(){        
	        self.launchApplication(params, function(err, result){
	            if(err) {
	            	self.currentWebAppUrl = null;
	            } else {
	            	self.currentWebAppUrl = options.url;
	        			self.isContents = options.isContents;
	            }
	            cb(err, result);
	        });
	    },1000);
	
			if(options.isContents){
				player = true;
			} else{
				player = false;
			}
	});
};

Tizen.prototype.terminateWebApplication = function(options, cb){
    this.terminateApplication({id:WEB_LAUNCHER_ID, url:options.url, isContents:player}, cb);
	player = false;
};

Tizen.prototype.requestRemoteControl = function (options, cb) {

    var params = { TypeOfRemote: options.TypeOfRemote, Cmd: options.Cmd, DataOfCmd: options.DataOfCmd, Option: options.Option };

    this.ipcClient.callRPC('application.remoteControl', params, function (err, result) {

        // intercept the callback to convert appId to id and running string to bool

        if (!err && result) {
            result.id = result.appId;
            delete result.appId;
        }

        cb(err, result);
    });
};

Tizen.prototype.requestACLPairing = function(options, cb){

    var params = { clientIp : options.clientIp, deviceName : options.deviceName};
    this.logger.verbose("requestACLPairing");

	this.ipcClient.callRPC( 'application.aclPairing', params, function(err, result){

        if(!err && result != null){
            result = (result === "true" || result === true);
        }

        cb(err,result);
    });
};

module.exports = Tizen;
