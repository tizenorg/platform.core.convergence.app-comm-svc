<!DOCTYPE html>
<html>
<head lang="en">
    <meta charset="UTF-8">
    <title>Debug Config</title>


    <link rel="stylesheet" href="/resources/css/bootstrap.min.css">
    <!-- Optional theme
    <link rel="stylesheet" href="/resources/css/bootstrap-theme.min.css">
    -->

    <script src="/resources/js/jquery.min.js"></script>
    <script src="/resources/js/bootstrap.min.js"></script>

    <style>

        table.table-striped tr td:first-child{
            width : 300px;
        }

        #devSettings label{
                font-weight: normal;
        }

        #devSettings select{
            padding: 5px;
            height: auto;
            margin-bottom: 20px;
            width: auto;
            min-width: 200px;
        }


    </style>

</head>
<body>

<div class="container-fluid">
    <div class="page-header">
        <h1>MultiScreen Service</h1>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading">Service Information</div>
        <div class="panel-body">
            <table class="table table-striped">
                <tbody>
                <% _.each(service, function(value, key, object){ %>
                <tr>
                    <td><%= key.toUpperCase() %></td>
                    <td><%= value %></td>
                </tr>
                <% }); %>
                </tbody>
            </table>
        </div>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading">Memory Information</div>
        <div class="panel-body">
            <table class="table table-striped">
                <tbody>
                <% _.each(mem, function(value, key, object){ %>
                <tr>
                    <td><%= key.toUpperCase() %></td>
                    <td><%= value %></td>
                </tr>
                <% }); %>
                </tbody>
            </table>
        </div>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading">Device Information</div>
        <div class="panel-body">
            <table class="table table-striped">
                <tbody>
                <% _.each(device, function(value, key, object){ %>
                <tr>
                    <td><%= key.toUpperCase() %></td>
                    <td><%= value %></td>
                </tr>
                <% }); %>
                </tbody>
            </table>
        </div>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading">Developer Options &nbsp;&nbsp;&nbsp; <a href="/logs/" target="ms-logger" style="color:greenyellow;">View Logs</a></div>
        <div class="panel-body" id="devSettings">


                <label for="logLevel">Logging Level</label>
                <select name="logLevel" id="logLevel" class="form-control">
                    <% _.each(logging.levels, function(value){ %>
                    <% if(value === logging.level){ %>
                    <option value="<%= value %>" selected><%= value.toUpperCase() %></option>
                    <% }else{ %>
                    <option value="<%= value %>"><%= value.toUpperCase() %></option>
                    <% } %>
                    <% }); %>
                </select>


                <div class="checkbox">
                    <label>
                        <% if(allowAllContent){ %>
                        <input checked type="checkbox" value="allow" name="allowAllContent" id="allowAllContent">
                        <% }else{ %>
                        <input type="checkbox" value="allow" name="allowAllContent" id="allowAllContent">
                        <% } %>
                        Allow All Content
                    </label>
                </div>
                <img src="/captcha.png"/>
                <div class="captcha form-group">
                    <label>
                        <input checked type="text" class="form-control" value="" name="captchaAnswer" id="captchaAnswer">
                        Enter text above to submit
                    </label>
                </div>
                <button type="button" class="btn btn-primary" id="submit">Submit</button>
        </div>
    </div>

</div>


<script>

    var uiLogLevel = $('#logLevel');
    var uiAllowAllContent = $('#allowAllContent');
    var captchaAnswer = $('#captchaAnswer');
    var submit = $('#submit');

    function onUpdateSuccess(data){
        alert(data);
    }

    function onUpdateError(error){
        alert(error.statusText);
        window.location.reload();
    }

    submit.on('click',function(){
        console.log('submitting setting change ::');
        $.post( 'settings', {captchaAnswer:captchaAnswer.val(), allowAllContent:uiAllowAllContent.prop('checked'), logLevel:uiLogLevel.val()}).done(onUpdateSuccess).fail(onUpdateError);
    });


</script>







</body>
</html>