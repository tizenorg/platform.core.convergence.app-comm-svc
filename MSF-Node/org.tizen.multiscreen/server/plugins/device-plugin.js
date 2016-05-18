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
var util = require('util');
var _ = require('lodash');
var os = require("os");
var MSFPlugin = require('./plugin');

function DevicePlugin(config){

    // Set all plugins to be event emitters
    MSFPlugin.call(this);

    this.attributes = {
        type                : 'Multiscreen Device',
        duid				: '',
        model				: os.type()+' '+os.arch(),
        modelName			: os.type()+' '+os.arch(),
        description			: os.type()+' '+os.arch()+' on '+os.cpus()[0].model,
        networkType			: "wireless",
        ssid				: '',
        ip                  : this.utils.getInternalIP(),
        firmwareVersion     : os.release(),
        name                : os.hostname(),
        id                  : '',
        udn                 : '',
        resolution          : '1920x1080',
        countryCode			: 'US',
        msfVersion			: this.service.version,
		smartHubAgreement   : 'false'
    };
    DevicePlugin.verifyImplementation(this);

}

/* Extend the base plugin */
MSFPlugin.extend(DevicePlugin);

/*
 Static Members
 */

DevicePlugin.extend = function(targetClass){
    util.inherits(targetClass, DevicePlugin);
};


DevicePlugin.verifyImplementation = function(canidate){

    var members = {
        methods : [
            "getApplicationData",
            "getApplication",
            "getPartyMode",
            "launchApplication",
            "terminateApplication",
            "installApplication",
            "getWebApplication",
            "launchWebApplication",
            "terminateWebApplication",
            "requestRemoteControl",
            "requestACLPairing"
        ]
    };

    for(var i=0; i<members.methods.length; i++){
        var method = members.methods[i];
        var eMsg = "";
        if(!canidate[method]){

            eMsg = this.name + " is missing required method "+method;
            canidate.logger.error(eMsg);
            throw eMsg;

        }else if(canidate[method].length !== 2){

            eMsg = this.name + " has invalid signature for method "+method;
            canidate.logger.error(eMsg);
            throw eMsg;

        }
    }

};

/*
 Public Members
 */

DevicePlugin.prototype.register = function(){
    this.device.registerDevice(this);
};

DevicePlugin.prototype.updateAttributes = function(attributes){
    var keys = _.keys(attributes);
    var currentAttributes = _.pick(this.attributes,keys);

    if(!_.isEqual(currentAttributes, attributes)){
        _.extend(this.attributes,attributes);
        this.logger.info("Device attributes changed : ", attributes);
        this.device.emit('change', attributes);

    }
};





module.exports = DevicePlugin;