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
var express = require('express');
var path = require('path');
var logger = require('../logging').createNamedLogger('Router');
var fs = require('fs');

function configure(app){

    /*

    BUG FIX : Helper

    Handles situations where a post, put, delete.... is sent with type application/json and empty body.
    Just converts the request to text/plain instead
     */
    app.use(function(req, res, next){
        if(req.headers['content-type'] === "application/json" && req.headers['content-length'] === "0"){
            req.headers['content-type'] = "text/plain";
        }
        next();
    });



    /*
     Middleware : Parses body for different content types including forms/json/ect...
     */
    app.use(express.bodyParser());


    /*
     Middleware : Logs all request
     */
    app.use(function log(req, res, next){

        // Log the port number (requested by wasp team)
        //var thePortNo = (req.headers.host.slice(req.headers.host.indexOf(":")+1));
        //console.log("thePortNo  is -->>>> ",thePortNo );

        logger.silly('Incoming Request',{
            ip      : req.headers['x-forwarded-for'] || req.connection.remoteAddress,
            port    : req.headers.host.slice(req.headers.host.indexOf(":")+1),
            method  : req.method,
            url     : req.url,
            body    : req.body
        });

        res.on('_send', function(body){
            logger.silly('Outgoing Response : ', body.toString());
        });
        next();
    });



    /*
     Headers added to all request (also contains origin policy, which should be tightened up)
     */
    app.all('*', function(req, res, next) {
        res.header("Access-Control-Allow-Origin", "*");
        res.header("Access-Control-Allow-Credentials", true);
        res.header('Access-Control-Allow-Methods', "GET,PUT,POST,DELETE");
        res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, SilentLaunch");

        // TODO : Perhaps lock this down a little more (discuss when browsers should be able to use the API)
        if (req.method === 'OPTIONS') {
            return res.send('MultiScreen Server 2.0');
        }
        next();
    });


    /*
     Static resources
     */
    app.use('/resources',express.static(path.join(__dirname,'../resources/public')));

}

module.exports.configure = configure;

