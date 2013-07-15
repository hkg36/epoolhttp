#include "../tools/stdext.h"
#ifndef CHTTPSERVERRESPONSE_H_
#define CHTTPSERVERRESPONSE_H_

class CHttpServerResponse
{
private:
    std::string vision;
    int code;
    std::string message;
    typedef std::map<std::string,std::string,string_less_nocase> HeadMap;
    HeadMap headmap;
public:
    void Vision ( const std::string& value ) {
        vision=value;
    }
    void Message ( int code );
    void Message ( int code,const std::string& msgstr );
    void AddHead ( const std::string& key,const std::string& value );
    void Init();
    std::string SaveHead();
    int Code() {
        return code;
    }

    void setContentLength ( unsigned long long v );
    void setContentType ( std::string& maintype,std::string& subtype );
    void setContentRange ( long long fullsize,long long start=-1,long long end=-1 );
    void setContentEncoding ( std::string& v );
    void setByteAcceptRanges();
    void setAcceptRanges ( std::string& v );
    void setTransferEncoding ( std::string& v );

    static std::string GetHttpCodeMessage ( unsigned int code );
};

#endif /* CHTTPSERVERRESPONSE_H_ */
