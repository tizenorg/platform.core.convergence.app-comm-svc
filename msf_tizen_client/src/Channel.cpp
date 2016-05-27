#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <sstream>
#include <string>
#include <iterator>
#include "Clients.h"
#include "Service.h"
#include "Channel.h"
#include "Message.h"
#include "Debug.h"

#define JSON_KEY_IS_HOST 0
#define JSON_KEY_CONNECT_TIME 1
#define JSON_KEY_STATUS 2
#define JSON_KEY_CODE 3
#define JSON_KEY_ATTRIBUTES 4
#define JSON_KEY_RESULT 5
#define JSON_KEY_MESSAGE_ID 6
#define JSON_KEY_ERROR 7
#define JSON_KEY_EVENT 8
#define JSON_KEY_FROM 9
#define JSON_KEY_MESSAGE 10
#define JSON_KEY_METHOD 11
#define JSON_KEY_DATA 12
#define JSON_KEY_CLIENTS 13

class Application;

using namespace std;

string ChannelConnectionHandler::PING = "channel.ping";
string ChannelConnectionHandler::PONG = "pong";
Channel *ChannelConnectionHandler::channel_ptr = NULL;
string Channel::ROUTE = "channels";
string Channel::ERROR_EVENT = "ms.error";
string Channel::CONNECT_EVENT = "ms.channel.connect";
string Channel::CLIENT_CONNECT_EVENT = "ms.channel.clientConnect";
string Channel::CLIENT_DISCONNECT_EVENT = "ms.channel.clientDisconnect";
string Channel::READY_EVENT = "ms.channel.read";
map<Channel *, int> Channel::channel_alive_map;
map<string, int> Channel::json_keys;
pthread_t Channel::connect_thread;
pthread_t ChannelConnectionHandler::ping_thread = 0;

ChannelConnectionHandler::ChannelConnectionHandler() {
    pingTimeout = 5000000;
    lastPingReceived = 0;
    running = false;
}

Channel::Channel() {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    clientisHost = false;
    isWrite = false;
    buflen = 0;
    was_closed = 1;
    wsi_mirror = NULL;
    Context = NULL;
    mirror_lifetime = 0;
    disconnecting = false;
    from = "";
    clients = new Clients(this); // new to destroy in destructer
    isLaunched = false;
    connectionHandler = new ChannelConnectionHandler();
    resultresp = false;
    channel_alive_map.insert({this, 1});
    init_json_key_map();
}

Channel::Channel(Service *service1, string uri1) {
    dlog_print(DLOG_INFO, "MSF", "Channel()");
    clientisHost = false;
    isWrite = false;
    buflen = 0;
    was_closed = 1;
    wsi_mirror = NULL;
    Context = NULL;
    mirror_lifetime = 0;
    disconnecting = false;
    from = "";
    clients = new Clients(this); // new to destroy in destructer
    isLaunched = false;
    connectionHandler = new ChannelConnectionHandler();
    resultresp = false;
    service = service1;
    m_uri = uri1;
    channel_alive_map.insert({this, 1});
    init_json_key_map();
}

Channel *Channel::create(Service *service, string uri) {
    dlog_print(DLOG_INFO, "MSF", "Channel()");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    fprintf(stderr, "\n********************************\n");
    if ((service == NULL) || uri.length() <= 0)
        return NULL;
    Channel *channel = new Channel(service, uri);
    return channel;
}

Channel::~Channel() {
    dlog_print(DLOG_INFO, "MSF", "~Channel()");
    connect_cb = NULL;
    disconnect_cb = NULL;
    onConnectListener = NULL;
    onDisconnectListener = NULL;
    onClientConnectListener = NULL;
    onClientDisconnectListener = NULL;
    onReadyListener = NULL;
    channel_alive_map[this] = 0;

	if (clients != NULL) {
		delete clients;
		clients = NULL;
	}
	if (connectionHandler != NULL) {
		delete connectionHandler;
		connectionHandler = NULL;
	}
}

void Channel::init_json_key_map() {
    static bool done = false;

    if (!done) {
        json_keys["isHost"] = JSON_KEY_IS_HOST;
        json_keys["connectTime"] = JSON_KEY_CONNECT_TIME;
        json_keys["status"] = JSON_KEY_STATUS;
        json_keys["code"] = JSON_KEY_CODE;
        json_keys["attributes"] = JSON_KEY_ATTRIBUTES;
        json_keys[Message::PROPERTY_RESULT] = JSON_KEY_RESULT;
        json_keys[Message::PROPERTY_MESSAGE_ID] = JSON_KEY_MESSAGE_ID;
        json_keys[Message::PROPERTY_ERROR] = JSON_KEY_ERROR;
        json_keys[Message::PROPERTY_EVENT] = JSON_KEY_EVENT;
        json_keys[Message::PROPERTY_FROM] = JSON_KEY_FROM;
        json_keys[Message::PROPERTY_MESSAGE] = JSON_KEY_MESSAGE;
        json_keys[Message::PROPERTY_METHOD] = JSON_KEY_METHOD;
        json_keys[Message::PROPERTY_DATA] = JSON_KEY_DATA;
        json_keys[Message::PROPERTY_CLIENTS] = JSON_KEY_CLIENTS;

        done = true;
    }
}

