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
var util = require('util');
var EventEmitter = require('eventemitter2').EventEmitter2;
var uuid = require('node-uuid');
var url = require("url");

/* TODO : Need to clean up these path dependencies... perhaps a global msfRootRequire method */
var service = require('../../../lib/multiscreen-service');
var WebSocketServer = require('../../../lib/ws/index').Server;

var logger = service.logger.createNamedLogger('Channel v2');
var device = service.device;
var EventBus = service.events;
var utils = service.utils;

var timePingPong = 5000;


function Channel(path, httpServer){

    logger.info("Creating channel", path);

    this.session = {
        channelSession    : uuid.v1(),
        path              : path,
        createdTime       : Date.now(),
        get duration () {
            return Date.now() - this.createdTime;
        }
    };

    this.path = path;
    this.clientConnections = {};
    this.clients = {};
    this.host = null;
    this.server = new WebSocketServer({server: httpServer, path: path});

    this.server.on('error', this.onServerError.bind(this));
    this.server.on('connection', this.onSocketConnection.bind(this));

    EventBus.emit('channel:session:start', this.session);
	
}

util.inherits(Channel, EventEmitter);


Channel.prototype.handleProtocols = function(protocols, callback){
    // In future versions more logic will need to be added here
    // For now always default to msf-2
    callback(true, 'msf-2');
};


Channel.prototype.isMaxConnections = function(){

    return this.server.clients.length >= 50;

};

Channel.prototype.onServerError = function(err){

    var message = err.message;
    logger.error(message);

};


Channel.prototype.onSocketError = function(socket, err){

    var message = err.message;
    logger.error(message);

};


Channel.prototype.onSocketConnection = function(socket){

    // Tick up the channels connection count
    socket.session = {
        channelSession  : this.session.channelSession,
        txMessages      : 0,
        rxMessages      : 0,
        txMessageSize   : 0,
		createdTime       : Date.now(),
        userAgent       : socket.upgradeReq.headers['user-agent'],
        get duration () {
            return Date.now() - this.createdTime;
        }
    };

    /*
     *  Test if we are over the limit and disconnect the socket if so
     */
    if(this.isMaxConnections()){
        socket.close(4000,"Maximum number of connections");
        return;
    }


    /*
     *  Create client object with an id and connect time
     */

    var id = uuid.v1();
    var client = {id:id, connectTime:Date.now()};


    // TODO : Improve me
    // hate to store the id on the websocket but is the easiest way for now
    socket.id = id;
    socket.ip = utils.checkWhetherIpv6(socket.req[0].connection.remoteAddress);

    
    /*
     *  Copy all query string attributes to the client attributes object
     */
    client.attributes = url.parse(socket.upgradeReq.url, true).query;
	client.deviceName = typeof client.attributes.name === 'undefined' ? "Smart Device" : client.attributes.name;
	socket.deviceName = client.deviceName;
	
    /*
     *  Is this the tv connecting?
     */
    if(socket.upgradeReq.headers['host'].indexOf('127.0.0.1') === 0){
        this.host = socket;
        client.isHost = true;
        logger.debug("TV has connected");
    }

    /*
     *  This is a client
     */
    else{
        client.isHost = false;
        logger.debug("Client has connected");
    }

    /*
     *  Add the connection and info to the maps
     */
    this.clientConnections[id] = socket;
    this.clients[id] = client;

    EventBus.emit('channel:client:session:start', socket.session);

    /*
     *  Add our listeners to the socket
     */
    socket.on('error', this.onSocketError.bind(this, socket));
    socket.on('close', this.onSocketClose.bind(this, socket));
    socket.on('message', this.onSocketMessage.bind(this, socket));


    /*
    *	register Ping Pong for socket connection.
    */
    this.registerPingPong(socket);
	
    /*
     *  Notify the client he is connected providing the id and client list
     *  TODO : Make this more efficient : currently converting clients Map to Array
     */

    var clientList = _.map(this.clients, function(client){
        if(client){
            return client;
        }else{
            console.warn("Found null client in list");
            return void 0;
        }

    });

    /*
     *  Notify the client he is connected and give him his id and the current client list
     */
    var msgConnect = {
        event : "ms.channel.connect",
        data : {
            id:client.id,
            clients:clientList
        }
    };
    this.send(msgConnect,socket);

    /*
     *  Notify everyone else the client connected
     */
    var msgClientConnect = {
        event : "ms.channel.clientConnect",
        data : client
    };
    this.broadcast(msgClientConnect,socket);


    var msgReady = {
        event : "ms.channel.ready",
        data : {}
    };
    /*
     *  If the host (tv) is already connected then send the read event to the client
     */
    if(this.host){
        this.send(msgReady,socket);
    }
    /*
     *  If the client is the host then broadcast to everyone that we are ready
     */
    if(client.isHost){
        this.broadcast(msgReady,socket);
    }


};

