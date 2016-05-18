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
var winston = require('winston');
var config = require('../config');

var logger = new (winston.Logger)({
    transports : [
        new (winston.transports.Console)({
            level : config.logging.level,
            /*prettyPrint : true,*/
            colorize: 'true',
            label: 'MultiScreen',
            handleExceptions: config.logging.handleExceptions
        }),
        new (winston.transports.File)({
            level : config.logging.level,
            colorize: 'true',
            label: 'MultiScreen',
            filename : '/tmp/multiscreen.log',
            maxsize : 10000000,
            maxFiles : 3,
            handleExceptions: config.logging.handleExceptions
        })
    ],
    exitOnError : config.logging.exitOnError
});

logger.level = config.logging.level;

logger.setLogLevel = function(level){
    logger.level = level;
    _.each(logger.transports, function(transport){
        transport.level = level;
    });
};

logger.createNamedLogger = function(name){
    var namedLogger = {};

    // Extend the logger
    logger.extend(namedLogger);

    // Augment the logging methods to prefix the name
    Object.keys(winston.levels).forEach(function (method) {
        namedLogger[method] = function () {
            var args = Array.prototype.slice.call(arguments);
            args.unshift('['+name+']');
            return logger[method].apply(namedLogger, args);
        };
    });

    return namedLogger;

};

module.exports = logger;