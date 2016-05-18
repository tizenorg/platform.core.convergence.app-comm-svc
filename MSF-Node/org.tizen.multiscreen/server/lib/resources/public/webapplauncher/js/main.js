(function(){

	if(typeof tizen !== "undefined") {
		var app = tizen.application.getCurrentApplication();
	}

	var out = document.getElementById('out');
	var loading = document.getElementById('loading');
	var loadingMsg = document.getElementById('loadingMsg');
	var error = document.getElementById('error');
	var errorMsg = document.getElementById('errorMsg');

	var logError = function (msg) {
		if(typeof(msg) !== 'string') msg = JSON.stringify(msg);
		log('ERROR : '+msg);
	};

	var log = function (msg) {
		if(typeof(msg) !== 'string') msg = JSON.stringify(msg);
		if(out) out.value += msg + '\n';
	};


	var showError = function(msg){

		errorMsg.innerHTML = msg;
		error.style.display = 'block';
		loading.style.display = 'none';

		setTimeout(function(){
			if(app)app.exit();
		},5000);


	};

	var showLoading = function(msg){

		loadingMsg.innerHTML = msg;
		loadingMsg.style.display = 'block';
		loading.style.display = 'block';
		error.style.display = 'none';
	};

	var getLaunchData = function (callback){

		log("getLaunchData() called");

		var request = new XMLHttpRequest();
		request.timeout = 5000;

		request.onload = function(e){
			log("getLaunchData() : result : "+this.responseText);
			if (this.status === 200) {
				var result;
				try{
					result = JSON.parse(this.responseText);
					callback(null, result);
				} catch(e){
					callback(e);
				}
			}else{
				callback(new Error('Unable to retrieve launch data, server responded with '+this.status));
			}

		};
		request.onerror = function(e){
			callback(e.message);
		};
		request.ontimeout = function(e){
			callback(new Error('Request timeout'));
		};

		request.open("GET",'http://127.0.0.1:8001/api/v2/webapplication/data');
		request.send();
	};


	var init = function () {

		log("init : "+window.location.href);

		getLaunchData(function(err, result){

			if(err)	{

				logError(err.message);
				showError('UNABLE TO LOAD LAUNCH DATA');

			}else if(!result.url) {

				logError('data has no url property');
				showError('NO URL FOUND IN LAUNCH DATA');

			}else {

				if(result.url.indexOf('msf-debug') > -1) out.style.display = "block";

				var parser = document.createElement('a');
				parser.href = result.url;
				showLoading(parser.hostname);

				window.location = result.url;
			}

		});

	};

	window.onerror = function (errorMsg){
		logError(errorMsg);
	};

	window.onload = function(){
		init();
	};

	window.addEventListener('keydown', function(e) {
		log('key pressed : '+e.keyCode);
	});



})();