void Channel::foreach_json_object(JsonObject *object, const gchar *name,
                                  JsonNode *node, gpointer user_data) {
    dlog_print(DLOG_INFO, "MSF", "foreach_json_object name : %s", name);
    if (json_keys.find(name) == json_keys.end()) {
        dlog_print(DLOG_INFO, "MSF",
                   "foreach_json_object : there is no %s in map", name);
        return;
    }

    int key = json_keys[name];
    Channel *p = static_cast<Channel *>(user_data);
    static int iferror = 0;
    static bool arrayofclients = false;

    switch (key) {
    // key : isHost
    case JSON_KEY_IS_HOST: {
        p->clientisHost = json_node_get_boolean(node);
        dlog_print(DLOG_INFO, "MSF", "isHost set as %s",
                   p->clientisHost ? "true" : "false");

        if (!strncmp(p->eventType.c_str(), CLIENT_CONNECT_EVENT.c_str(), 25) ||
            !strncmp(p->eventType.c_str(), CONNECT_EVENT.c_str(), 25)) {
            p->client.create(p, p->clientid, p->clientconnectTime,
                             p->clientisHost, map<string, string>());
            dlog_print(DLOG_INFO, "MSF", "add clientList");
            p->clientList.push_back(p->client);
        }
    } break;

    // key : connectTime
    case JSON_KEY_CONNECT_TIME: {
        p->clientconnectTime = json_node_get_int(node);
        dlog_print(DLOG_INFO, "MSF", "clientconnectTime set as %lld",
                   p->clientconnectTime);
    } break;

    // key : status
    case JSON_KEY_STATUS: {
        if (iferror) {
            p->errstatus = json_node_get_int(node);
            dlog_print(DLOG_INFO, "MSF", "p->errstatus set");
        }
    } break;

    // key : code
    case JSON_KEY_CODE: {
        if (iferror) {
            p->errcode = json_node_get_int(node);
            string eid = std::to_string(p->msg_id);
            dlog_print(DLOG_INFO, "MSF", "p->errcode set");

            if (p->callbacks.find(eid) != p->callbacks.end()) {
                dlog_print(DLOG_INFO, "MSF", "test 1");
                pair<void *, int> temp = p->getcallback(eid);
                if (temp.first != NULL) {
                    Result_Base *temp1 = (Result_Base *)(temp.first);
                    string err = "";
                    err.append("status:");
                    err.append(to_string(p->errstatus));
                    err.append(" message:");
                    err.append(p->errMsg);
                    err.append(" code:");
                    err.append(to_string(p->errcode));
                    dlog_print(DLOG_INFO, "MSF", "test 2");
                    if (temp1)
                        temp1->onError(Error::create(err));
                    if (p->onErrorListener)
                        p->onErrorListener->onError();
                }
            }
        }
    } break;

    // key : attributes
    case JSON_KEY_ATTRIBUTES: {
    } break;

    // key : MESSAGE::PROPERTY_RESULT
    case JSON_KEY_RESULT: {
        if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
            p->resultresp = json_node_get_boolean(node);
            if (p->resultresp) {
                dlog_print(DLOG_INFO, "MSF", "set resultresp as true");
            } else {
                dlog_print(DLOG_INFO, "MSF", "set resultresp as false");
            }
            p->msg_subject = Message::PROPERTY_RESULT;
        } else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
            dlog_print(DLOG_INFO, "MSF", "set resultobj");
            g_free(p->resultobj);
            p->resultobj = json_node_dup_object(node);
        }
    } break;

    // key : MESSAGE::PROPERTY_MESSAGE_ID
    case JSON_KEY_MESSAGE_ID: {
        if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
            if (json_node_get_value_type(node) == G_TYPE_INT ||
                json_node_get_value_type(node) == G_TYPE_INT64) {
                p->msg_id = json_node_get_int(node);
                dlog_print(DLOG_INFO, "MSF", "msg-id set as %d", p->msg_id);
            } else if (json_node_get_value_type(node) == G_TYPE_STRING) {
                if ((p->eventType == CLIENT_CONNECT_EVENT) || arrayofclients) {
                    p->clientid = json_node_get_string(node);
                    dlog_print(DLOG_INFO, "MSF", "clientid set as = %s",
                               p->clientid.c_str());
                } else {
                    p->from = json_node_get_string(node);
                    dlog_print(DLOG_INFO, "MSF", "from set as = %s",
                               p->from.c_str());
                }
            }
        }
    } break;

    // key : MESSAGE::PROPERTY_ERROR
    case JSON_KEY_ERROR: {
        if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
            iferror = 1;
            dlog_print(DLOG_INFO, "MSF", "iferror set as 1");
        } else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
            iferror = 1;
            dlog_print(DLOG_INFO, "MSF", "iferror set as 1");
            // g_free(p->errobj);
            p->errobj = true;
            json_object_foreach_member(json_node_get_object(node),
                                       foreach_json_object, user_data);
        }
    } break;

    // key : MESSAGE::PROPERTY_EVENT
    case JSON_KEY_EVENT: {
        p->eventType = json_node_get_string(node);
        dlog_print(DLOG_INFO, "MSF", "eventType set as = %s",
                   p->eventType.c_str());
    } break;

    // key : MESSAGE::PROPERTY_FROM
    case JSON_KEY_FROM: {
        p->from = json_node_get_string(node);
        dlog_print(DLOG_INFO, "MSF", "from set as = %s", p->from.c_str());
    } break;

    // key : MESSAGE::PROPERTY_MESSAGE
    case JSON_KEY_MESSAGE: {
        p->errMsg = json_node_get_string(node);
        dlog_print(DLOG_INFO, "MSF", "errMsg set as = %s", p->errMsg.c_str());
    } break;

    // key : MESSAGE::PROPERTY_METHOD
    case JSON_KEY_METHOD: {
        p->method = json_node_get_string(node);
        dlog_print(DLOG_INFO, "MSF", "method set as = %s", p->method.c_str());
    } break;
    // key : MESSAGE:PROPERTY_DATA
    case JSON_KEY_DATA: {
        if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
            if (json_node_get_value_type(node) == G_TYPE_STRING) {
                p->data = json_node_get_string(node);
                dlog_print(DLOG_INFO, "MSF", "data set as = %s",
                           p->data.c_str());
            }
        } else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
            json_object_foreach_member(json_node_get_object(node),
                                       foreach_json_object, user_data);
        }
    } break;

    // key : MESSAGE:PROPERTY_CLIENTS
    case JSON_KEY_CLIENTS: {
        arrayofclients = true;
        dlog_print(DLOG_INFO, "MSF", "array start");
        json_array_foreach_element(json_node_get_array(node),
                                   foreach_json_array, user_data);
        dlog_print(DLOG_INFO, "MSF", "array end");
        arrayofclients = false;
    } break;

    default:
        break;
    }
}

void Channel::foreach_json_array(JsonArray *array, guint index, JsonNode *node,
                                 gpointer user_data) {
    json_object_foreach_member(json_node_get_object(node), foreach_json_object,
                               user_data);
}

void Channel::json_parse(const char *in) {
    dlog_print(DLOG_INFO, "MSF", "Channel::json_parse : %s", in);

    JsonParser *parser = json_parser_new();

    if (json_parser_load_from_data(parser, in, -1, NULL)) {
        JsonNode *node = json_parser_get_root(parser);

        if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
            json_object_foreach_member(json_node_get_object(node),
                                       foreach_json_object, this);
        }

    } else {
        dlog_print(DLOG_ERROR, "MSF", "json_parsing error");
    }
}

Clients *Channel::getclients() {
    if (clients->size())
        return clients;
    else
        return NULL;
}

void Channel::setConnectionTimeout(long timeout) {
    if (timeout < 0)
        return;
    else if (timeout == 0)
        connectionHandler->stopPing();
    else
	{
        connectionHandler->setPingTimeout(timeout);
        if (isWebSocketOpen())
            connectionHandler->startPing(this);
    }
}

