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
var _ = require("lodash");
var logger = require('../../../lib/logging');
var net = require('net');
var createError = require('http-errors');
var uuid   = require("node-uuid");

var IpcClient = function(path){

    this.ipcPath = path;

    this.send = function(message, callback) {
		if (typeof message === 'object'){
			message = JSON.stringify(message);
		}

        var client = null;

        try{

            var self = this;

            client = net.connect({path: this.ipcPath},
                function() {
                    logger.info('ipc client connected::', self.ipcPath);
                    client.end(message.length + ',' + message);
                }
            );

            client.on('end', function() {
                logger.info('ipc client disconnected::', self.ipcPath);
                callback(null, true);
            });

            client.on('error', function(err){
                logger.error('ipc client error::', self.ipcPath, err.message);
                callback(createError(500));
            });

        }catch(e){

            logger.error(e.message);
            if(client && client.end){
                client.end();
            }
        }
    };

    this.callRPC = function(method, params, callback){

        var message = {
            method : method,
            params : params,
            id     : uuid.v1()
        };

        this.transceive(message, function(err, response){

            if(err) return callback(err);

            try{
                response = JSON.parse(response);

                if(!response.error){

                    logger.debug('IPC callRPC : result : ', response.result);
                    return callback(null, response.result);

                }else {

                    logger.error('IPC callRPC : error : ', response.error);

                    var code = parseInt(response.error.code, 10);

                    // TODO : Finalize IPC error codes
                    switch (code) {
                        case -4009 :
                            // TODO : request change to IPC so that launching/installing app already running/installed returns true
                            //return callback(createError(409));
                            return callback(null, true);
                        case -4004 :
                            return callback(createError(404));
                		case -4001 :
                            return callback(createError(401));
                        default :
                            return callback(createError(500));
                    }
                }

            }catch(e){
                logger.error('IPC callRPC : ', e.message);
                callback(createError(500));
            }

        });
    };

    this.transceive = function(message, callback) {

        logger.debug("IPC transceive called : ", message);

        if (_.isObject(message)){
            logger.debug("Stringifying message");
            try{
                message = JSON.stringify(message);
            }catch(e){
                return callback(400,"Unable to parse to JSON");
            }
        }


        var client = null;
        var callbackFired = false;

        try{

            var self = this;
            var response = null;

            client = net.connect({path: this.ipcPath}, function() {
                logger.debug('IPC client connected :', self.ipcPath);
                try {
                    client.write(message.length + ',' + message);
                } catch (err) {
                    logger.error("IPC client client.write - FAILED", err);
                    client.removeAllListeners();
                    client.destroy();
                }
            });



            var timeout = setTimeout(function(){
                if(client)client.end();
                return callback(createError(500, 'Timeout in IPC request'));
            }, 65000);



            client.on('error', function(err){
                logger.error('ipc on error :', err.message);
                if(client)client.end();
                return callback(createError(500, 'unknown IPC error'));
            });



            client.on('end', function() {

                logger.debug('ipc client on end');
                if(timeout)clearTimeout(timeout);

            });

            client.on('close', function() {

                logger.debug('ipc client on close');

            });

            client.on('data', function(data) {

                logger.debug('ipc client on data');

                if(data) {

                    data = data.toString();

                    logger.debug('ipc data = ', data);

                    try {

                        var cPos = data.indexOf(',');
                        var len = parseInt(data.substr(0, cPos),10);
                        var result = data.substring(cPos + 1, cPos + 1 + len);

                        logger.silly('ipc on data : cPos : ', cPos);
                        logger.silly('ipc on data : len : ', len);
                        logger.silly('ipc on data : result : ', result);

                        client.end();

                        if(callbackFired){
                            logger.warn('IPC data : callback was already fired!!!');
                            return;
                        }

                        callbackFired = true;
                        return callback(null, result);



                    } catch (e) {

                        client.end();

                        logger.error(e.message);

                        if(callbackFired){
                            logger.warn('IPC data : callback was already fired!!!');
                            return;
                        }

                        callbackFired = true;
                        return callback(createError(500, 'unable to parse IPC response'));


                    }
                }else{
                    client.end();

                    if(callbackFired){
                        logger.warn('IPC data : callback was already fired!!!');
                        return;
                    }

                    callbackFired = true;
                    return callback(createError(500, 'IPC response data was empty'));

                }

            });

        }catch(e){

            logger.error("IPC client - FAILED: ", e.message);

            if(client && client.end){
                client.end();
                return callback(createError(500, 'unknown IPC error'));
            }

        }


    };
};

module.exports = IpcClient;
