/*
 * HttpServer.h
 *
 *  Created on: Apr 16, 2012
 *      Author: amen
 */
#include "../tools/stdext.h"
#include "IOLoop.h"
#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_
#define WriteQueueAdded 0
#define WriteQueueNotFound 1

class CHttpHost;
class FileTask: public IPtrBase<FileTask>
{
private:
    int fd;
    bool haserror;
    class CHttpHost* epoolhost;
public:
    CHttpHost* GetHost() {
        return epoolhost;
    }
    int getFD() {
        return fd;
    }
    FileTask ( int fd, CHttpHost* host ) :
        fd ( fd ), haserror ( false ), epoolhost ( host ) {

    }
    void setError() {
        haserror = true;
    }
    bool isError() {
        return haserror;
    }
    virtual int datainput ( LPCBUFFER data ) =0;
    virtual int fail ( int code ) =0;
    virtual ~FileTask() {
    }
};

class CHttpHost
{
public:
    CHttpHost ( unsigned short port );
    void Run();
    void Stop();
    int AddWriteQueue ( int fd, LPCBUFFER buffer );
    void SetCloseAfterSendAll ( int fd );
    void SetCreateFileTaskCallBack (
        FileTask * ( *FileTaskFun ) ( int fd, CHttpHost* host ) ) {
        CreateFileTaskFunction = FileTaskFun;
    }
private:
    unsigned short port;
    bool running;
    FileTask* ( *CreateFileTaskFunction ) ( int fd, CHttpHost* host );
    class WriteQueue: public IPtrBase<WriteQueue> ,
        public std::queue<LPCBUFFER>
    {
    public:
        bool closeAfterSendAll;
        unsigned int lastPos;
        WriteQueue() :closeAfterSendAll ( false ),
            lastPos ( 0 ) {
        }
    };
    class SocketState: public IPtrBase<SocketState>
    {
    public:
        bool canwrite;
        unsigned long long writecount;
        unsigned long long readcount;
        time_t lastread;
        time_t lastwrite;
        SocketState() :
            canwrite ( false ) {
        }
    };
    typedef CIPtr<WriteQueue> LPWriteQueue;
    typedef std::map<int, LPWriteQueue> WriteQueueMap;

    MyCond SocketWriteWait;

    WriteQueueMap writequeuemap;
    MyMutex writequeuemaplock;
    std::list<LPCBUFFER> readdataqueue;
    MyMutex readdataqueuelock;
    MyCond readqueueWait;

    std::map<int, CIPtr<FileTask> > taskpoll;
    MyMutex taskpollLock;
    std::set<int> processingFile;
    MyMutex procFileLock;

    IOLoop ioloop;

    typedef std::map<int, CIPtr<SocketState> > SocketStates;
    SocketStates socketstates;
    MyMutex sokectstatesMutex;

    static void* _writeSocketThread ( void* obj );
    void writeSocketThread();
    static void* _processthread ( void* obj );
    void processthread();
    void setnonblock ( int fd );

    void closesocket ( int fd );
public:
    void processListener(IOLoop* looper,int fd);
    void processClient(IOLoop* looper,int fd,int events);
    void idelProcess();
};
#endif /* HTTPSERVER_H_ */
