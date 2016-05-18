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
var dgram = require('dgram');
var MSFPlugin = require('../plugin');

/*
 Constants
 some aren't currently used but may need to be incorporated later
*/

var SERVICE_NAME = 'samsungmsf';
var RESPONSE_TTL = 60;

var MDNS_ADDRESS = "224.0.0.251";
var MDNS_PORT = 5353;
var CLASS_IN = 1;
var RESPONSE_CODE_OK = 0;

var TYPE_A    = 0x0001;//host address
var TYPE_AAAA    = 0x001C;//host address IPv6
var TYPE_PTR  = 0x000C;
var TYPE_TXT  = 0x0010;
var TYPE_SRV  = 0x0021;//service location
var TYPE_NSEC = 0x002F;//next secured

/*
 Constructor
 */
function Mdns(config){

	/* Call the super constructor */
	MSFPlugin.apply(this, arguments);

	/* event handlers */
    this.device.on('ready', this.initialize.bind(this));
	this.device.on('change', this.update.bind(this));


}

/*
 Extend the MSFPlugin
 */
MSFPlugin.extend(Mdns);

/*
 Initializes the plugin and begins broadcasting on udp socket
 */
Mdns.prototype.initialize = function(){

	this.logger.verbose('initializing MDNS');

	var self = this;
    var logger = this.logger;

    try {
        this.sock = dgram.createSocket({ type: 'udp4', reuseAddr: true });
    } catch (e){
        this.sock = dgram.createSocket('udp4');
    }

    this.sock.bind(MDNS_PORT, function(err){
        if(err) return this.logger.error(err.message);
        self.sock.addMembership(MDNS_ADDRESS);
        self.sock.setBroadcast(true);
        self.sock.setMulticastTTL(2);
        self.sock.setTTL(64);
        self.sock.setMulticastLoopback(false);
        self.sock.on('error', function (err) {
            logger.error(err);
        });


        self.sock.on('message', function (msg, rinfo) {
            self.onReceive(new Uint8Array(msg),rinfo.address,rinfo.port);
        });

        self.answer([
            {type: 12},
            {type: 16},
            {type: 33},
            {type: 1}
        ], MDNS_ADDRESS, MDNS_PORT);

        process.on('exit', function(){
            self.answer([
                {type: 12},
                {type: 16},
                {type: 33},
                {type: 1}
            ], MDNS_ADDRESS, MDNS_PORT, 0);
        });

    });
};

Mdns.prototype.update = function(){
	var self = this;
	if (self.sock){
		self.logger.debug('Closing MDNS socket to reinitialize');
		self.sock.on('close', function(){
			self.logger.debug('MDNS socket closed');
			self.initialize();
		});
		self.sock.close();

	} else {
		self.initialize();
	}


};


/*
 Parses incoming message and writes the response back to the socket
 */
