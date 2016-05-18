#ifndef _MSF_ERROR_H
#define _MSF_ERROR_H

#include <stdio.h>
#include<iostream>
#include<string>

using namespace std;

class Error
{
private:
	long code;
	string name;
	string message;

public:
	Error();
	Error(long code, string name, string message);
	static Error create(string message);
	static Error create(long code, string name , string message);
	const char *get_error_message();
	//write other create funcs
};

#endif
