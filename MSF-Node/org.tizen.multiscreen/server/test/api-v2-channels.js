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
var WebSocket = require('../lib/ws/index');
var assert = require('assert');
var _ = require('lodash');

if(!global.msfService){
    global.msfService = require('../lib/multiscreen-service');
    global.msfService.logger.transports.console.silent = true;
    global.msfService.start();
}


// service will assume 127.0.0.1 host is the tv so we have 2 different endpoints
var wsHostEndpoint = "ws://127.0.0.1:8001/api/v2/channels/test";
var wsClientEndpoint = "ws://localhost:8001/api/v2/channels/test";

// Used in test for storing websocket refs and client info
var hostSocket;
var hostInfo;
var client1Socket;
var client1Info;
var client2Socket;
var client2Info;


var packMessage = function(oMsg, payload){

    var mStr = JSON.stringify(oMsg);
    var mBuff = new Buffer(mStr, 'utf8');

    var hBuff = new Buffer(2);
    hBuff.writeUInt16BE(mBuff.length, 0);

    return Buffer.concat([hBuff,mBuff,payload]);

};

var unpackMessage = function(buf){

    var mSize = buf.readUInt16BE(0);
    var mStr = buf.toString('utf8',2,2+mSize);

    var message = JSON.parse(mStr);
    var payload = buf.slice(2+mSize);

    return {message: message, payload: payload};

};