void Channel::handleConnectMessage(string UID) {
    dlog_print(DLOG_INFO, "MSF", "handleConnectMessage(uid)");

    clients->reset();
    clients->add(clientList);
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    clients->setMyClientId(from);

    handleConnect(UID);
    dlog_print(DLOG_INFO, "MSF", "handleConnectMessage(uid) 1");
}

void Channel::handleMessage(string UID) { handleMessage(UID, NULL); }

void Channel::handleMessage(string UID, unsigned char payload[]) {
    if (eventType.length() == 0) {
        // if (msg_subject == Message::PROPERTY_RESULT) {
        //	handleClientMessage(NULL,NULL);
        //	return;
        //}

        handleApplicationMessage(UID);
    } else if (eventType == CLIENT_CONNECT_EVENT) {
        dlog_print(DLOG_INFO, "MSF", "call handleClientConnectMessage");
        handleClientConnectMessage();
    } else if (eventType == CLIENT_DISCONNECT_EVENT) {
        dlog_print(DLOG_INFO, "MSF", "call handleclientdisconnect");
        handleClientDisconnectMessage();
    } else if (eventType == ERROR_EVENT) {
        dlog_print(DLOG_INFO, "MSF", "call handleErrorMessage");
        handleErrorMessage(UID);
    } else if (eventType == READY_EVENT) {
        dlog_print(DLOG_INFO, "MSF", "call handleReadyMessage");
        handleReadyMessage();
    } else {
        dlog_print(DLOG_INFO, "MSF", "call handleClientMessage 2");
        handleClientMessage(data.c_str(), payload);
    }
}

void Channel::handleApplicationMessage(string uid) {
    dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 1");
    MSF_DBG(
        "[MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n",
        __FUNCTION__, __LINE__, __FILE__);
    string messageId = uid;
    bool errorMap = errobj;
    dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 2");
    pair<void *, int> temp = getcallback(messageId);

    dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 3");

    if (temp.first != NULL) {
        dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 4");
        if ((waitForOnReady) && (errorMap == false)) {
            dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 5");
            MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in "
                    "file [%s] \n",
                    __FUNCTION__, __LINE__, __FILE__);
            onReadyCallbacks.insert(temp);
            if (!isLaunched) {
                dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 6");
                MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] "
                        "in file [%s] \n",
                        __FUNCTION__, __LINE__, __FILE__);
                isLaunched = true;
            } else {
                dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 7");
                MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] "
                        "in file [%s] \n",
                        __FUNCTION__, __LINE__, __FILE__);
                // handleClientDisconnectMessage();
                isLaunched = false;

                if (temp.second == Result_bool) {
                    dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 8");
                    doApplicationCallback((Result_Base *)(temp.first));
                }
            }
            return;
        }

        if (temp.second == Result_bool) {
            dlog_print(DLOG_INFO, "MSF", "handleApplicationMessage 8");
            MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in "
                    "file [%s] \n",
                    __FUNCTION__, __LINE__, __FILE__);
            doApplicationCallback((Result_Base *)(temp.first));
        }
    } else {
        doApplicationCallback(NULL);
    }
}

void Channel::doApplicationCallback(Result_Base *result1) {
    dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 1");
    MSF_DBG(
        "[MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n",
        __FUNCTION__, __LINE__, __FILE__);
    if (resultobj != NULL) {
        dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 2");
        MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in file "
                "[%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
    }

    bool errorMap = errobj;

    if (errorMap != false) {
        dlog_print(DLOG_ERROR, "MSF", "doApplicationCallback 3");
        MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in file "
                "[%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        int code = errcode;
        stringstream ss;
        ss << code;
        string str = ss.str();
        if (result1 != NULL)
            result1->onError(Error::create(str));
    } else {
        dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 4");

        MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in file "
                "[%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        //	ApplicationInfo *InfoObj=new ApplicationInfo();
        if (resultobj != NULL) {
            dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 5");
            //	string json=json_object_to_json_string(resultobj);
            //	InfoObj->create(json);
            if (result1 != NULL) {
                dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 6");
                result1->onSuccess(true);
            }
        }

        if (msg_subject == Message::PROPERTY_RESULT) {
            dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 7");
            if (result1 != NULL) {
                dlog_print(DLOG_INFO, "MSF", "doApplicationCallback 8");
                result1->onSuccess(true);
            }
        }
    }
}

void Channel::handleErrorMessage(string UID) {
    Error err = Error::create(errMsg);
    handleError(UID, err);
}

void Channel::handleError(string UID, Error err) {
    if (UID.length() != 0) {
        pair<void *, int> temp = Channel::getcallback(UID);
        MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in file "
                "[%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        if (temp.first != NULL) {
            if (temp.second == Result_Client) {
                Result_Base *temp1 = (Result_Base *)(temp.first);
                if (temp1)
                    temp1->onError(err);
            } else if (temp.second == Result_bool) {
                Result_Base *temp1 = (Result_Base *)(temp.first);
                if (temp1)
                    temp1->onError(err);
            }

            if (onErrorListener)
                onErrorListener->onError();
        }
    }
}

void Channel::handleClientMessage(const char *msg, unsigned char payload[]) {
    dlog_print(DLOG_INFO, "MSF", "handleClientMessage 0");

    emit(eventType, msg, from, cl_payload, cl_payload_size);
}

void Channel::emit(string event, const char *msg, string from,
                   unsigned char *payld, int payld_size) {
    dlog_print(DLOG_INFO, "MSF", "emit");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    if (msg == NULL) {
        dlog_print(DLOG_INFO, "MSF", "emit msg = NULL");
        return;
    }

    if (message_receive_cb) {
        Message mesg(this, event, msg, from, payld, payld_size);
        message_receive_cb(mesg);
    }

    if (everyMessageListener) {
        Message mesg(this, event, msg, from, payld, payld_size);
        everyMessageListener->onMessage(mesg);
    }

    if (messageListeners.size() != 0) {
        dlog_print(DLOG_INFO, "MSF", "emit 1");

        if (messageListeners.find(event) == messageListeners.end()) {
            dlog_print(DLOG_INFO, "MSF", "map not found");
            return;
        }

        list<OnMessageListener *> &onMessageListeners =
            (messageListeners.find(event))->second;
        dlog_print(DLOG_INFO, "MSF", "emit 2");

        if (onMessageListeners.size() != 0) {
            dlog_print(DLOG_INFO, "MSF", "emit 3");
            std::list<OnMessageListener *>::const_iterator iterator;
            for (iterator = onMessageListeners.begin();
                 iterator != onMessageListeners.end(); ++iterator) {
                Message mesg(this, event, msg, from, payld, payld_size);
                (*iterator)->onMessage(mesg);
            }
        } else {
            dlog_print(DLOG_INFO, "MSF", "empty listeners");
        }
    }
}

