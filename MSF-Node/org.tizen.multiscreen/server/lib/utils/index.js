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
var fs = require('fs');
var path = require('path');
var uuid = require("node-uuid");
var logger = require('../logging/index').createNamedLogger('Utils');
var config = require('../config');

function generateDeviceId(){

    var id = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for( var i=0; i < 13; i++ ){
        id += possible.charAt(Math.floor(Math.random() * possible.length));
    }
    return id;
}

function generateUdn(){
    return uuid.v1();
}

function getInternalAddresses(){

    var os = require('os');
    var interfaces = os.networkInterfaces();
    var addresses = [];
    for (var k in interfaces) {
        for (var k2 in interfaces[k]) {
            var address = interfaces[k][k2];
            if (address.family === 'IPv4' && !address.internal) {
                addresses.push(address.address);
            }
        }
    }
    return addresses;
}

function getInternalIP(){
    var addresses = getInternalAddresses();
    return addresses[0];
}


function isInternalIp(ip){
    var addresses = getInternalAddresses();
    addresses.push('127.0.0.1');
    logger.debug('isInternalIp('+ip+') verifying from addresses ', addresses);
    return (addresses.indexOf(ip) > -1);
}

function checkWhetherIpv6(clienIP) {
    var ip4address = clienIP.replace("::ffff:", "");
    return ip4address;
}

function isSupportDevelopement(clientIP)
{
    if(config.developement.os == "Tizen")
    {
		if(config.developement.developerMode == '0')
			return false;
		if(clientIP == config.developement.developerIP)
			return true;

		return false;
    }
    else
    {
		return true;
    }
}

exports.getInternalIP = getInternalIP;
exports.isInternalIp = isInternalIp;
exports.checkWhetherIpv6 = checkWhetherIpv6;
exports.generateDeviceId = generateDeviceId;
exports.generateUdn = generateUdn;
exports.isSupportDevelopement = isSupportDevelopement;
