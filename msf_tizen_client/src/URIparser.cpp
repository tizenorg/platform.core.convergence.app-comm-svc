#include "URIparser.h"
#include <stdio.h>
#include <cstdlib>
#include <cstring>

namespace uri_parser {

const std::string URIParse::URIRESERVED_PATH     = "?#";
const std::string URIParse::URIRESERVED_QUERY    = "#";
const std::string URIParse::URIRESERVED_FRAGMENT = "";
const std::string URIParse::URIILLEGAL           = "%<>{}|\\\"^`";

URIParse::URIParse():
	_URIport(0), _URIflags(0)
{
}

URIParse::URIParse(const std::string& uri):
	_URIport(0), _URIflags(0)
{
	parseURI(uri);
}

URIParse::URIParse(const char* uri):
	_URIport(0), _URIflags(0)
{
	parseURI(std::string(uri));
}

URIParse::~URIParse()
{
}

void URIParse::clear()
{
	_URIscheme.clear();
	_URIuserInfo.clear();
	_URIhost.clear();
	_URIport = 0;
	_URIportStr.clear();
	_URIpath.clear();
	_URIquery.clear();
	_URIfragment.clear();
	_URIflags = 0;
}

void URIParse::setURIScheme(const std::string& scheme)
{
	_URIscheme = scheme;
	toLowerInPlaceURI(_URIscheme);
	if (_URIport == 0) {
		_URIport = getWellKnownPortforURI();
	}
}

bool URIParse::empty() const
{
	return _URIscheme.empty() && _URIhost.empty() && _URIpath.empty() && _URIquery.empty() && _URIfragment.empty();
}

void URIParse::decodeURI(const std::string& str, std::string& decodedStr, long _flags2)
{
	std::string::const_iterator it  = str.begin();
	std::string::const_iterator end = str.end();

	while (it != end) {
		char c = *it++;
		if (c == '%') {
			if (it == end) {
				if (_flags2 & 0x1)
					printf("ERROR: URI encoding: no hex digit following percent sign.\n");
				return;
			}

			char hi = *it++;

			if (it == end) {
				if (_flags2 & 0x1)
					printf("ERROR: URI encoding: two hex digits must follow percent sign.\n");
				return;
			}

			char lo = *it++;

			if (hi >= '0' && hi <= '9') {
				c = hi - '0';
			} else if (hi >= 'A' && hi <= 'F') {
				c = hi - 'A' + 10;
			} else if (hi >= 'a' && hi <= 'f') {
				c = hi - 'a' + 10;
			} else {
				if (_flags2 & 0x1)
					printf("ERROR: URI encoding: not a hex digit.\n");
				return;
			}
			c *= 16;
			if (lo >= '0' && lo <= '9') {
				c += lo - '0';
			} else if (lo >= 'A' && lo <= 'F') {
				c += lo - 'A' + 10;
			} else if (lo >= 'a' && lo <= 'f') {
				c += lo - 'a' + 10;
			} else {
				if (_flags2 & 0x1)
					printf("ERROR: URI encoding: not a hex digit.\n");
				return;
			}
		}
		decodedStr += c;
	}
}

unsigned short URIParse::getWellKnownPortforURI() const
{
	if (_URIscheme == "ftp")
		return 21;
	else if (_URIscheme == "ssh")
		return 22;
	else if (_URIscheme == "telnet")
		return 23;
	else if (_URIscheme == "http")
		return 80;
	else if (_URIscheme == "nntp")
		return 119;
	else if (_URIscheme == "ldap")
		return 389;
	else if (_URIscheme == "https")
		return 443;
	else if (_URIscheme == "rtsp")
		return 554;
	else if (_URIscheme == "sip")
		return 5060;
	else if (_URIscheme == "sips")
		return 5061;
	else if (_URIscheme == "xmpp")
		return 5222;
	else
		return 0;
}

void URIParse::parseURI(const std::string& uri)
{
	std::string::const_iterator it  = uri.begin();
	std::string::const_iterator end = uri.end();
	if (it == end) return;
	if (*it != '/' && *it != '.' && *it != '?' && *it != '#') {
		std::string scheme;
		while (it != end && *it != ':' && *it != '?' && *it != '#' && *it != '/') scheme += *it++;
		if (it != end && *it == ':') {
			++it;

			if (it == end) {
				if (_URIflags & 0x1)
					printf("ERROR: URI scheme must be followed by authority or path.\n");
				return;
			}
			setURIScheme(scheme);
			if (*it == '/') {
				++it;
				if (it != end && *it == '/') {
					++it;
					URIparseAuthority(it, end);
				} else {
					--it;
				}
			}
			URIparsePathEtc(it, end);
		} else {
			it = uri.begin();
			URIparsePathEtc(it, end);
		}
	} else {
		URIparsePathEtc(it, end);
	}
}

void URIParse::URIparseAuthority(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string userInfo;
	std::string part;
	while (it != end && *it != '/' && *it != '?' && *it != '#')	{
		if (*it == '@')	{
			userInfo = part;
			part.clear();
		} else {
			part += *it;
		}
		++it;
	}
	std::string::const_iterator pbeg = part.begin();
	std::string::const_iterator pend = part.end();
	URIparseHostAndPort(pbeg, pend);
	_URIuserInfo = userInfo;
}

void URIParse::URIparseHostAndPort(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	if (it == end) return;

	std::string host;
	if (*it == '[')	{
		++it;
		while (it != end && *it != ']') host += *it++;

		if (it == end) {
			if (_URIflags & 0x1)
				printf("ERROR: unterminated IPv6 address.\n");
			return;
		}
		++it;
	} else {
		while (it != end && *it != ':') host += *it++;
	}

	if (it != end && *it == ':') {
		++it;
		std::string port;
		while (it != end) port += *it++;
		if (!port.empty()) {
			int nport = 0;

			_URIportStr = port.c_str();
			nport = atoi(port.c_str());
			if (nport < 0) {
				_URIport = 0;
			} else if (nport > 65535) {
				_URIport = 65535;
			} else {
				_URIport = (unsigned short) nport;
			}
		} else {
			_URIport = getWellKnownPortforURI();
		}
	} else {
		_URIport = getWellKnownPortforURI();
	}
	_URIhost = host;
	toLowerInPlaceURI(_URIhost);
}

void URIParse::URIparsePath(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string path;
	while (it != end && *it != '?' && *it != '#') path += *it++;
	decodeURI(path, _URIpath, _URIflags);
}

void URIParse::URIparsePathEtc(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	if (it == end) return;

	if (*it != '?' && *it != '#')
		URIparsePath(it, end);

	if (it != end && *it == '?') {
		++it;
		URIparseQuery(it, end);
	}

	if (it != end && *it == '#') {
		++it;
		URIparseFragment(it, end);
	}
}

void URIParse::URIparseQuery(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	_URIquery.clear();
	while (it != end && *it != '#') _URIquery += *it++;
}

void URIParse::URIparseFragment(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string fragment;
	while (it != end) fragment += *it++;
	decodeURI(fragment, _URIfragment, _URIflags);
}

bool URIParse::isIPAddressinURI(const std::string& str)
{
	const char *str2 = str.c_str();
	const char *ptr = strrchr(str2, (int)'.');
	if (ptr && *ptr == '.') {
		++ptr;
		if (strchr("0123456789", *ptr))
			return 1;
	}
	return 0;
}

}
