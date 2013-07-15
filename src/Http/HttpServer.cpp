#include "HttpServer.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
CHttpHost::CHttpHost ( unsigned short port ) :
    port ( port ),
    running ( false ),
    CreateFileTaskFunction ( NULL ),
    epfd ( 0 )
{
}

void CHttpHost::Stop ()
{
    running = false;
}

void CHttpHost::Run ()
{
    assert ( CreateFileTaskFunction );
    int res = 0;
    epoll_event ev, events[20];
    epfd = epoll_create ( 256 );

    struct sockaddr_in serveraddr;
    bzero ( &serveraddr, sizeof ( serveraddr ) );
    serveraddr.sin_family = AF_INET;
    res = inet_pton ( serveraddr.sin_family, "localhost", &serveraddr.sin_addr );
    serveraddr.sin_port = htons ( port );

    int listenfd = socket ( serveraddr.sin_family, SOCK_STREAM, 0 );
    setnonblock ( listenfd );
    int opt = 1;
    setsockopt ( listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof ( opt ) );
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;
    res = epoll_ctl ( epfd, EPOLL_CTL_ADD, listenfd, &ev );
    res = bind ( listenfd, ( sockaddr* ) & serveraddr, sizeof ( serveraddr ) );
    if ( res == -1 ) {
        printf ( "bind error %s\n", strerror ( errno ) );
        return;
    }
    res = listen ( listenfd, 20 );

    const int workThreadCount = 5;
    pthread_t workid[workThreadCount] = {
        0
    };
    pthread_t writeWorkID = 0;
    pthread_attr_t attr;

    printf ( "work start\n" );
    running = true;
    pthread_attr_init ( &attr );
    for ( int i = 0; i < workThreadCount; i++ ) {
        pthread_create ( workid + i, &attr, _processthread, this );
    }
    pthread_create ( &writeWorkID, &attr, _writeSocketThread, this );

    int flgs = fcntl ( 0, F_GETFL );
    fcntl ( 0, F_SETFL, flgs | O_NONBLOCK );
    while ( running ) {
        int epollres = epoll_wait ( epfd, events, 20, -1 );
        if ( epollres == -1 ) {
            int count = 0;
            writequeuemaplock.Lock ();
            for ( std::map < int, LPWriteQueue >::iterator i =
                        writequeuemap.begin (); i != writequeuemap.end (); i++ ) {
                count += i->second->size ();
            }
            writequeuemaplock.Unlock ();
            printf ( "write queue left:%d\n", count );

            count = 0;
            readdataqueuelock.Lock ();
            for ( std::list < LPCBUFFER >::iterator i =
                        readdataqueue.begin (); i != readdataqueue.end (); i++ ) {
                count++;
            }
            readdataqueuelock.Unlock ();
            printf ( "read queue left:%d\n", count );
            continue;
        }
        if ( epollres == 0 ) {

        } else {
            bool WriteEnable = false;
            for ( int i = 0; i < epollres; i++ ) {
                epoll_event nowev = events[i];
                if ( nowev.events & EPOLLIN ) {
                    if ( nowev.data.fd == listenfd ) {
                        for ( ;; ) {
                            struct sockaddr clientaddr;
                            socklen_t caddrlen = sizeof ( clientaddr );
                            int connfd = accept ( listenfd, &clientaddr,
                                                  &caddrlen );
                            if ( connfd == -1 ) {
                                if ( errno != EAGAIN )
                                    printf ( "accept error %s\n",
                                             strerror ( errno ) );
                                break;
                            } else {
                                setnonblock ( connfd );
                                ev.data.fd = connfd;
                                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                                res =
                                    epoll_ctl ( epfd, EPOLL_CTL_ADD, connfd, &ev );
                                if ( res == 0 ) {
                                    CIPtr < SocketState > state =
                                        new SocketState ();
                                    sokectstatesMutex.Lock ();
                                    socketstates.insert
                                    ( SocketStates::value_type
                                      ( connfd, state ) );
                                    sokectstatesMutex.Unlock ();

                                    writequeuemaplock.Lock ();
                                    LPWriteQueue writequeue = new WriteQueue ();
                                    writequeuemap.insert
                                    ( WriteQueueMap::value_type
                                      ( connfd, writequeue ) );
                                    writequeuemaplock.Unlock ();

                                } else {
                                    closesocket ( connfd );
                                }
                            }
                        }
                    } else {
                        CIPtr < SocketState > state;
                        sokectstatesMutex.Lock ();
                        SocketStates::iterator nowsocketstate =
                            socketstates.find ( nowev.data.fd );
                        if ( nowsocketstate != socketstates.end () ) {
                            state = nowsocketstate->second;
                        }
                        sokectstatesMutex.Unlock ();
                        bool readsome = false;
                        while ( true ) {
                            LPCBUFFER buffer = CBuffer::getBuffer ( 1024 );
                            int res = read ( nowev.data.fd, buffer->Buffer (),
                                             buffer->BufLen () );
                            if ( res == -1 && errno == EAGAIN ) {
                                break;
                            } else if ( res == 0 ) {
                                closesocket ( nowev.data.fd );
                                nowev.data.fd = -1;
                                break;
                            }
                            if ( state ) {
                                state->readcount += res;
                            }
                            buffer->datalen = res;
                            buffer->exdata = nowev.data.fd;

                            readdataqueuelock.Lock ();
                            readdataqueue.push_back ( buffer );
                            readdataqueuelock.Unlock ();
                            readsome = true;
                        }
                        if ( readsome ) {
                            readqueueWait.Signal ();
                        }
                        if ( state ) {
                            state->lastread = time ( NULL );
                        }
                    }
                }
                if ( nowev.events & EPOLLOUT ) {
                    sokectstatesMutex.Lock ();
                    SocketStates::iterator nowsocketstate =
                        socketstates.find ( nowev.data.fd );
                    if ( nowsocketstate != socketstates.end () ) {
                        nowsocketstate->second->canwrite = true;
                        WriteEnable = true;
                    }
                    sokectstatesMutex.Unlock ();
                }
                if ( nowev.events & EPOLLPRI ) {
                }
                if ( nowev.events & EPOLLERR ) {
                    if ( nowev.data.fd != listenfd ) {
                        closesocket ( nowev.data.fd );
                    }
                }
                if ( nowev.events & EPOLLHUP ) {
                    // hand up
                    closesocket ( nowev.data.fd );
                }
            }
            if ( WriteEnable ) {
                SocketWriteWait.Signal ();
            }
        }
    }

    if ( listenfd != -1 ) {
        close ( listenfd );
    }
    readqueueWait.Broadcast ();
    for ( int i = 0; i < workThreadCount; i++ ) {
        pthread_join ( workid[i], NULL );
    }
    SocketWriteWait.Signal ();
    pthread_join ( writeWorkID, NULL );
    running = false;
}

