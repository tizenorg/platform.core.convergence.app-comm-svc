#include"Error.h"

Error::Error()
{
}

Error::Error(long code, string name, string message)
{
	this->code = code;
	this->name = name;
	this->message = message;
}

Error Error::create(string message)
{
	return create(-1, "error", message);
}

Error Error::create(long code, string name, string message)
{
	if ((name == "") || (message == "")) {
		name = "error";
	}

	Error *error = new Error(code, name, message);
	return *error;
}
const char *Error::get_error_message()
{
	return message.c_str();
}