describe('MultiScreen Service : API V2 : Channels', function() {


    afterEach(function(done) {
        if (hostSocket) hostSocket.removeAllListeners();
        if (client1Socket) client1Socket.removeAllListeners();
        if (client2Socket) client2Socket.removeAllListeners();
        done();
    });

    it('waits 4 seconds before testing', function(done) {
        this.timeout(5000);
        assert.ok(true, "waiting");
        setTimeout(done,4000);
    });


    it('connects as host', function(done) {

        var endpoint = wsHostEndpoint + "?name=TheHost";

        hostSocket = new WebSocket(endpoint);

        hostSocket.on('error',function (evt) {
            console.error(evt);
        });

        hostSocket.on('open',function (evt) {
            assert.ok(true, "Host socket connected");
        });

        hostSocket.on('message',function (msg) {

            msg = JSON.parse(msg);

            if(msg.event === "ms.channel.connect"){
                assert.ok(_.isString(msg.data.id), "valid client id");
                assert.ok(_.isArray(msg.data.clients), "valid clients array");

                msg.data.clients.forEach(function (info) {
                    if (info.id === msg.data.id) hostInfo = info;
                });
                assert.ok(_.isObject(hostInfo), "contained the host client information");
            }

            if(msg.event === "ms.channel.ready"){
                done();
            }

        });

    });

    it('connects as client1', function(done) {

        var endpoint = wsClientEndpoint + "?name=TheClient";

        client1Socket = new WebSocket(endpoint);

        client1Socket.on('error',function (evt) {
            console.error(evt);
        });

        client1Socket.on('open',function (evt) {
            assert.ok(true, "Client socket connected");
        });

        client1Socket.on('message',function (msg) {
            msg = JSON.parse(msg);

            if(msg.event === 'ms.channel.connect'){
                assert.ok(true, "client got connect event");
                assert.ok(_.isString(msg.data.id), "valid client id");
                assert.ok(_.isArray(msg.data.clients), "valid clients array");

                msg.data.clients.forEach(function (info) {
                    if (info.id === msg.data.id)  client1Info = info;
                });

                assert.ok(_.isObject(client1Info), "contained the client information");

            }else if(msg.event === 'ms.channel.ready'){
                assert.ok(true, "client got ready event");
            }

        });

        hostSocket.on('message',function (msg) {
            msg = JSON.parse(msg);

            assert.equal(msg.event, 'ms.channel.clientConnect', "host received the clientConnect event");
            assert.deepEqual(msg.data, client1Info, "contained the correct client information");

            done();
        });

    });


    it('emit an event with `ms.` prefix (not allowed)', function(done) {

        var message = {
            "method": "ms.channel.emit",
            "params": {
                to: client1Info.id,
                event: 'ms.channel.anything',
                data: 'this should give an error'
            }
        };

        hostSocket.on('message', function (msg) {

            msg = JSON.parse(msg);
            assert.equal(msg.event, 'ms.error', "host received the correct error event");
            done();

        });

        hostSocket.send(JSON.stringify(message));

    });


    it('emits an invalid binary event to client', function(done) {

        hostSocket.on('message', function (msg, flags) {

            msg = JSON.parse(msg);
            assert.equal(msg.event, 'ms.error', "host received the correct error event");
            done();

        });

        var message = {
            "method": "invalid",
            "params": {
                to: client1Info.id,
                event: 'say',
                data: 'hello'
            }
        };

        var payload = new Buffer(100);
        hostSocket.send(packMessage(message, payload));

    });

    it('emits an invalid binary data', function(done) {

        hostSocket.on('message', function (msg) {

            msg = JSON.parse(msg);
            assert.equal(msg.event, 'ms.error', "host received the correct error event");
            done();

        });

        var payload = new Buffer(100);
        hostSocket.send(payload);

    });


    it('emits a binary event', function(done) {

        client1Socket.on('message', function (msg) {

            assert.ok(msg instanceof Buffer, "client received a Buffer");

            var decoded = unpackMessage(msg);

            msg = decoded.message;

            var expected = {
                event: "say",
                from: hostInfo.id,
                data: 'hello'
            };

            assert.deepEqual(msg, expected, "client received correct message");
            assert.equal(decoded.payload.length, 100, "payload was the correct size");

            done();

        });


        var message = {
            "method": "ms.channel.emit",
            "params": {
                to: client1Info.id,
                event: 'say',
                data: 'hello'
            }
        };

        var payload = new Buffer(100);

        hostSocket.send(packMessage(message, payload));

    });


    it('emits an event to clients by all possible `to` fields', function(done) {

        var count = 0;

        client1Socket.on('message', function (msg) {

            msg = JSON.parse(msg);

            if( msg.data === client1Socket.id ||
                msg.data === 'all' ||
                msg.data === 'broadcast' ||
                msg.data === 'host' ||
                msg.data instanceof Array
            ){
                assert.ok(true, "client received message from target of : "+msg.data);
            }

            assert.equal(msg.event, "testToTargets", "client received correct event");
            assert.equal(msg.from, hostInfo.id, "client received correct from id");
            if (++count === 7) done();

        });

        hostSocket.on('message', function (msg) {

            msg = JSON.parse(msg);

            if( msg.data === hostSocket.id ||
                msg.data === 'all' ||
                msg.data === 'broadcast' ||
                msg.data === 'host' ||
                msg.data instanceof Array
            ){
                assert.ok(true, "host received message from target of : "+msg.data);
            }

            assert.equal(msg.event, "testToTargets", "host received correct event");
            assert.equal(msg.from, hostInfo.id, "host received correct from id");
            if (++count === 7) done();

        });


        var message = {
            "method": "ms.channel.emit",
            "params": {
                to: hostSocket.id,
                event: 'testToTargets',
                data: ''
            }
        };

        // To client id
        message.params.to = message.params.data = hostInfo.id;
        hostSocket.send(JSON.stringify(message));

        // To client all
        message.params.to = message.params.data = 'all';
        hostSocket.send(JSON.stringify(message));

        // To broadcast
        message.params.to = message.params.data = 'broadcast';
        hostSocket.send(JSON.stringify(message));

        // To host
        message.params.to = message.params.data = 'host';
        hostSocket.send(JSON.stringify(message));

        // To array of clients
        message.params.to = message.params.data = [client1Info.id, hostInfo.id];
        hostSocket.send(JSON.stringify(message));

    });

    it('connects client 2 correctly and all others are notified', function(done) {

        var count = 0;

        var endpoint = wsClientEndpoint + "?name=TheClient2";

        client2Socket = new WebSocket(endpoint);

        client2Socket.on('open', function () {
            assert.ok(true, "Client2 socket connected");
        });

        client2Socket.on('message', function (msg) {

            msg = JSON.parse(msg);

            if(msg.event === 'ms.channel.connect'){
                assert.ok(true, "client received the connect event");
                assert.ok(_.isString(msg.data.id), "valid client id");
                assert.ok(_.isArray(msg.data.clients), "valid clients array");

                msg.data.clients.forEach(function (info) {
                    if (info.id === msg.data.id)  client2Info = info;
                });

                assert.ok(_.isObject(client2Info), "contained the client information");

            }else if(msg.event === 'ms.channel.ready'){
                assert.ok(true, "client received the connect event");
                if (++count === 3) done();
            }


        });

        client1Socket.on('message', function (msg) {

            msg = JSON.parse(msg);

            assert.equal(msg.event, 'ms.channel.clientConnect', "client 1 received the correct method");
            assert.deepEqual(msg.data, client2Info, "contained the correct client information");
            if (++count === 3) done();
        });

        hostSocket.on('message', function (msg) {

            msg = JSON.parse(msg);

            assert.equal(msg.event, 'ms.channel.clientConnect', "host received the correct method");
            assert.deepEqual(msg.data, client2Info, "contained the correct client information");
            if (++count === 3) done();
        });

    });


    it('disconnects client 2 and all others are notified', function(done) {

        var count = 0;

        var expected = {
            event: "ms.channel.clientDisconnect",
            data: client2Info
        };

        hostSocket.on('message', function (msg) {
            msg = JSON.parse(msg);
            assert.deepEqual(msg, expected, "host received the correct disconnect message");
            if (++count === 3) done();
        });

        client1Socket.on('message', function (msg) {
            msg = JSON.parse(msg);
            assert.deepEqual(msg, expected, "client 1 received the correct disconnect message");
            if (++count === 3) done();
        });

        client2Socket.on('close',function () {
            assert.ok(true, "client 2 socket has closed");
            if (++count === 3) done();
        });

        client2Socket.close();

    });

    it('disconnects client 1 and all host is notified', function(done) {

        var count = 0;

        var expected = {
            event: "ms.channel.clientDisconnect",
            data: client1Info
        };

        hostSocket.on('message', function (msg) {
            msg = JSON.parse(msg);
            assert.deepEqual(msg, expected, "host received the correct disconnect message");
            if (++count === 2) done();
        });

        client1Socket.on('close', function () {
            assert.ok(true, "Client1 socket closed");
            if (++count === 2) done();
        });

        client1Socket.close();

    });

    it('something', function(done) {

        hostSocket.on('close', function () {

            assert.ok(true, "host socket closed");
            done();

        });

        hostSocket.close();

    });


    /*

     it('something', function(done) {


     });


     */



});