void* CHttpHost::_writeSocketThread ( void* obj )
{
    CHttpHost* server = ( CHttpHost* ) obj;
    server->writeSocketThread ();
    return NULL;
}

void CHttpHost::writeSocketThread ()
{
    timespec waittime;
    while ( running ) {
        struct timeval now;
        gettimeofday ( &now, NULL );
        waittime.tv_sec = now.tv_sec;
        waittime.tv_nsec = ( now.tv_usec + 100 ) * 1000;
        writequeuemaplock.Lock ();
        int res = SocketWriteWait.TimedWait ( &writequeuemaplock, &waittime );
        std::set < int >fddead;
        sokectstatesMutex.Lock ();
        for ( SocketStates::iterator i = socketstates.begin (); i
                != socketstates.end (); i++ ) {
            if ( i->second->canwrite ) {
                LPWriteQueue writequeue;
                WriteQueueMap::iterator pos = writequeuemap.find ( i->first );
                if ( pos != writequeuemap.end () ) {
                    writequeue = pos->second;
                }
                if ( writequeue ) {
                    while ( writequeue->size () > 0 ) {
                        LPCBUFFER nowbuf = writequeue->front ();
                        if ( writequeue->lastPos >= nowbuf->datalen ) {
                            writequeue->pop ();
                            writequeue->lastPos = 0;
                        } else {
                            int towrite = nowbuf->datalen - writequeue->lastPos;
                            int writeres = write ( i->first,
                                                   nowbuf->Buffer () +
                                                   writequeue->lastPos,
                                                   towrite );
                            if ( writeres == -1 ) {
                                if ( errno != EAGAIN ) {
                                    fddead.insert ( i->first );
                                    i->second->canwrite = false;
                                }
                                break;
                            } else {
                                writequeue->lastPos += writeres;
                            }
                        }
                    }
                    if ( writequeue->closeAfterSendAll && writequeue->empty () ) {
                        fddead.insert ( i->first );
                    }
                }
            }
        }
        sokectstatesMutex.Unlock ();
        writequeuemaplock.Unlock ();

        for ( std::set < int >::iterator i = fddead.begin ();
                i != fddead.end (); i++ ) {
            closesocket ( *i );
        }
    }
}

