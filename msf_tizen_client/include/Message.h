#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "Client.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

class Message
{
public:
	Channel *m_channel;
	string m_event;
	string m_data;
	string m_from;

	unsigned char *m_payload;
	int m_payload_size;
	//std::vector<unsigned char> m_payload;

public:
	static string TARGET_BROADCAST;
	static string TARGET_ALL;
	static string TARGET_HOST;

	static string METHOD_APPLICATION_GET;
	static string METHOD_APPLICATION_START;
	static string METHOD_APPLICATION_STOP;
	static string METHOD_APPLICATION_INSTALL;
	static string METHOD_WEB_APPLICATION_GET;
	static string METHOD_WEB_APPLICATION_START;
	static string METHOD_WEB_APPLICATION_STOP;
	static string METHOD_EMIT;

	static string PROPERTY_MESSAGE;
	static string PROPERTY_METHOD;
	static string PROPERTY_MESSAGE_ID;
	static string PROPERTY_PARAMS;
	static string PROPERTY_ID;
	static string PROPERTY_URL;
	static string PROPERTY_ARGS;
	static string PROPERTY_EVENT;
	static string PROPERTY_DATA;
	static string PROPERTY_TO;
	static string PROPERTY_FROM;
	static string PROPERTY_CLIENTS;
	static string PROPERTY_RESULT;
	static string PROPERTY_ERROR;

	Message(Channel *ch, string event, string data, string from, unsigned char *payload, int payload_size);
	Message();
	virtual ~Message();
	Message(const Message&);
};

#endif
