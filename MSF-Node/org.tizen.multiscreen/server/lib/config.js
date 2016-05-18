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
var pkg = require('../package.json');
var path = require('path');
var localConfig;

try{
    localConfig = require(path.join(__dirname,'../config.json'));
}catch(e){
    console.warn("[MultiScreen] No service config found! ... using default");
}

var configuration = {

    allowAllContent : false,

    logging : {

        level : 'debug',
        handleExceptions : true,
        exitOnError : false
    },

    service : {
        version     : pkg.version,
        port        : '8001',
        secureport  : '8002',
        apiVersion  : '2.0',
        remoteVersion: '1.0'

    },

	developement : {
		os : "default",
    	developerMode : '0' ,
		developerIP : '0.0.0.0'
	},

    plugins : {}
};

if(localConfig){
    _.merge(configuration, localConfig);
}

console.info("[MultiScreen] config : ", configuration);

module.exports = configuration;