void* CHttpHost::_processthread ( void* obj )
{
    CHttpHost* server = ( CHttpHost* ) obj;
    server->processthread ();
    return NULL;
}

void CHttpHost::processthread ()
{
    while ( running ) {
        LPCBUFFER buffer;
        {
            AutoMyMutexLock al1 ( readdataqueuelock );
            if ( readdataqueue.empty () ) {
                readqueueWait.Wait ( &readdataqueuelock );
            }
            if ( readdataqueue.empty () ) {
                continue;
            }
            AutoMyMutexLock al2 ( procFileLock );
            std::list < LPCBUFFER >::iterator walker = readdataqueue.begin ();
            while ( walker != readdataqueue.end () ) {
                if ( processingFile.end () ==
                        processingFile.find ( ( *walker )->exdata ) ) {
                    buffer = *walker;
                    readdataqueue.erase ( walker );
                    break;
                }
                walker++;
            }
        }
        if ( buffer == NULL ) {
            continue;
        }
        CIPtr < FileTask > fdtask;
        {
            AutoMyMutexLock lock ( taskpollLock );
            std::map < int,
                CIPtr <
                FileTask > >::iterator walker = taskpoll.find ( buffer->exdata );
            if ( walker != taskpoll.end () ) {
                fdtask = walker->second;
            } else {
                fdtask = CreateFileTaskFunction ( buffer->exdata, this );
                taskpoll.insert ( std::map < int,
                                  CIPtr < FileTask >
                                  >::value_type ( buffer->exdata, fdtask ) );
            }
            AutoMyMutexLock lock2 ( procFileLock );
            processingFile.insert ( buffer->exdata );
        }
        fdtask->datainput ( buffer );
        {
            AutoMyMutexLock lock2 ( procFileLock );
            processingFile.erase ( buffer->exdata );
        }
    }
}

void CHttpHost::setnonblock ( int fd )
{
    int opts;
    opts = fcntl ( fd, F_GETFL );
    if ( opts < 0 ) {
        printf ( "fcntl error %s\n", strerror ( errno ) );
        return;
    }
    opts = opts | O_NONBLOCK;
    if ( fcntl ( fd, F_SETFL, opts ) < 0 ) {
        printf ( "fcntl error %s\n", strerror ( errno ) );
    }
}

void CHttpHost::closesocket ( int fd )
{
    if ( fd == -1 ) {
        return;
    }

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = 0;
    epoll_ctl ( epfd, EPOLL_CTL_DEL, fd, &ev );
    sokectstatesMutex.Lock ();
    socketstates.erase ( fd );
    sokectstatesMutex.Unlock ();

    readdataqueuelock.Lock ();
    for ( std::list < LPCBUFFER >::iterator i = readdataqueue.begin (); i
            != readdataqueue.end (); ) {
        if ( ( *i )->exdata == fd ) {
            i = readdataqueue.erase ( i );
        } else {
            i++;
        }
    }
    readdataqueuelock.Unlock ();

    taskpollLock.Lock ();
    taskpoll.erase ( fd );
    taskpollLock.Unlock ();

    procFileLock.Lock ();
    processingFile.erase ( fd );
    procFileLock.Unlock ();

    writequeuemaplock.Lock ();
    writequeuemap.erase ( fd );
    writequeuemaplock.Unlock ();

    close ( fd );
}

int CHttpHost::AddWriteQueue ( int fd, LPCBUFFER buffer )
{
    LPWriteQueue writequeue;
    {
        AutoMyMutexLock al ( writequeuemaplock );
        WriteQueueMap::iterator pos = writequeuemap.find ( fd );
        if ( pos == writequeuemap.end () ) {
            return WriteQueueNotFound;
        }
        writequeue = pos->second;
        writequeue->push ( buffer );
    }
    SocketWriteWait.Signal ();
    return WriteQueueAdded;
}

void CHttpHost::SetCloseAfterSendAll ( int fd )
{
    AutoMyMutexLock al ( writequeuemaplock );
    WriteQueueMap::iterator pos = writequeuemap.find ( fd );
    if ( pos == writequeuemap.end () ) {
        return;
    }
    pos->second->closeAfterSendAll = true;
}