bool Channel::connect() { return connect(connect_cb); }

bool Channel::connect(Result_Base *result1) {
    bool ret = connect(map<string, string>(), result1);
    return ret;
}

bool Channel::connect(map<string, string> attributes, Result_Base *result1) {
    dlog_print(DLOG_INFO, "MSF", "Channel::connect(at, res)");

    string uid = getUID();

    dlog_print(DLOG_INFO, "MSF", "connect call registercallback");
    registerCallback(uid, (void *)result1, Result_Client);

    UID = uid;
    if (isWebSocketOpen()) {
        handleError(uid, Error::create("Already Connected"));
        return false;
    }

    map<string, string> *at_data = new map<string, string>(attributes);
    map<Channel *, map<string, string> *> *pt_user_data =
        new map<Channel *, map<string, string> *>();
    pt_user_data->insert({this, at_data});

    int status = pthread_create(&connect_thread, NULL, pt_startConnect,
                                (void *)pt_user_data);

    if (status)
        return true;
    else
        return false;
}

void Channel::disconnect() {
    dlog_print(DLOG_INFO, "MSF", "channel disconnect()");
    disconnect(disconnect_cb);
}

void Channel::disconnect(Result_Base *result1) {
    dlog_print(DLOG_INFO, "MSF", "channel disconnect(result) 1");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    string randID = getUID();
    UID = randID;
    dlog_print(DLOG_INFO, "MSF", "disconnect call registercallback");
    registerCallback(randID, (void *)result1, Result_Client);
    string message = "";
    if (!isWebSocketOpen())
        message = "Already Disconnected";
    if (disconnecting)
        message = "Already Disconnecting";
    if (message != "")
        handleError(UID, Error::create(message));
    else
	{
        dlog_print(DLOG_INFO, "MSF", "channel disconnect(result) 2");
        MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in "
                "file [%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        disconnecting = true;
        // consume the callback
        getcallback(randID);

        dlog_print(DLOG_INFO, "MSF", "channel disconnect(result) 3");

        while (1) {
            if (mirror_lifetime > 0)
                mirror_lifetime--;

            if (!mirror_lifetime) {
                dlog_print(DLOG_INFO, "MSF", "channel disconnect(result) 4");
                break;
            }
        }

        was_closed = 1;

        if (result1) {
            dlog_print(DLOG_INFO, "MSF", "channel disconnect(result) 5");

            if (Channel::onDisconnectListener != NULL) {
                dlog_print(DLOG_INFO, "MSF", "disconnect()");
                // Channel::onDisconnectListener->onDisconnect(client);
            }

            result1->onSuccess(clients->me());
        }

        if (disconnecting) {
            disconnecting = false;
        }

        connectionHandler->stopPing();
    }
}

void Channel::handleReadyMessage() {
    waitForOnReady = false;
    std::map<void *, int>::const_iterator iterator;
    for (iterator = onReadyCallbacks.begin();
         iterator != onReadyCallbacks.end(); ++iterator) {
        Result_Base *res = (Result_Base *)iterator->first;
        onReadyCallbacks.erase(res);
        doApplicationCallback(res);
    }
    if (onConnectListener)
        onConnectListener->onConnect(clients->me());

    if (onReadyListener != NULL)
        onReadyListener->onReady();
}

void Channel::handleClientConnectMessage() {
    dlog_print(DLOG_INFO, "MSF", "handleClientConnectMessage");

    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    if (client.isHost())
        connected = true;

    clients->add(client);

    dlog_print(DLOG_INFO, "MSF", "handleClientConnectMessage client = %s",
               client.getId());
    MSF_DBG("\n Debug Log: CLIENTLIST SIZE IS [%d],  %s %d in %s \n",
            clients->size(), __FUNCTION__, __LINE__, __FILE__);
    if (onClientConnectListener != NULL)
        onClientConnectListener->onClientConnect(client);
}

void Channel::handleClientDisconnectMessage() {
    dlog_print(DLOG_INFO, "MSF", "handle client disconnect Message event = %s",
               eventType.c_str());

    if (resultresp && isLaunched) {
        dlog_print(DLOG_INFO, "MSF", "handleclientdisconnectMessage 1");
        handleSocketClosed();
        return;
    }

    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    Client client = clients->get(clientid);
    if (client.isHost()) {
        dlog_print(DLOG_INFO, "MSF", "handleclientdisconnectMessage 2");
        connected = false;
    }
    clients->remove(client);

    if (onClientDisconnectListener != NULL) {
        dlog_print(DLOG_INFO, "MSF", "handleclientdisconnectMessage 3");
        onClientDisconnectListener->onClientDisconnect(client);
    }
}

void Channel::handleConnect(string UID) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    dlog_print(DLOG_INFO, "MSF", "handleConnect");
    Client clienttemp = clients->me();

    pair<void *, int> temp;

    temp = Channel::getcallback(UID);
    if (temp.second == Result_Client) {
        Result_Base *temp1 = (Result_Base *)(temp.first);
        if (temp1)
            temp1->onSuccess(clienttemp);
    } else if (temp.second == Result_bool) {
        Result_Base *temp1 = (Result_Base *)(temp.first);
        if (temp1)
            temp1->onSuccess(true);
    }

    me = clients->me();

    if (onConnectListener)
        onConnectListener->onConnect(clienttemp);
    // To start channnel heath check
    if (isWebSocketOpen()) {
        // connectionHandler->startPing(this);
    }

    dlog_print(DLOG_INFO, "MSF", "handleConnect 1");
}

void Channel::handleSocketClosedAndNotify() {
    Client client = clients->me();
    handleSocketClosed();

    if (Channel::onDisconnectListener != NULL) {
        dlog_print(DLOG_INFO, "MSF", "handleSocketClosedAndNotify");
        Channel::onDisconnectListener->onDisconnect(client);
    }
}

void Channel::handleSocketClosed() {
    dlog_print(DLOG_INFO, "MSF", "handleSocketClosed");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    connectionHandler->stopPing();
    wsi_mirror = NULL;
    connected = false;
    was_closed = 1;
    if (disconnecting)
        disconnecting = false;
    callbacks.clear();
    clients->reset();
}

