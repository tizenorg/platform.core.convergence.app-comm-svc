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
var utils = require('../utils');
var path = require("path");
var fs = require("fs");
var logger = require('../logging/index').createNamedLogger('Settings');

var settingsPath = path.join(__dirname,'../../../.settings.json');
var _data;

var settings = {

    get data(){

        if(_data) {
            return _data;
        }else if(fs.existsSync(settingsPath)){
            try{
                logger.info('Loading settings from '+settingsPath);
                _data = require(settingsPath);
            }catch(e){
                logger.warn('Unable to load settings : ', e.message);
                _data = {};
            }
            return _data;
        }else{
            logger.info('No settings file detected, creating...');
            _data = {};
            this.save();
            return _data;
        }


    },

    save : function(){
        try{
            fs.writeFileSync(settingsPath, JSON.stringify(_data,null,4));
            logger.info('Settings saved to disk at '+settingsPath);
        }catch(e){
            logger.error('Unable to write settings to disk. Settings will not persist after this session : ', e.message);
        }
    }

};

module.exports = settings;