Mdns.prototype.parseMdnsMessage = function(rawData){
    function consumeWord(){
        return (consumeByte() << 8) | consumeByte();
    }

	var result = {};
	var position = 0;
	var errored = false;
	result.transactionID = consumeWord();
	var flags = consumeWord();
	result.isQuery = (flags & 0x8000) === 0;
	result.opCode = (flags >> 11) & 0xF;
	result.authoritativeAnswer = (flags & 0x400) !== 0;
	result.truncated = (flags & 0x200) !== 0;
	result.recursionDesired = (flags & 0x100) !== 0;
	result.recursionAvailable = (flags & 0x80) !== 0;
	result.responseCode = flags & 0xF;
	var questionCount = consumeWord();
	var answerCount = consumeWord();
	var authorityRecordsCount = consumeWord();
	var additionalRecordsCount = consumeWord();
	result.questions = [];
	result.answers = [];
	result.authorityRecords = [];
	result.additionalRecords = [];

	function consumeDNSName(){

		var parts = [];
		while (true){
			if (position >= rawData.byteLength){
				break;
			}
			var partLength = consumeByte();
			if (partLength === 0)
				break;
			if (partLength === 0xC0){
				var bytePosition = consumeByte();
				var oldPosition = position;
				position = bytePosition;
				parts = parts.concat(consumeDNSName().split("."));
				position = oldPosition;
				break;
			}
			if (position + partLength > rawData.byteLength){
				if (!errored){
					errored = true;
				}
				partLength = rawData.byteLength - position;
			}
			var part = "";
			while (partLength-- > 0)
				part += String.fromCharCode(consumeByte());
			parts.push(part);
		}
		return parts.join(".");
	}

	function consumeByte(){
		if (position + 1 > rawData.byteLength){
			if (!errored){
				errored = true;
			}
			return 0;
		}
		return rawData[position++];
	}
	function consumeDWord(){
		return (consumeWord() << 16) | consumeWord();
	}
	function consumeQuestion(){
		var question = {};
		question.name = consumeDNSName();
		question.type = consumeWord();
		question.class = consumeWord();
		question.unicastResponseRequested = (question.class & 0x8000) !== 0;
		question.class &= 0x7FFF;
		return question;
	}
	function consumeByteArray(length){
		length = Math.min(length,rawData.byteLength - position);
		var data = new Uint8Array(length);
		for (var i = 0; i < length; i++){
			data[i] = consumeByte();
		}
		return data;
	}
    _.each(_.range(questionCount), function(){
        result.questions.push(consumeQuestion());
    });
	function consumeResourceRecord(){
		var resource = {};
		resource.name = consumeDNSName();
		resource.type = consumeWord();
		resource.class = consumeWord();
		resource.flushCache = (resource.class & 0x8000) !== 0;
		resource.class &= 0x7FFF;
		resource.timeToLive = consumeDWord();
		var extraDataLength = consumeWord();
		resource.resourceData = consumeByteArray(extraDataLength);
		return resource;
	}
    _.each(_.range(answerCount), function(){
        result.answers.push(consumeResourceRecord());
    });
    _.each(_.range(authorityRecordsCount), function(){
        result.authorityRecords.push(consumeResourceRecord());
    });
    _.each(_.range(additionalRecordsCount), function(){
        result.additionalRecords.push(consumeResourceRecord());
    });
	return result;
};


/*
 Encodes messages to bytearray
 */
