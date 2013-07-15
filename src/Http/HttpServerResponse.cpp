/*
 * CHttpServerResponse.cpp
 *
 *  Created on: Feb 4, 2012
 *      Author: sa
 */

#include "HttpServerResponse.h"
#include <stdio.h>

void CHttpServerResponse::AddHead ( const std::string& key,
                                    const std::string& value )
{
    headmap[key] = value;
}

std::string CHttpServerResponse::SaveHead()
{
    std::string savedhead;
    if ( vision.empty() || code == 0 ) {
        return savedhead;
    }
    char formatbuf[128];
    sprintf ( formatbuf, "%s %d %s\r\n", vision.c_str(), code, message.c_str() );
    savedhead.append ( formatbuf );
    for ( HeadMap::const_iterator i = headmap.begin(); i != headmap.end(); i++ ) {
        sprintf ( formatbuf, "%s:%s\r\n", i->first.c_str(), i->second.c_str() );
        savedhead.append ( formatbuf );
    }
    savedhead.append ( "\r\n" );
    return savedhead;
}

void CHttpServerResponse::Init()
{
    vision.clear();
    code = 0;
    message.clear();
    headmap.clear();
}
void CHttpServerResponse::Message ( int code )
{
    this->code = code;
    message = GetHttpCodeMessage ( code );
}
void CHttpServerResponse::Message ( int code, const std::string& msgstr )
{
    this->code = code;
    message = msgstr;
}

struct HttpResCode {
    unsigned int code;
    const char* msg;
};
HttpResCode rescode[] = { { 100, "Continue" }, { 101, "Switching Protocols" }, {
        200, "OK"
    }, { 201, "Created" }, { 202, "Accepted" }, {
        203,
        "Non-Authoritative Information"
    }, { 204, "No Content" }, {
        205,
        "Reset Content"
    }, { 206, "Partial Content" },
    { 300, "Multiple Choices" }, { 301, "Moved Permanently" }, {
        302,
        "Found"
    }, { 303, "See Other" }, { 304, "Not Modified" }, {
        305,
        "Use Proxy"
    }, { 307, "Temporary Redirect" }, {
        400,
        "Bad Request"
    }, { 401, "Unauthorized" }, {
        402,
        "Payment Required"
    }, { 403, "Forbidden" },
    { 404, "Not Found" }, { 405, "Method Not Allowed" }, {
        406,
        "Not Acceptable"
    }, { 407, "Proxy Authentication Required" }, {
        408, "Request Time-out"
    }, { 409, "Conflict" }, { 410, "Gone" },
    { 411, "Length Required" }, { 412, "Precondition Failed" }, {
        413,
        "Request Entity Too Large"
    }, { 414, "Request-URI Too Large" },
    { 415, "Unsupported Media Type" }, {
        416,
        "Requested range not satisfiable"
    },
    { 417, "Expectation Failed" }, { 500, "Internal Server Error" }, {
        501,
        "Not Implemented"
    }, { 502, "Bad Gateway" }, {
        503,
        "Service Unavailable"
    }, { 504, "Gateway Time-out" }, {
        505,
        "HTTP Version not supported"
    }
};
struct HttpCodeLess {
    bool operator() ( const HttpResCode& a, const HttpResCode& b ) const {
        return a.code < b.code;
    }
    bool operator() ( const unsigned int a, const HttpResCode& b ) const {
        return a < b.code;
    }
    bool operator() ( const HttpResCode& a, const unsigned int b ) const {
        return a.code < b;
    }
};

std::string CHttpServerResponse::GetHttpCodeMessage ( unsigned int code )
{
    const HttpResCode* a = std::lower_bound ( &rescode[0],
                           &rescode[_countof ( rescode )], code, HttpCodeLess() );
    if ( a != &rescode[_countof ( rescode )] && a->code == code ) {
        return a->msg;
    }
    return "";
}
void CHttpServerResponse::setContentLength ( unsigned long long v )
{
    char buf[32];
    sprintf ( buf, "%llu", v );
    headmap["Content-Length"] = buf;
}
void CHttpServerResponse::setContentType ( std::string& maintype,
        std::string& subtype )
{
    char buf[128];
    sprintf ( buf, "%s/%s", maintype.c_str(), subtype.c_str() );
    headmap["Content-Type"] = buf;
}
void CHttpServerResponse::setContentRange ( long long fullsize, long long start,
        long long end )
{
    char buf[128];
    if ( start < 0 && end < 0 && fullsize >= 0 ) {
        sprintf ( buf,"*/%lld", fullsize );
        headmap["Content-Range"] = buf;
    } else if ( start >= 0 && end >= 0 && fullsize >= 0 ) {
        sprintf ( buf,"%lld-%lld/%lld", start, end, fullsize );
        headmap["Content-Range"] = buf;
    } else {
        assert ( false );
    }
}
void CHttpServerResponse::setContentEncoding ( std::string& v )
{
    headmap["Content-Encoding"] = v;
}
void CHttpServerResponse::setByteAcceptRanges()
{
    headmap["Accept-Ranges"]="byte";
}
void CHttpServerResponse::setAcceptRanges ( std::string& v )
{
    headmap["Accept-Ranges"] = v;
}
void CHttpServerResponse::setTransferEncoding ( std::string& v )
{
    headmap["Transfer-Encoding"] = v;
}

