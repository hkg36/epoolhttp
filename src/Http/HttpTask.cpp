/*
 * HttpTask.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: sa
 */

#include "HttpTask.h"
#include <sys/param.h>
#include <stdio.h>
HttpTask::HttpTask(int fd, CHttpHost *host) :
	FileTask(fd, host) {
	// TODO Auto-generated constructor stub

}

HttpTask::~HttpTask() {
	// TODO Auto-generated destructor stub
}

int HttpTask::datainput(LPCBUFFER data) {
	size_t proced = 0;
	char* buf = (char*) data->Buffer();
	size_t datalen = data->datalen;
	bool procres = true;
	while (datalen > 0) {
		procres = request.InputBuffer(buf, datalen, proced);
		buf += proced;
		datalen -= proced;
		if (procres == false) {
			ProcessRequest();
			request.Init();
		}
	}
	WriteOutFlush();
	if (!(procres == true && datalen == 0))
		setError();
	return 0;
}
int HttpTask::fail(int code) {
	return 0;
}

void HttpTask::ProcessRequest() {
	CHttpServerResponse response;
	response.Vision(request.Vision());
	response.Message(200);
	std::string content = "it worked!";
	response.setContentLength(content.size());
	std::string maintype("text");
	std::string subtype("plain");
	response.setContentType(maintype, subtype);

	std::string reshead = response.SaveHead();
	WriteOut(reshead.c_str(), reshead.size());
	WriteOut(content.c_str(), content.size());
}

void HttpTask::WriteOut(const char* data, int len) {
	while (len > 0) {
		if (writeoutbuffer) {
			if (writeoutbuffer->BufLen() == writeoutbuffer->datalen) {
				GetHost()->AddWriteQueue(getFD(), writeoutbuffer);
				writeoutbuffer = NULL;
			}
		}
		if (writeoutbuffer == NULL) {
			writeoutbuffer = CBuffer::getBuffer(1024);
		}
		int towrite = MIN(writeoutbuffer->BufLen()-writeoutbuffer->datalen,len);
		memcpy(writeoutbuffer->Buffer() + writeoutbuffer->datalen, data,
				towrite);
		writeoutbuffer->datalen += towrite;
		len -= towrite;
		data += towrite;
	}
}
void HttpTask::WriteOutFlush() {
	if (writeoutbuffer && writeoutbuffer->BufLen() > 0) {
		GetHost()->AddWriteQueue(getFD(), writeoutbuffer);
		writeoutbuffer = NULL;
	}
}