int Channel::callback_lws_mirror(struct lws *wsi,
                                 enum lws_callback_reasons reason, void *user,
                                 void *in, size_t len) {
    int n;
    void *user_data;
    Channel *this_ptr = NULL;
    struct lws_context *context = lws_get_context(wsi);
    user_data = lws_context_user(context);
    this_ptr = static_cast<Channel *>(user_data);

    if (channel_alive_map.find(this_ptr) != channel_alive_map.end()) {
        if (channel_alive_map[this_ptr] == 0) {
            return -1;
        }
    } else {
        return -1;
    }

    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        break;

    case LWS_CALLBACK_CLOSED:
        if (this_ptr->disconnecting) {
            dlog_print(DLOG_INFO, "MSF_SOCKET", "socket closed diconnecting");
            MSF_DBG("mirror: LWS_CALLBACK_CLOSED\n");
            this_ptr->wsi_mirror = NULL;
            this_ptr->mirror_lifetime = 0;
            this_ptr->disconnecting = false;

        } else {
            dlog_print(DLOG_INFO, "MSF_SOCKET",
                       "socket closed not diconnecting");
            dlog_print(DLOG_INFO, "MSF_SOCKET",
                       "call handleSocketClosedAndNotify");
            this_ptr->handleSocketClosedAndNotify();
        }
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        MSF_DBG("[MSF LOG]: LWS_CALLBACK_CLIENT_ESTABLISHED [%s]\n",
                __FUNCTION__);
        lws_callback_on_writable(wsi);
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        if (this_ptr == NULL) {
            dlog_print(DLOG_INFO, "MSF", "user ptr is NULL. return.");
            // it means Channel object was deleted
            return -1;
        } else {
            dlog_print(DLOG_INFO, "MSF", "user ptr is not NULL.");
        }

        this_ptr->eventType = "";

        if (lws_frame_is_binary(wsi)) {
            dlog_print(DLOG_INFO, "MSF", "BINARY MESSAGE ARRIVED");

            int header_size = 0;

            header_size = (int)((unsigned char *)in)[0];
            header_size = header_size << 8;
            header_size = header_size & 0xff00;
            header_size = header_size | (int)((unsigned char *)in)[1];
            dlog_print(DLOG_INFO, "MSF", "header_size = %d", header_size);

            char header_json[header_size + 1] = {0};

            memcpy(&header_json[0], &(((unsigned char *)in)[2]), header_size);
            header_json[header_size + 1] = 0;

            dlog_print(DLOG_INFO, "MSF", "in = %s", &header_json[0]);

            this_ptr->json_parse(header_json);
            memcpy(&(this_ptr->cl_payload),
                   &(((unsigned char *)in)[2 + header_size]),
                   len - 2 - header_size);

            this_ptr->cl_payload[len - 2 - header_size] = 0;
            this_ptr->cl_payload_size = len - 2 - header_size;

            dlog_print(DLOG_INFO, "MSF", "payload = %s",
                       &(this_ptr->cl_payload[0]));

            this_ptr->connectionHandler->resetLastPingReceived();

            if (this_ptr->eventType == CONNECT_EVENT) {
                this_ptr->handleConnectMessage(this_ptr->UID);
            } else {
                // this_ptr->handleMessage(this_ptr->UID);
                this_ptr->handleMessage(this_ptr->UID, this_ptr->cl_payload);
            }
        } else {
            dlog_print(DLOG_INFO, "MSF", "TEXT MESSAGE ARRIVED");
            MSF_DBG("[MSF LOG]LWS_CALLBACK_RECEIVE_ESTABLISHED:\t");
            MSF_DBG("\n%s\n", (char *)in);
            fprintf(stderr, "\n %s \n", (char *)in);
            dlog_print(DLOG_INFO, "MSF", "socket in = %s", (char *)in);
            this_ptr->json_parse((char *)in);
            this_ptr->connectionHandler->resetLastPingReceived();

            // dead code.
            // server do not send METHOD_EMIT now
            /*
               if (!((this_ptr->method).compare(Message::METHOD_EMIT))) {
               this_ptr->emit(this_ptr->eventType, (this_ptr->data).c_str(),
               this_ptr->from, this_ptr->Payload);
               break;
               }
             */
            if (this_ptr->eventType == CONNECT_EVENT) {
                this_ptr->handleConnectMessage(this_ptr->UID);
            } else {
                // this_ptr->handleMessage(this_ptr->UID);
                this_ptr->handleMessage(this_ptr->UID, NULL);
            }
        }

        dlog_print(DLOG_INFO, "MSF", "Socket receive end ");
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
        if (this_ptr->isWrite) {
            this_ptr->isWrite = false;

            if (&(this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING]) == NULL ||
                this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING] == 0 ||
                this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING] == '\0') {
                printf("\ntry write NULL data");
                fflush(stdout);
            } else {
                // printf("\ntry write data = %s",
                // &(this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING]));
                // printf("\ndata size = %d", this_ptr->buflen);
                fflush(stdout);
            }

            if (this_ptr->buflen <= 0) {
                printf("\ntry write 0 size data");
                fflush(stdout);
            }

            // printf("\nwrite socket");
            // fflush(stdout);
            if (this_ptr->binary_message) {
                n = lws_write(wsi,
                              &(this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING]),
                              this_ptr->buflen, LWS_WRITE_BINARY);
            } else {
                n = lws_write(wsi,
                              &(this_ptr->buf[LWS_SEND_BUFFER_PRE_PADDING]),
                              this_ptr->buflen, LWS_WRITE_TEXT);
            }

            if (n < 0) {
                MSF_DBG("Writing failed\n");
                dlog_print(DLOG_ERROR, "MSF", "socket write failed");
                printf("\nwrite socket failed");
                printf("\ncallback isWrite=false");
                fflush(stdout);
                return -1;
            }
        }
        lws_callback_on_writable(wsi);
        break;

    case LWS_CALLBACK_RECEIVE:
        dlog_print(DLOG_INFO, "MSF_SOCKET", "socket LWS_CALLBACK_RECEIVE %d",
                   reason);
        break;

    case LWS_CALLBACK_PROTOCOL_DESTROY:
        dlog_print(DLOG_INFO, "MSF_SOCKET",
                   "socket LWS_CALLBACK_PROTOCOL_DESTROY %d", reason);
        break;

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
    case LWS_CALLBACK_ADD_POLL_FD:
    case LWS_CALLBACK_DEL_POLL_FD:
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
    case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
    case LWS_CALLBACK_HTTP:
    case LWS_CALLBACK_HTTP_WRITEABLE:
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
    case LWS_CALLBACK_SERVER_WRITEABLE:
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
        break;

    default:
        break;
    }

    return 0;
}

Client Channel::getclient(string id) { return clients->get(id); }

bool Channel::isConnected() { return isWebSocketOpen(); }

bool Channel::isWebSocketOpen() { return wsi_mirror == NULL ? false : true; }

string Channel::getChannelUri(map<string, string> *attributes) {
    return service->getUri();
}

void Channel::publish(string event, const char *data) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    string to = "\"";
    to.append(Message::TARGET_ALL.c_str());
    to.append("\"");

    publishMessage(event, data, to.c_str(), NULL, 0);
}

