#ifndef _URI_PARSER_H_
#define _URI_PARSER_H_

#include <vector>
#include <string>

namespace uri_parser
{

class URIParse
{
public:

	URIParse();

	URIParse(const std::string& uri);

	URIParse(const char *uri);

	~URIParse();
	void clear();

	const std::string& getURIScheme() const;

	void setURIScheme(const std::string& scheme);

	const std::string& getURIHost() const;

	bool empty() const;

	static void decodeURI(const std::string& str, std::string& decodedStr, long _flags2);

	static bool isIPAddressinURI(const std::string& str);

protected:

	unsigned short getWellKnownPortforURI() const;

	void parseURI(const std::string& uri);

	void URIparseAuthority(std::string::const_iterator& it, const std::string::const_iterator& end);

	void URIparseHostAndPort(std::string::const_iterator& it, const std::string::const_iterator& end);

	void URIparsePath(std::string::const_iterator& it, const std::string::const_iterator& end);

	void URIparsePathEtc(std::string::const_iterator& it, const std::string::const_iterator& end);

	void URIparseQuery(std::string::const_iterator& it, const std::string::const_iterator& end);

	void URIparseFragment(std::string::const_iterator& it, const std::string::const_iterator& end);

	static const std::string URIRESERVED_PATH;
	static const std::string URIRESERVED_QUERY;
	static const std::string URIRESERVED_FRAGMENT;
	static const std::string URIILLEGAL;

private:
	template <class S>
	S& toLowerInPlaceURI(S& str){
		typename S::iterator it  = str.begin();
		typename S::iterator end = str.end();
		while (it != end) { *it = tolower(*it); ++it; }
		return str;
	}
	std::string    _URIscheme;
	std::string    _URIuserInfo;
	std::string    _URIhost;
	unsigned short _URIport;
	std::string    _URIportStr;
	std::string    _URIpath;
	std::string    _URIquery;
	std::string    _URIfragment;
	long           _URIflags;
};

inline const std::string& URIParse::getURIScheme() const
{
	return _URIscheme;
}

inline const std::string& URIParse::getURIHost() const
{
	return _URIhost;
}

}

#endif //_URI_PARSER_H_
