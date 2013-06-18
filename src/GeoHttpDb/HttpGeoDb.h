#ifndef HTTPGEODB_H_
#define HTTPGEODB_H_

#include "../Http/HttpTask.h"
#include <string>
class GeoHttpTask:public HttpTask
{
public:
	GeoHttpTask(int fd,CHttpHost *host);
	void ProcessRequest();
	void ProcessGeoPoint(UriSplit &urisplit,CHttpServerResponse &response,std::string &rescontent);

	static FileTask *NewGeoHttpTask(int fd,CHttpHost* host);
};
#endif /* HTTPGEODB_H_ */