void Channel::publish(string event, const char *data, unsigned char payload[],
                      int payload_size) {
    dlog_print(DLOG_INFO, "MSF", "publishMessage");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    string to = "\"";
    to.append(Message::TARGET_ALL.c_str());
    to.append("\"");
    publishMessage(event, data, to.c_str(), payload, payload_size);
}

void Channel::publish(string event, const char *data, const char *target) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    string to = "\"";
    to.append(target);
    to.append("\"");
    publishMessage(event, data, to.c_str(), NULL, 0);
}

void Channel::publish(string event, const char *data, const char *target,
                      unsigned char payload[], int payload_size) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    string to = "\"";
    to.append(target);
    to.append("\"");
    publishMessage(event, data, to.c_str(), payload, payload_size);
}

void Channel::publish(string event, const char *data, Client client) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    string to = "\"";
    to.append(client.getId());
    to.append("\"");
    publishMessage(event, data, to.c_str(), NULL, 0);
}

void Channel::publish(string event, const char *data, Client client,
                      unsigned char payload[], int payload_size) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    string to = "\"";
    to.append(client.getId());
    to.append("\"");
    publishMessage(event, data, to.c_str(), payload, payload_size);
}

void Channel::publish(string event, const char *data, list<Client> clients) {
    publish(event, data, clients, NULL, 0);
}

void Channel::publish(string event, const char *data, list<Client> clients,
                      unsigned char payload[], int payload_size) {
    string to = "[";

    std::list<Client>::iterator iterator;
    for (iterator = clients.begin(); iterator != clients.end(); ++iterator) {
        to.append("\"");
        to.append(iterator->getId());
        to.append("\"");

        if (std::next(iterator, 1) != clients.end()) {
            to.append(",");
        } else {
            to.append("]");
        }
    }

    // TODO
    publishMessage(event, data, to.c_str(), payload, payload_size);
}

void Channel::publishMessage(string event, const char *data, const char *to,
                             unsigned char payload[], int payload_size) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    publishMessage(Message::METHOD_EMIT, event, data, to, payload,
                   payload_size);
}

void Channel::publishMessage(string method, string event, const char *data,
                             const char *to, unsigned char payload[],
                             int payload_size) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    if (!isWebSocketOpen()) {
        handleError(string(), Error::create("Not Connected"));
        return;
    } else {
        // int l=0;
        // l += sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING]," {\n
        // \"method\": \"%s\",\n \"params\": {\n \"event\": \"%s\",\n \"data\":
        // \"%s\",\n \"to\": \"%s\"\n }\n}",method.c_str(),event.c_str(),
        // (unsigned char*)data, (unsigned char*)to);

        // unsigned char  *bufer=new char[LWS_SEND_BUFFER_PRE_PADDING + 4096
        // +LWS_SEND_BUFFER_POST_PADDING];
        // bufer=prepareMessageMap(method,event,data,to);

        long prepare_buf_len = 0;
        unsigned char *prepare_buf = prepareMessageMap(
            method, event, data, to, &prepare_buf_len, payload, payload_size);

        memcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING],
               &prepare_buf[LWS_SEND_BUFFER_PRE_PADDING], prepare_buf_len);

        buf[LWS_SEND_BUFFER_PRE_PADDING + prepare_buf_len] = 0;

        buflen = prepare_buf_len;
        // buf code
        // for(int i=0; i<buflen;i++)
        //	buf[i]=prepare_buf[i];

        // buf[buflen] = 0;

        delete[](prepare_buf);

        MSF_DBG("buf is = %s %d \n", &buf[LWS_SEND_BUFFER_PRE_PADDING], buflen);
        // buflen=l;
        // messagedata=((char*)data);

        if (binary_message) {
            dlog_print(DLOG_INFO, "MSF", "publish buffer = %s",
                       &buf[LWS_SEND_BUFFER_PRE_PADDING + 2]);
        } else {
            dlog_print(DLOG_INFO, "MSF", "publish buffer = %s",
                       &buf[LWS_SEND_BUFFER_PRE_PADDING]);
        }

        // if (bufer)
        //	free(bufer);

        isWrite = true;
        // printf("\npublish isWrite=true\n");
    }
}

unsigned char *Channel::prepareMessageMap(string method, string event,
                                          const char *data, const char *to,
                                          long *prepare_buf_len,
                                          unsigned char payload[],
                                          int payload_size) {
    int l = 0;
    int header_size = 0;

	int prepare_buf_size = LWS_SEND_BUFFER_PRE_PADDING + 4096 + LWS_SEND_BUFFER_POST_PADDING;
    unsigned char *prepare_buf = new unsigned char[prepare_buf_size];

    if (payload) {
        l += snprintf((char *)&prepare_buf[LWS_SEND_BUFFER_PRE_PADDING + 2],
					prepare_buf_size - (LWS_SEND_BUFFER_PRE_PADDING + 2),
					"{\n \"method\": \"%s\",\n \"params\": {\n \"event\": "
					"\"%s\",\n \"data\": \"%s\",\n \"to\": %s\n }\n}",
					method.c_str(), event.c_str(), (unsigned char *)data,
					(unsigned char *)to);

        header_size = l;

        prepare_buf[LWS_SEND_BUFFER_PRE_PADDING + 1] =
            (unsigned char)header_size;
        prepare_buf[LWS_SEND_BUFFER_PRE_PADDING] =
            (unsigned char)((header_size) >> 8);

        dlog_print(DLOG_INFO, "MSF", "publish header size = %d", header_size);
        dlog_print(DLOG_INFO, "MSF", "buf[1] = %d",
                   (char)prepare_buf[LWS_SEND_BUFFER_PRE_PADDING + 1]);
        dlog_print(DLOG_INFO, "MSF", "buf[0] = %d",
                   (char)prepare_buf[LWS_SEND_BUFFER_PRE_PADDING]);

        l += 2;

        memcpy(&prepare_buf[LWS_SEND_BUFFER_PRE_PADDING + l], payload,
               payload_size);

        l += payload_size;

        binary_message = true;
    } else {
        l += snprintf((char *)&prepare_buf[LWS_SEND_BUFFER_PRE_PADDING],
					prepare_buf_size - LWS_SEND_BUFFER_PRE_PADDING,
					"{\n \"method\": \"%s\",\n \"params\": {\n \"event\": "
					"\"%s\",\n \"data\": \"%s\",\n \"to\": %s\n }\n}",
					method.c_str(), event.c_str(), (unsigned char *)data,
					(unsigned char *)to);
        binary_message = false;
    }
    MSF_DBG("buf is = %s %d \n", &buf[LWS_SEND_BUFFER_PRE_PADDING], l);

    *prepare_buf_len = l;
    return prepare_buf;
}

