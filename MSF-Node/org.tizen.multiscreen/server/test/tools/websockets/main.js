$(function(){

    var socket;

    /*
        UI Elements
     */
    var txtConnectUrl   = $("#txtConnectUrl");
    var btnConnect      = $("#btnConnect");
    var btnDisconnect   = $("#btnDisconnect");
    var selectMethod    = $("#selectMethod");
    var btnSend         = $("#btnSend");
    var txtSend         = $("#txtSend");
    var txtReceive      = $("#txtReceive");
    var txtNotification = $("#txtNotification");


    btnConnect.on('click', function(){
        console.debug('btnConnect : click');
        socket = new WebSocket(txtConnectUrl.val(),['msf-2','msf-3']);
        console.debug('connecting to '+txtConnectUrl.val());
        socket.addEventListener('open',     onSocketOpen );
        socket.addEventListener('message',  onSocketMessage );
        socket.addEventListener('close',    onSocketClose );
        socket.addEventListener('error',    onSocketError );
    });

    btnDisconnect.on('click', function(){
        console.debug('btnDisconnect : click');
        if(socket){
            socket.close();
        }
    });

    btnSend.on('click', function(){
        console.debug('btnSend : click');
        if(socket){
            socket.send(txtSend.val());
        }
    });

    selectMethod.on('change', function(){
        console.debug('selectMethod : change');
        var key = $(this).val();
        if(methods[key]){
            var tplObj = methods[key];
            txtSend.val(JSON.stringify(tplObj, null, 2));
        }else{
            console.warn('no template defined for selected method');
        }

    });

    var onSocketOpen = function(){
        console.info('websocket connected');
        btnConnect.prop("disabled",true);
        btnSend.prop("disabled",false);
        btnDisconnect.prop("disabled",false);
        $('body').css('background-color','#C9FF9C');
    };

    var onSocketMessage = function(msg){
        try{
            msg = JSON.parse(msg.data);
            console.info('websocket message : ', msg);
        }catch(e){
            console.error('Unable to parse message : ', msg.data);
        }

        if(msg.id){
            txtReceive.val(JSON.stringify(msg, null, 2));
            txtReceive.addClass('flash');
            txtReceive.one('animationend webkitAnimationEnd', function() {
                $(this).removeClass('flash');
            });
        }else{
            txtNotification.val(JSON.stringify(msg, null, 2));
            txtNotification.addClass('flash');
            txtNotification.one('animationend webkitAnimationEnd', function() {
                $(this).removeClass('flash');
            });
        }

    };

    var onSocketClose = function(){
        console.warn('websocket disconnected');
        btnConnect.prop("disabled",false);
        btnDisconnect.prop("disabled",true);
        btnSend.prop("disabled",true);
        socket = null;
        $('body').css('background-color','#E5E5E5');
    };

    var onSocketError = function(evt){
        console.error('websocket error : ', evt);
    };

    var initUI = function(){
        txtConnectUrl.val('ws://'+location.host+'/api/v2/channels/test');
        btnDisconnect.prop("disabled",true);
        btnSend.prop("disabled",true);
        for(var key in methods){
            if (methods.hasOwnProperty(key)) {
                var option = $('<option />')
                    .attr('value', key)
                    .text(key)
                    .appendTo(selectMethod);
            }
        }
        selectMethod.change();
    };


    var methods = {

        "ms.channel.emit" : {
            method : "ms.channel.emit",
            params : {
                event : "say",
                data  : "Hello World!",
                to    : "all"
            }
        },

        "ms.application.get" : {
            method : "ms.application.get",
            get id(){return Date.now();},
            params : {
                id : "HelloWorld"
            }
        },

        "ms.application.start" : {
            method : "ms.application.start",
            get id(){return Date.now();},
            params : {
                id : "HelloWorld"
            }
        },

        "ms.application.stop" : {
            method : "ms.application.stop",
            get id(){return Date.now();},
            params : {
                id : "HelloWorld"
            }
        },

        "ms.application.install" : {
            method : "ms.application.install",
            get id(){return Date.now();},
            params : {
                id : "HelloWorld"
            }
        },

        "ms.webapplication.get" : {
            method : "ms.webapplication.get",
            get id(){return Date.now();},
            params : {
                url : "http://prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/"
            }
        },

        "ms.webapplication.start" : {
            method : "ms.webapplication.start",
            get id(){return Date.now();},
            params : {
                url : "http://prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/"
            }
        },

        "ms.webapplication.stop" : {
            method : "ms.webapplication.stop",
            get id(){return Date.now();},
            params : {
                url : "http://prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/"
            }
        },

        "ms.pincode.show" : {
            method : "ms.pincode.show",
            get id(){return Date.now();}
        },

        "ms.pincode.hide" : {
            method : "ms.pincode.hide",
            get id(){return Date.now();}
        },

        "custom" : {}

    };

    initUI();


});