Channel.prototype.registerPingPong = function(socket){

    socket.numberNotAck = 0;
    socket.intervalPingPong = setInterval(function(){

	if(socket.numberNotAck >= 2)
        {
           logger.warn("MSF can't get ACK during 10 sec from client ",socket.id);
           socket.close();
        }
        else
        {
           socket.ping();
           socket.numberNotAck++;
        }

    },timePingPong);

    socket.on('pong',function(){socket.numberNotAck = 0; });

};

Channel.prototype.encodeMessageToBuffer = function(oMsg, payload){

    var mStr = JSON.stringify(oMsg);
    var mBuff = new Buffer(mStr, 'utf8');

    var hBuff = new Buffer(2);
    hBuff.writeUInt16BE(mBuff.length, 0);

    return Buffer.concat([hBuff,mBuff,payload]);

};


Channel.prototype.decodeMessageFromBuffer = function(buf){

    var mSize = buf.readUInt16BE(0);
    var mStr = buf.toString('utf8',2,2+mSize);

    var message = JSON.parse(mStr);
    var payload = buf.slice(2+mSize);

    return {message: message, payload: payload};

};

Channel.prototype.createMessageResponder = function(socket, msg){

    if(!msg.id){
        // Since this message has no id create a noop responder
        return function(){};
    }

    return function(err, result){

        var e = {};
        var resMsg = {id : msg.id };

        if(err){

            if(err.status && err.status < 500){
                e.status    = err.status || err.statusCode || 500;
                e.message   = err.message || 'Error '+ e.message;
                e.code      = e.code || e.status;
                e.details   = err.details || void 0;
            }else{
                e.status    = 500;
                e.message   = 'Internal Server Error';
                e.code      = e.status;
            }

            resMsg.error = e;


        }else{
            resMsg.result = result;
        }

        socket.send(JSON.stringify(resMsg),function(err){
            if(err) logger.warn('socket is closed, unable to send message : ',resMsg);
        });
    };

};


Channel.prototype.onSocketMessage = function(socket, msg, flags){

    var msgByteLen = 0;

    if (flags.binary) {
        logger.silly('message from ('+socket.id+') : binary');
        msgByteLen = msg.length;
    }else{
        logger.silly('message from ('+socket.id+') : ' , msg);
        msgByteLen = Buffer.byteLength(msg);
    }


    // Add some data to the session
    socket.session.txMessages++;
    socket.session.txMessageSize += msgByteLen;

    // Ensure the message is below max
    if(msgByteLen > 10000000){
        logger.warn('Socket Message : byte length exceeds max');
        return this.sendErrorEvent(socket, 'Socket Message : byte length exceeds max');
    }


    // Let's try to parse the message
    try{

        var payload;

        if(flags.binary){

            // TODO : Handle blobs
            var decoded = this.decodeMessageFromBuffer(msg);
            payload = decoded.payload;
            msg = decoded.message;

        }else{

            msg = JSON.parse(msg);
			msg.params.clientIp = socket.ip;
			msg.params.deviceName = socket.deviceName;

        }

        if(!msg.method){

            logger.warn('missing method field from message', msg);
            return this.sendErrorEvent(socket, 'missing method field from message');

        }

        switch(msg.method){

            case 'ms.channel.emit':
                this.emitMessage(socket, msg, payload);
                break;
            case 'ms.application.get':
                device.getApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            case 'ms.application.stop':
                device.terminateApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            case 'ms.application.install':
                device.installApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            case 'ms.application.start':
                device.launchApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            case 'ms.webapplication.get':
                device.getWebApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            case 'ms.webapplication.stop':
                device.terminateWebApplication(msg.params, this.createMessageResponder(socket, msg));
                this.onSocketClose(socket);
                break;
            case 'ms.webapplication.start':
                device.launchWebApplication(msg.params, this.createMessageResponder(socket, msg));
                break;
            default :
                logger.warn('unrecognized method value : ', msg);
                this.sendErrorEvent(socket, 'unrecognized method value : '+msg.method);
        }


    }catch(e){
        // Fail silently
        logger.error("unable to handle message : " +  e.message);
        this.sendErrorEvent(socket, "unable to handle message : " + e.message);
    }

};