void Channel::handleBinaryMessage(unsigned char payload[]) {}

void Channel::start_app(char *data, int buflength, string msgID) {
    dlog_print(DLOG_INFO, "MSF", "start_app() buf = %s", data);
    UID = msgID;
    int l = 0;

    l += snprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING], sizeof(buf) - LWS_SEND_BUFFER_PRE_PADDING, data);
    MSF_DBG("buf is = %s %d \n", &buf[LWS_SEND_BUFFER_PRE_PADDING], l);
    buflen = l;
    buf[LWS_SEND_BUFFER_PRE_PADDING + l] = 0;

    dlog_print(DLOG_INFO, "MSF", "start_app() buf = %s",
               &buf[LWS_SEND_BUFFER_PRE_PADDING]);
    binary_message = false;

    isWrite = true;
    printf("start_app isWrite=true");

    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
}

void Channel::registerCallback(string uid, void *callback, int value_type) {
    dlog_print(DLOG_INFO, "MSF", "registerCallback uid = %s value_type = %d",
               uid.c_str(), value_type);
    callbacks[uid] = make_pair(callback, value_type);
}

void Channel::release_callback(string uid) { callbacks.erase(uid); }

pair<void *, int> Channel::getcallback(string uid) {
    map<string, pair<void *, int>>::iterator check_it = callbacks.find(uid);

    if (check_it == callbacks.end()) {
        dlog_print(DLOG_ERROR, "MSF",
                   "callbacks map not found. critical error");
        return pair<void *, int>();
    }

    if (uid.length() != 0) {
        pair<void *, int> r1 = callbacks[uid];

        callbacks.erase(uid);

        dlog_print(DLOG_INFO, "MSF", "getcallback success uid = %s",
                   uid.c_str());
        return r1;
    }

    dlog_print(DLOG_INFO, "MSF", "getcallback failed", uid.c_str());
    return pair<void *, int>();
}

string Channel::getUID() {
    std::stringstream ss;
	unsigned int seed = time(NULL);
    ss << (rand_r(&seed) % 9000 + 1000);
    string randID = ss.str();
    return randID;
}

void *Channel::pt_startConnect(void *att) {
    map<Channel *, map<string, string> *> *pt_user_data =
        static_cast<map<Channel *, map<string, string> *> *>(att);
    map<string, string> *attributes = pt_user_data->begin()->second;

    pt_user_data->begin()->first->create_websocket(attributes);

    delete attributes;
    delete pt_user_data;

    return NULL;
}

void Channel::create_websocket(void *att) {
    struct lws_protocols protocols[] = {
        {"lws-mirror-protocol", Channel::callback_lws_mirror, sizeof(int), 0, 0,
         NULL},

        // libwebsockets 1.7 has a bug.
        {NULL, NULL, 0, 0, 0, NULL} // this is anti-bug code
    };

    int port = 8001;
    int use_ssl = 0;
    int n = 0;
    // int ret = 0;
    map<string, string> *attributes = (map<string, string> *)(att);
    string uri = getChannelUri(attributes);

    was_closed = 0;
    struct lws_context *context;

    int ietf_version = -1; /* latest */

    struct lws_context_creation_info info;

    memset(&info, 0, sizeof info);

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.extensions = NULL;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.user = this;

#ifndef LWS_NO_EXTENSIONS
// info.extensions = lws_get_internal_extensions();
#endif

    if (isWebSocketOpen()) {
        MSF_DBG("\nAlready Connected");
        dlog_print(DLOG_INFO, "MSF", "create_websocket already Connected");
        // return 0;
    }
    context = lws_create_context(&info);
    Context = context;
    if (context == NULL) {
        MSF_DBG("Creating libwebsocket context failed\n");
        dlog_print(DLOG_ERROR, "MSF", "create_websocket context failed");
        // return 1;
    }
    n = 0;
    string address = getipaddressfromUri(uri);
    string api = getapifromUri(uri);
    api.append("channels/").append(ChannelID);

    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);

    struct lws_client_connect_info connect_info;
    connect_info.context = context;
    connect_info.address = address.c_str();
    connect_info.port = port;
    connect_info.ssl_connection = use_ssl;
    connect_info.path = api.c_str();
    connect_info.host = address.c_str();
    connect_info.origin = address.c_str();
    connect_info.protocol = protocols[0].name;
    connect_info.ietf_version_or_minus_one = ietf_version;
    connect_info.userdata = NULL;
    connect_info.client_exts = NULL;

    // loop until socket closed
    while (n >= 0 && !was_closed) {
        n = lws_service(context, 500);
        if (n < 0)
            continue;
        if (wsi_mirror)
            continue;
        if (was_closed)
            goto bail;
        MSF_DBG("Creating wsi_mirror [%s] [%d]\n", __FUNCTION__, __LINE__);
        // wsi_mirror = lws_client_connect_via_info(context, address.c_str(),
        // port, use_ssl,  api.c_str(),address.c_str(),address.c_str(),
        // protocols[0].name, ietf_version);
        wsi_mirror = lws_client_connect_via_info(&connect_info);
        if (wsi_mirror == NULL) {
            MSF_DBG("Fail to create was_mirror[%s] [%d]\n", __FUNCTION__,
                    __LINE__);
            dlog_print(DLOG_ERROR, "MSF", "create_websocket create wsi failed");
            handleError(UID, Error::create("ConnectFailed"));
            // ret = 1;
            goto bail;
        }
    }
//	return 0;
bail:
    dlog_print(DLOG_INFO, "MSF", "create_websocket destroy context");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    if (context)
        lws_context_destroy(context);
    context = NULL;
    wsi_mirror = NULL;
}

string Channel::getipaddressfromUri(string uri) {
    string startpt = "http://";
    const char *src = strstr(uri.c_str(), startpt.c_str());
    int startpoint = (src - uri.c_str()) + sizeof(startpt);

    std::string endpt = ":8001";
    const char *dest = strstr(src, endpt.c_str());
    int endpoint = (dest - uri.c_str());
    std::string address =
        uri.substr((startpoint + sizeof(startpt) - 1),
                   (endpoint - startpoint + 1 - sizeof(startpt)));
    return address;
}

string Channel::getapifromUri(string uri) {
    string startpt = "/api";
    const char *api = strstr(uri.c_str(), startpt.c_str());
    return api;
}
void Channel::setonConnectListener(OnConnectListener *obj) {
    onConnectListener = obj;
}

void Channel::unsetonConnectListener() { onConnectListener = NULL; }

