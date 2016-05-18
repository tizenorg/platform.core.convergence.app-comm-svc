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
var EventEmitter = require('eventemitter2').EventEmitter2;
var logger = require('../logging/index').createNamedLogger('Device');
var request = require('request');
var createError = require('http-errors');
var msfUtils = require('../utils/index');
var async = require('async');

var LAUNCHER_PROXY_URL = 'http://multiscreen.samsung.com/launchservice/v2/launch';

var device = new EventEmitter();
device.lastLaunchData = {};
device.allowAllContent = false;

/*
 A private reference to the underlying device implementation
 set by the registerDevice method
*/
var _device;


/*
 Use a getter for attributes so that we can see errors
 if anything is accessing them before registerDevice is called;
 */
Object.defineProperty(device, 'attributes', {
    get: function() {
        if(!_device) logger.warn('Access of device attribute before device implementation was registered');
        return _device.attributes;
    }
});


/*
 Registers a device implementation (typically a device plugin)
 that handles the underlying implementation for providing attributes about the device
 The preferred way to register a device implementation is using the device plugin base
*/

device.registerDevice = function(impl){
    if(!_device){
        _device = impl;
        logger.info(impl.constructor.name + " is now registered and ready");
        device.emit('ready');
    }
    else logger.error('The device ('+impl.constructor.name+') has already been registered');
};

/*
  Applications
*/

device.getApplication = function(options, callback){
    logger.verbose("getApplication", options);
    _device.getApplication(options, callback);
    device.emit('getApplication', options);
};

device.getPartyMode = function(options, callback){
    logger.verbose("getPartyMode", options);
    _device.getPartyMode(options, callback);
    device.emit('getPartyMode', options);
};
device.getApplicationData = function(options, callback){
    logger.verbose("getApplicationData", options);
    _device.getApplicationData(options, callback);
    device.emit('getApplicationData', options);

};

device.launchApplication = function(options, callback){
    logger.verbose("launchApplication", options);
    _device.launchApplication(options, callback);
    device.emit('launchApplication', options);

};

device.terminateApplication = function(options, callback){
    logger.verbose("terminateApplication", options);
    _device.terminateApplication(options, callback);
    device.emit('terminateApplication', options);
};

device.installApplication = function(options, callback){
    logger.verbose("installApplication", options);
    _device.installApplication(options, callback);
    device.emit('installApplication', options);
};


device.getWebApplication = function(options, callback){
    logger.verbose("getWebApplication", options);
    _device.getWebApplication(options, callback);
    device.emit('getWebApplication', options);

};

device.terminateWebApplication = function(options, callback){
    logger.verbose("terminateWebApplication", options);
    _device.terminateWebApplication(options, callback);
    device.emit('terminateWebApplication', options);
};


device.launchWebApplication = function(options, callback){

		logger.verbose("launchWebApplication", options);

		options.testmode = this.allowAllContent;
	  
	  var inputurl;
    var resultString = options.url.split("//");
    if(options.url.length === resultString[0].length) {
    	inputurl = options.url;
    } else {
    		inputurl = resultString[1];
    }
    inputurl = inputurl.replace("www.","");
    inputurl = inputurl.replace( /(\s*)/g, "" );
    if(inputurl.charAt(inputurl.length - 1) === "/")
    inputurl = inputurl.substr(0,inputurl.length - 1);

	  switch(inputurl)
	  {
	  	case 'dev-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv':
	  	case 'dev-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/index.html':
	  	case 'dev-multiscreen.samsung.com/casteroids/tv/index.html':
	  	case 'fling-tv.herokuapp.com':
	  	case 'google.com':
	  	case 'multiscreen.samsung.com/app-sample-photos/tv/index.html':
	  	case 'multiscreen.samsung.com/casteroids/tv/index.html':
	  	case 'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv':
	  	case 'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/index.html':
	  	case 'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/photoshare/tv':
	  	case 'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/photoshare/tv/index.html':
	  	case 'yahoo.com':
	  		options.testmode = true;
	  		break;
	  }
	 
		_device.launchWebApplication(options, callback);
		device.emit('launchWebApplication', options);

};

device.requestRemoteControl = function (options, callback) {
    logger.silly("requestRemoteControl", options);
    _device.requestRemoteControl(options, callback);
    //device.emit("requestRemoteControl", options);
};

device.requestACLPairing = function (options, callback) {
    logger.verbose("requestACLPairing", options);
    _device.requestACLPairing(options, callback);
    device.emit("requestACLPairing", options);
};

module.exports = device;