Mdns.prototype.encodeMDNSMessage = function(message){
    function writeByte(b,pos){
        if (pos != null){
            data[pos] = b;
        }
        else{
            data.push(b);
        }
        return 1;
    }

    function writeWord(w, pos){
        if (pos != null){
            return writeByte(w >> 8,pos) + writeByte(w & 0xFF,pos + 1);
        }
        return writeByte(w >> 8) + writeByte(w & 0xFF);
    }

    function writeDWord(d){
        return  writeWord(d >> 16) + writeWord(d & 0xFFFF);
    }

    function writeByteArray(b){
        var bytesWritten = 0;
        for (var i = 0, li = b.length; i < li; i++){
            bytesWritten += writeByte(b[i]);
        }
        return bytesWritten;
    }

    function writeIPAddress(a){
        var parts = a.split(".");
        var bytesWritten = 0;
        for (var i = 0, li = parts.length; i < li; i++){
            bytesWritten += writeByte(parseInt(parts[i]));
        }
        return bytesWritten;
    }

    function writeIP6Address(a){
        a = a.replace(":", '');
        var bytesWritten = 0;
        for (var i = 0, li = a.length; i < li; i++){
            bytesWritten += writeByte(parseInt(a.charAt(i)));
        }
        return bytesWritten;
    }

    function writeStringArray(parts,includeLastTerminator){
        var brokeEarly = false;
        var bytesWritten = 0;
        for (var i = 0, li = parts.length; i < li; i++){
            var remainingString = parts.slice(i).join("._-_.");
            var location = textMapping[remainingString];
            if (location != null){
                brokeEarly = true;
                bytesWritten += writeByte(0xC0);
                bytesWritten += writeByte(location);
                break;
            }
            if (data.length < 256){//we can't ever shortcut to a position after the first 256 bytes
                textMapping[remainingString] = data.length;
            }
            var part = parts[i];
            bytesWritten += writeByte(part.length);
            for (var j = 0, lj = part.length; j < lj; j++){
                bytesWritten += writeByte(part.charCodeAt(j));
            }
        }
        if (!brokeEarly && includeLastTerminator)
            bytesWritten += writeByte(0);
        return bytesWritten;


    }

    function writeDNSName(n){
        var parts = n.split(".");
        return writeStringArray(parts,true);
    }

    function writeQuestion(q){
        writeDNSName(q.name);
        writeWord(q.type);
        writeWord(1);
    }
    function writeRecord(r){
        writeDNSName(r.name);
        writeWord(r.type);
        writeWord(1);
        writeDWord(r.timeToLive);
        var lengthPos;
        var length;
        switch (r.type){
            case TYPE_NSEC:
                lengthPos = data.length;
                writeWord(0);
                length = writeDNSName(r.nsec_domainName);
                length += writeByte(0);//offset (always 0)
                r.nsec_types.sort();
                var bytesNeeded = Math.ceil(r.nsec_types[r.nsec_types.length - 1] / 8);
                length += writeByte(bytesNeeded);
                var bitMapArray = new Uint8Array(bytesNeeded);
                for (var i = 0, li = r.nsec_types.length; i < li; i++){
                    var type= r.nsec_types[i];
                    var byteNum = Math.floor(type / 8);
                    var bitNum = type % 8;
                    bitMapArray[byteNum] |= 1 << (7 - bitNum);
                }
                length += writeByteArray(bitMapArray);
                writeWord(length,lengthPos);
                break;
            case TYPE_TXT:
                lengthPos = data.length;
                writeWord(0);
                length = writeStringArray(r.txt_texts,false);
                writeWord(length,lengthPos);
                break;
            case TYPE_A:
                lengthPos = data.length;
                writeWord(0);
                length = writeIPAddress(r.a_address);
                writeWord(length,lengthPos);
                break;
            case TYPE_AAAA:
                lengthPos = data.length;
                writeWord(0);
                length = writeIP6Address(r.a_address);
                writeWord(length,lengthPos);
                break;
            case TYPE_SRV:
                lengthPos = data.length;
                writeWord(0);
                length = writeWord(r.srv_priority);
                length += writeWord(r.srv_weight);
                length += writeWord(r.srv_port);
                length += writeDNSName(r.srv_target);
                writeWord(length,lengthPos);
                break;
            case TYPE_PTR:
                lengthPos = data.length;
                writeWord(0);
                length = writeDNSName(r.ptr_domainName);
                writeWord(length,lengthPos);
                break;
            default:
                writeWord(r.resourceData.byteLength);
                writeByteArray(r.resourceData);
        }
    }

	var data = [];
	var textMapping = {};

	writeWord(message.transactionID);
	var flags = 0;
	if (!message.isQuery)
		flags |= 0x8000;
	flags |= (message.opCode & 0xFF) << 11;
	if (message.authoritativeAnswer)
		flags |= 0x400;
	if (message.truncated)
		flags |= 0x200;
	if (message.recursionDesired)
		flags |= 0x100;
	if (message.recursionAvailable)
		flags |= 0x80;
	flags |= message.responseCode & 0xF;
	writeWord(flags);
	writeWord(message.questions.length);
	writeWord(message.answers.length);
	writeWord(message.authorityRecords.length);
	writeWord(message.additionalRecords.length);

	var i, li;
	for (i = 0, li = message.questions.length; i < li; i++){
		writeQuestion(message.questions[i]);
	}
	for (i = 0, li = message.answers.length; i < li; i++){
		writeRecord(message.answers[i]);
	}
	for (i = 0, li = message.authorityRecords.length; i < li; i++){
		writeRecord(message.authorityRecords[i]);
	}
	for (i = 0, li = message.additionalRecords.length; i < li; i++){
		writeRecord(message.additionalRecords[i]);
	}

	return new Uint8Array(data);


};

/*
 Responds to mdns questions based on question type
 */