void Channel::setonDisconnectListener(OnDisconnectListener *obj) {
    onDisconnectListener = obj;
}

void Channel::unsetonDisconnectListener() { onDisconnectListener = NULL; }
void Channel::setonClientConnectListener(OnClientConnectListener *obj) {
    onClientConnectListener = obj;
}

void Channel::unsetonClientConnectListener() { onClientConnectListener = NULL; }

/*
   void Channel::setmessageListeners(list<OnMessageListener>
   onMessageListenerlist){
   std::list<OnMessageListener>::const_iterator iterator;
   std::list<OnMessageListener>::const_iterator it;
   for (iterator = onMessageListenerlist.begin(),it =onMessageListeners.begin();
   iterator != onMessageListenerlist.end(); ++iterator,++it)
   {
   it=iterator;
   }

   }
   }
 */

void Channel::addOnMessageListener(string event,
                                   OnMessageListener *onMessageListener) {
    if (event.length() == 0 || onMessageListener == NULL)
        return;

    if (messageListeners.find(event) == messageListeners.end()) {
        messageListeners.insert({event, list<OnMessageListener *>()});
    }

    list<OnMessageListener *> &onMessageListeners =
        (messageListeners.find(event))->second;

    onMessageListeners.push_back(onMessageListener);
}

void Channel::addOnAllMessageListener(OnMessageListener *onMessageListener) {
    everyMessageListener = onMessageListener;
}

void Channel::removeAllMessageListener() { everyMessageListener = NULL; }

void Channel::removeOnMessageListeners(string event) {
    if (event.length() == 0)
        return;

    if (messageListeners.find(event) == messageListeners.end()) {
        return;
    }

    list<OnMessageListener *> &onMessageListeners =
        (messageListeners.find(event))->second;
    onMessageListeners.clear();
}

void Channel::removeOnMessageListener(string event,
                                      OnMessageListener *onMessageListener) {
    if ((event.length() == 0) || (onMessageListener != NULL))
        return;

    if (messageListeners.find(event) == messageListeners.end()) {
        return;
    }

    list<OnMessageListener *> &onMessageListeners =
        (messageListeners.find(event))->second;

    if (onMessageListeners.size() != 0) {
        onMessageListeners.remove(onMessageListener);
    }
}

void Channel::removeOnMessageListeners() { messageListeners.clear(); }

void Channel::removeAllListeners() {
    setonConnectListener(NULL);
    setonDisconnectListener(NULL);
    setonClientConnectListener(NULL);
    setonClientDisconnectListener(NULL);
    setonReadyListener(NULL);
    setonErrorListener(NULL);
    removeOnMessageListeners();
}

void Channel::register_message_receive_cb(_message_receive_cb cb) {
    message_receive_cb = cb;
}

void Channel::setonClientDisconnectListener(OnClientDisconnectListener *obj) {
    onClientDisconnectListener = obj;
}

void Channel::unsetonClientDisconnectListener() {
    onClientDisconnectListener = NULL;
}

void Channel::setonErrorListener(OnErrorListener *obj) {
    onErrorListener = obj;
}

void Channel::unsetonErrorListener() { onErrorListener = NULL; }

void Channel::setonReadyListener(OnReadyListener *obj) {
    onReadyListener = obj;
}

void Channel::unsetonReadyListener() { onReadyListener = NULL; }

void Channel::set_isWrite(bool flag) {}

void ChannelConnectionHandler::resetLastPingReceived() {
    this->lastPingReceived = time(0);
}

void ChannelConnectionHandler::calculateAverageRT() {
    long lastRT = lastPingReceived - pingSent;
    if (lastRT > longestRT) {
        longestRT = lastRT;
    }
    // average=((numPings++ * average)+lastRT)/numPings; //will check ankit
}

void ChannelConnectionHandler::stopPing() {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    if (ping_thread != 0) {
        MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in "
                "file [%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        pthread_cancel(ping_thread);
        MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in "
                "file [%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        pthread_join(ping_thread, NULL);
        MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in "
                "file [%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        running = false;
        ping_thread = 0;
        printf("ping thread close");
        fflush(stdout);
    }
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
}

void ChannelConnectionHandler::startPing(Channel *ptr) {
    dlog_print(DLOG_INFO, "MSF", "## startPing ###");
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    if (running) {
        dlog_print(DLOG_INFO, "MSF",
                   "## startPing already running. return ###");
        return;
    }
    stopPing();
    if (pingTimeout <= 0) {
        dlog_print(DLOG_INFO, "MSF", "## startPing ping timeout. return ###");
        MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in "
                "file [%s] \n",
                __FUNCTION__, __LINE__, __FILE__);
        return;
    }
    running = true;
    numPings = 0;
    average = 0;
    longestRT = 0;
    startTime = time(0);
    pingSent = startTime;
    channel_ptr = ptr;

    int err = pthread_create(&ping_thread, NULL, Pinging, ptr);
    if (err) {
        dlog_print(DLOG_INFO, "MSF", "pthread_create failed err = %d", err);
    }
    // Need to check this
    // Pinging(ptr);
}

void ChannelConnectionHandler::ping_again(void *arg) {
    dlog_print(DLOG_INFO, "MSF", "## ping again ###");
    Channel *ptr = static_cast<Channel *>(arg);
    long now = time(0);

    if (now > ptr->connectionHandler->lastPingReceived +
                  ptr->connectionHandler->pingTimeout) {
        dlog_print(DLOG_INFO, "MSF", "## Pinging timeout. disconnect ###");
        ptr->disconnect();
    } else {
        ptr->publish("msfVersion2", "msfVersion2", ptr->clients->me());
        sleep(1);
        ptr->publish(PING, PONG.c_str(), ptr->clients->me());
        ptr->connectionHandler->pingSent = time(0);
    }
}

void *ChannelConnectionHandler::Pinging(void *arg) {
    dlog_print(DLOG_INFO, "MSF", "## Pinging ###");

    Channel *ptr = static_cast<Channel *>(arg);

    while (1) {
        ping_again(ptr);
        dlog_print(DLOG_INFO, "MSF", "## pinging 1 ###");
        // usleep(ptr->connectionHandler->pingTimeout);
        sleep(5);
    }
}

void ChannelConnectionHandler::setPingTimeout(long t) {
    MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file "
            "[%s] \n",
            __FUNCTION__, __LINE__, __FILE__);
    pingTimeout = t;
}

void Channel::set_connect_result(Result_Base *r) { connect_cb = r; }

void Channel::unset_connect_result() { connect_cb = NULL; }

void Channel::set_disconnect_result(Result_Base *r) { disconnect_cb = r; }

void Channel::unset_disconnect_result() { disconnect_cb = NULL; }
