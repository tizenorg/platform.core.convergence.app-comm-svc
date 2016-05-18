#ifndef _MSF_RESULT_H_
#define _MSF_RESULT_H_

#include <iostream>

using namespace std;

class Service;
class ApplicationInfo;
class Client;
class Device;
class Error;

//template <class T>

typedef enum {
	RESULT_BASE_APP_INFO,
	RESULT_BASE_CLIENT,
	RESULT_BASE_DEVICE,
	RESULT_BASE_BOOL,
	RESULT_BASE_SERVICE,
	RESULT_BASE_ERROR
} result_base_e;

typedef void(*_appinfo_result_cb)(ApplicationInfo appinfo, void *user_data);
typedef void(*_client_result_cb)(Client client, void *user_data);
typedef void(*_device_result_cb)(Device dev, void *user_data);
typedef void(*_bool_result_cb)(bool result, void *user_data);
typedef void(*_service_result_cb)(Service service, void *user_data);
typedef void(*_error_result_cb)(Error err, void *user_data);

class Result_Base
{
public:

	_appinfo_result_cb appinfo_result_cb = NULL;
	_client_result_cb client_result_cb = NULL;
	_device_result_cb device_result_cb = NULL;
	_bool_result_cb bool_result_cb = NULL;
	_service_result_cb service_result_cb = NULL;
	_error_result_cb error_result_cb = NULL;
	void *user_data;
	Result_Base();
	Result_Base(void*, void*, result_base_e);
	virtual ~Result_Base();

	virtual void onSuccess(ApplicationInfo argu);
	virtual void onSuccess(Client argu);
	virtual void onSuccess(Device argu);
	virtual void onSuccess(bool argu);
	virtual void onSuccess(Service argu);
	virtual void onError(Error);
};

#endif