Mdns.prototype.answer = function(questions, address, port, ttl, transactionID){

	var self = this;

	transactionID = transactionID || 0;
	ttl = ttl || RESPONSE_TTL;

	_.each(questions, function(question){
		var targetAddress = MDNS_ADDRESS;
		var targetPort = MDNS_PORT;
		if (question.unicastResponseRequested) {
			targetAddress = address;
			targetPort = port;
		}
		var response = {
			transactionID: transactionID,
			isQuery: false,
			opCode: 0,
			authoritativeAnswer: true,
			truncated: false,
			recursionDesired: false,
			recursionAvailable: false,
			responseCode: RESPONSE_CODE_OK,
			questions: [],
			answers: [],
			additionalRecords: [],
			authorityRecords: []
		};

		if (question.name === "_" + SERVICE_NAME + "._tcp.local"){
			if (question.type === 12){
				var addPtrRecord = {
					name: "_" + SERVICE_NAME + "._tcp.local",
					type: TYPE_PTR,
					class: CLASS_IN,
					flushCache: true,
					timeToLive: ttl,
					ptr_domainName: self.device.attributes.duid + "._" + SERVICE_NAME + "._tcp.local"
				};
				if (response.answers.length > 0){
					response.additionalRecords.push(addPtrRecord);
				} else {
					response.answers.push(addPtrRecord);
				}
			}
		}
		if (question.name === "._" + SERVICE_NAME + "._tcp.local"){
			if (question.type === 12){
				var addPtrDotRecord = {
					name: "._" + SERVICE_NAME + "._tcp.local",
					type: TYPE_PTR,
					class: CLASS_IN,
					flushCache: true,
					timeToLive: ttl,
					ptr_domainName: self.device.attributes.duid + "._" + SERVICE_NAME + "._tcp.local"
				};
				if (response.answers.length > 0){
					response.additionalRecords.push(addPtrDotRecord);
				} else {
					response.answers.push(addPtrDotRecord);
				}
			}
		}
		if (question.name === self.device.attributes.duid + "._" + SERVICE_NAME + "._tcp.local"){
			if (question.type === 16){
				var addTxtRecord = {
					name: self.device.attributes.duid + "._" + SERVICE_NAME + "._tcp.local",
					type: TYPE_TXT,
					class: CLASS_IN,
					flushCache: true,
					timeToLive: ttl,
					txt_texts:[
						"id=" + self.device.attributes.id,
                        "remote=" + self.service.remoteVersion,
						"ve=" + self.service.version,
						"md=" + self.device.attributes.type,
						"ic=/resources/img/icon.png",
						"fn=" + self.device.attributes.name,
						"se=" + "http://" + self.utils.getInternalIP() + ":8001" + "/api/v2/"
					]
				};
				if (response.answers.length > 0){
					response.additionalRecords.push(addTxtRecord);
				} else {
					response.answers.push(addTxtRecord);
				}
			}
			if (question.type === 33){
				var addSrvRecord = {
					name: self.device.attributes.duid + "._" + SERVICE_NAME + "._tcp.local",
					type: TYPE_SRV,
					class: CLASS_IN,
					flushCache: true,
					timeToLive: ttl,
					srv_priority: 0,
					srv_weight: 0,
					srv_port: 8001,
					srv_target: self.device.attributes.duid + ".local"
				};
				if (response.answers.length > 0){
					response.additionalRecords.push(addSrvRecord);
				} else {
					response.answers.push(addSrvRecord);
				}
			}
		}
		if (question.name === self.device.attributes.duid + ".local"){
			if (question.type === 1){
				var addARecord = {
					name: self.device.attributes.duid + ".local",
					type: TYPE_A,
					class: CLASS_IN,
					flushCache: true,
					timeToLive: ttl,
					a_address: self.utils.getInternalIP()
				};
				if (response.answers.length > 0){
					response.additionalRecords.push(addARecord);
				} else {
					response.answers.push(addARecord);
				}
			}
			var addNsecRecord = {
				name: self.device.attributes.duid + ".local",
				type: TYPE_NSEC,
				class: CLASS_IN,
				flushCache: true,
				timeToLive: ttl,
				nsec_domainName: self.device.attributes.duid + ".local",
				nsec_types:[TYPE_AAAA]
			};

			if (response.answers.length > 0){
				response.additionalRecords.push(addNsecRecord);
			} else {
				response.answers.push(addNsecRecord);
			}
		}
		if (response.answers.length > 0){
			response = self.encodeMDNSMessage(response);
			response = new Buffer(response);
			self.send(response, targetAddress, targetPort);
		}

	});
};

/*
 Handler for receiving messages and answering them
 */
Mdns.prototype.onReceive = function(data,address,port){
	var self = this;
	var message = self.parseMdnsMessage(data);
	this.answer(message.questions, address, port, RESPONSE_TTL, message.transactionID);
};

/*
 Sends mDNS message accross the socket
 */
Mdns.prototype.send = function(message, address, port){
    var logger = this.logger;
	this.sock.send(message, 0, message.length, port, address, function(err){
		if (err){
            logger.error(err);
		}
	});
};

module.exports = Mdns;
