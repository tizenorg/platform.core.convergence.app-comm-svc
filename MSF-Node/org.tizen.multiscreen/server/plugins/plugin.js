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
var winston = require('winston');
var EventEmitter = require('eventemitter2').EventEmitter2;
var service = require('../lib/multiscreen-service');

function MSFPlugin(config){

    // Set all plugins to be event emitters
    EventEmitter.call(this);

    // Properties available to all plugins
    this.name       = this.constructor.name;
    this.config     = _.defaults(config || {}, this.configDefaults || {});
    this.service    = service;
    this.app        = service.app;
    this.server     = service.server;
    this.device     = service.device;
    this.utils      = service.utils;
    this.logger     = service.logger.createNamedLogger('Plugin : '+this.name);

    this.logger.info('Loaded');
}

MSFPlugin.extend = function(targetClass){
    util.inherits(targetClass, MSFPlugin);
};

util.inherits(MSFPlugin, EventEmitter);

module.exports = MSFPlugin;