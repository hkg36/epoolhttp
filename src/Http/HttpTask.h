/*
 * HttpTask.h
 *
 *  Created on: Jan 3, 2012
 *      Author: sa
 */

#ifndef HTTPTASK_H_
#define HTTPTASK_H_

#include "../tools/stdext.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "HttpServer.h"
class HttpTask: public FileTask
{
protected:
    CHttpServerRequest request;
    LPCBUFFER writeoutbuffer;
public:
    HttpTask ( int fd,CHttpHost* host );
    virtual ~HttpTask();
    int datainput ( LPCBUFFER data );
    int fail ( int code );
    virtual void ProcessRequest();
    void WriteOut ( const char* data,int len );
    void WriteOutFlush();
};

#endif /* HTTPTASK_H_ */
