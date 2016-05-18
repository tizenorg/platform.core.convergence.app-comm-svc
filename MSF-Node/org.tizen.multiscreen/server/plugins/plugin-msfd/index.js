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
var MSFPlugin = require('../plugin');
var dgram = require('dgram');
var uuid = require('node-uuid');

var MULTICAST_ADDRESS   = '224.0.0.7';
var MULTICAST_TTL       = 1;
var MULTICAST_PORT      = 8001;
var MAX_MESSAGE_LENGTH  = 2000;

var MSG_UP              = 'up';
var MSG_ALIVE           = 'alive';
var MSG_DOWN            = 'down';
var MSG_DISCOVER        = 'discover';

var HEARTBEAT_INTERVAL  = 2000;
var RECORD_TTL          = HEARTBEAT_INTERVAL * 4;

var URI_V2 = '/api/v2/';
var URI_V1 = '/ms/1.0/';


function MFSD(config){

    MSFPlugin.apply(this, arguments);

    this.intervalAlive = null;

    this.device.on('ready', this.start.bind(this));

    // TODO : Need to look at other alternatives. This is never going to work to send the dead message
    process.on('exit', this.stop.bind(this));

}

MSFPlugin.extend(MFSD);

MFSD.prototype.start = function(){

    this.logger.info('starting....');
    if(this.socket) return this.logger.warn('attempted to start, already started');
	this.socket = dgram.createSocket({ type: 'udp4', reuseAddr: true });

    this.socket.on('message',this.onMessage.bind(this));
    this.socket.on('error',this.logger.error.bind(this.logger));

    this.socket.bind(MULTICAST_PORT, function(err){
        if(err) return this.logger.error(err.message);
        try {
            this.socket.setMulticastTTL(MULTICAST_TTL);
            this.socket.addMembership(MULTICAST_ADDRESS);
        } catch (err){
            this.logger.error(err);
        }

        // broadcast up
        var msg = this.createMessage(MSG_UP, this.getServices());
        this.socket.send(msg, 0, msg.length, MULTICAST_PORT, MULTICAST_ADDRESS, function (err) {
            if(err) return this.logger.error(err);
            this.logger.silly('sent up....');
        }.bind(this));

    }.bind(this));

    this.intervalAlive = setInterval(this.sendAlive.bind(this), HEARTBEAT_INTERVAL);
};

MFSD.prototype.stop = function(){

    this.logger.info('stopping....');
    clearInterval(this.intervalAlive);
    if(!this.socket) return this.logger.warn('attempted to stop, already stopped');

    var msg = this.createMessage(MSG_DOWN, this.getServices());
    // broadcast down
    this.socket.send(msg, 0, msg.length, MULTICAST_PORT, MULTICAST_ADDRESS, function(err){
        if(err) this.logger.error(err);
        // cleanup
        this.socket.close();
        this.socket.removeAllListeners();
        this.socket = null;
    }.bind(this));
};

MFSD.prototype.sendAlive = function(){

    //this.logger.silly('sending alive');
    var msg = this.createMessage(MSG_ALIVE, this.getServices());
    // broadcast alive
    this.socket.send(msg, 0, msg.length, MULTICAST_PORT, MULTICAST_ADDRESS, function(err, bytes) {
        if(err) return this.logger.error(err.message);
    }.bind(this));
};

MFSD.prototype.createMessage = function(type, data){
    var msg = {
        type: type,
        ttl: RECORD_TTL,
        remote: this.service.remoteVersion,
        sid: this.device.attributes.id,
        data: data || {}
    };
    return new Buffer(JSON.stringify(msg));
};

MFSD.prototype.getServices = function(){
    var base = "http://"+ this.utils.getInternalIP() + ":" + this.service.port;
    return {
        v1 : { uri : base + URI_V1 },
        v2 : { uri : base + URI_V2 }
    };
};

MFSD.prototype.onMessage = function(msg, rinfo) {

    /* Dont attempt to deal with messages over the max */
    if(msg.length > MAX_MESSAGE_LENGTH) return this.logger.warn('ignoring message over limit of '+msg.length);

    /* attempt to parse it */
    try{ msg = JSON.parse(msg);}
    catch(e){ return this.logger.error('unable to parse msg : ',msg.toString()); }

    var sid = this.device.attributes.id;

    /* dont care about my own messages */
    if(msg.sid === sid) return;

    if (msg.type === MSG_DISCOVER) {
        this.logger.verbose("received discovery request from : " + rinfo.address + ":" + rinfo.port);
        // answer here
        var response = this.createMessage(MSG_ALIVE, this.getServices());
        this.socket.send(response, 0, response.length, rinfo.port, rinfo.address, function (err) {
            if(err) return this.logger.error(err);
            this.logger.silly('sent alive to '+rinfo.address + ":" + rinfo.port);
        }.bind(this));

    }

};


module.exports = MFSD;










