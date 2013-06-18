/*
 * HttpServerRequest.h
 *
 *  Created on: Jan 14, 2012
 *      Author: sa
 */

#ifndef HTTPSERVERREQUEST_H_
#define HTTPSERVERREQUEST_H_
#include <string>
#include <map>
#include <vector>
#include "../tools/stdext.h"
class CHttpServerRequest
{
private:
	std::string act;
	std::string uri;
	std::string vision;

	typedef std::map<std::string,std::string,string_less_nocase> HeadMap;
	HeadMap headmap;
	ByteStream::LPByteStream bodystream;
	std::vector<std::string> headlines;

	std::string inputbuffer;
	enum{RecvFirstLine,RecvStatLine,RecvBody};
	int state;
	bool resault;

	int torecvbody;
public:
	std::string GetHead(const std::string &key) const;
	bool getContentLength(int &value)const;
	bool Resault()const{return resault;}
	std::string Act()const{return act;}
	std::string Uri()const{return uri;}
	std::string Vision() const{return vision;}
	ByteStream::LPByteStream Body(){return bodystream;}
	CHttpServerRequest();
	void Init();
	bool InputChar(char one);
	bool InputBuffer(char* buf,size_t size,size_t &proced);
	static std::string HttpDecodeUri(std::string &srcuri);

	void DebugPrint();
};

class UriSplit
{
private:
	std::vector<std::string> path;
	typedef std::map<std::string,std::string,string_less_nocase> Params;
	Params params;
	std::string fragment;
public:
	const std::string GetParam(const std::string name) const;
	bool ParamExist(const std::string name) const;
	const std::vector<std::string>& GetPath()const{return path;}
	bool Decode(const char* str);
};
#endif /* HTTPSERVERREQUEST_H_ */
