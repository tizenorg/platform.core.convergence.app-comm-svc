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
var assert = require('assert');
var request = require('request');
var _ = require('lodash');

if(!global.msfService){
    global.msfService = require('../lib/multiscreen-service');
    global.msfService.logger.transports.console.silent = true;
    global.msfService.start();
}


// version 2 endpoint
var serviceURL = "http://127.0.0.1:8001/api/v2/";

// test values for app and webapp
var testAppId = "e16522e6-1dbb-11e4-9ae8-b2227cce2b54";
var testWebAppUrl = "http://engadget.com";


describe('MultiScreen Service : API V2 : REST', function() {

    it('waits 4 seconds before testing', function(done) {
        this.timeout(5000);
        assert.ok(true, "waiting");
        setTimeout(done,4000);
    });

    it('gets service information', function(done) {

        request.get({url: serviceURL, json: true}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('shows pincode', function(done) {

        request.post({url: serviceURL + 'pincode'}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('hides pincode', function(done) {

        request.del({url: serviceURL + 'pincode'}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('gets application information', function(done) {

        request.get({url: serviceURL + 'applications/' + testAppId}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('launches application', function(done) {
        this.timeout(5000);
        request.post({url: serviceURL + 'applications/' + testAppId}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            setTimeout(done,3000);
        });

    });

    it('stops application', function(done) {
        this.timeout(5000);
        request.del({url: serviceURL + 'applications/' + testAppId}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            setTimeout(done,2000);
        });

    });

    it('installs application', function(done) {
        request.put({url: serviceURL + 'applications/' + testAppId}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('gets web-application information', function(done) {
        request.get({url: serviceURL + 'webapplication/'}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            done();
        });

    });

    it('launches the web-application', function(done) {
        this.timeout(5000);
        request.post({ url: serviceURL + 'webapplication/', json: {url: testWebAppUrl} }, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            setTimeout(done,3000);
        });

    });

    it('stops the web-application', function(done) {
        this.timeout(5000);
        request.del({url: serviceURL + 'webapplication/'}, function (err, response, result) {
            assert.equal(response.statusCode, 200, 'status code is 200');
            setTimeout(done,2000);
        });

    });



    /*
    it('does something', function(done) {



    });
    */

});