Channel.prototype.emitMessage = function(socket, msg, payload){

    var targetId = msg.params.to;
    var event = msg.params.event;
    var data = msg.params.data;

    if(event.indexOf('ms.') === 0){
        this.sendErrorEvent(socket, 'Usage of `ms.` in custom event is not allowed. Perhaps use a alternative namespace.');
        return;
    }

    var notification = {
        event   : event,
        data    : data,
        from    : socket.id
    };

    var options = {};

    if(payload){
        notification = this.encodeMessageToBuffer(notification, payload);
        options.binary = true;
    }

    if(targetId === "all"){
        this.broadcast(notification, null, options);
    }else if(targetId === "broadcast"){
        this.broadcast(notification, socket, options);
    }else if(targetId === "host"){
        this.send(notification, this.host, options);
    }else if(typeof targetId === "string" && this.clientConnections[targetId]){
        this.send(notification,this.clientConnections[targetId], options);      
    }else if(_.isArray(targetId)){
        targetId.forEach(function(id){
            if(this.clientConnections[id]){
                this.send(notification,this.clientConnections[id], options);
            }
        }, this);
    }else{
        logger.warn('unrecognized `to` value : ', msg);
        this.sendErrorEvent(socket, 'unrecognized `to` value : '+targetId);
    }

};

Channel.prototype.onSocketClose = function(socket){

    if(this.clientConnections &&
        this.clients && socket &&
        this.clients[socket.id]){

        var client = this.clients[socket.id];

        EventBus.emit('channel:client:session:end', socket.session);

        try{
            var notification = {
                event : 'ms.channel.clientDisconnect',
                data : client
            };
            this.broadcast(notification,socket);

            if(client.isHost){
                delete this.host;
                logger.debug('TV has disconnected');
            }else{
                logger.debug("Client has disconnected");
            }
            clearInterval(socket.intervalPingPong);
            delete this.clientConnections[socket.id];
            delete this.clients[socket.id];

        }catch(e){
            // Just a fail safe as there are times when multiple connections are closing at once
            logger.warn("Issue sending clientDisconnect notification to client : ", e.message);
        }
    }

};

Channel.prototype.send = function(msg, socket, options){

    options = options || {};
    socket.session.rxMessages++;

    try{
        if(options.binary){
            socket.send(msg, options);
        }else{
            socket.send(JSON.stringify(msg), options);
        }

    }catch(e){

        logger.error(e.message);

    }

};


Channel.prototype.broadcast = function(msg, excludedClient, options){

    this.server.clients.forEach(function(client){
        if(client !== excludedClient){
            this.send(msg, client, options);
        }
    }, this);

};

Channel.prototype.sendErrorEvent = function(socket, message){

    return socket.send(JSON.stringify({
        event : 'ms.error',
        data  : {
            message : message || ""
        }
    }));

};


Channel.prototype.shutDown = function(){

    logger.warn("Shutting down channel", this.path);

    // Clear the test interval
    clearTimeout(this.activeTimeout);

    delete this.activeTimeout;
    delete this.host;
    delete this.clientConnections;
    delete this.clients;

    if(this.server){
        this.server.close(4000);
        delete this.server;
    }

    EventBus.emit('channel:session:end', this.session);
    this.emit("shutDown");

    this.removeAllListeners();

};


module.exports.Channel = Channel;
