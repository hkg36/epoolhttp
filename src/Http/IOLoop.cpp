#include "IOLoop.h"
#include <unistd.h>
IOLoop& IOLoop::instanse()
{
  static IOLoop looper;
  return looper;
}
IOLoop::IOLoop()
{
  epfd = epoll_create ( 256 );
}
IOLoop::~IOLoop()
{
  close(epfd);
}

int IOLoop::AddFD(int fd,io_callback callback,int events,unsigned long userdata)
{
  if(fdmap.find(fd)!=fdmap.end())
    return 100;
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = events | EPOLLET;
  int res = epoll_ctl ( epfd, EPOLL_CTL_ADD, fd, &ev );
  if(res==0)
  {
    ioinfo info{userdata=userdata,callback=callback};
    fdmap.insert(std::map<int,ioinfo>::value_type(fd,info));
    return 0;
  }
  else
    return res;
}
int IOLoop::DelFD(int fd)
{
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = 0;
  epoll_ctl ( epfd, EPOLL_CTL_DEL, fd, &ev );
  fdmap.erase(fd);
}
int IOLoop::AddIdelCallback(io_callback callback,unsigned long userdata)
{
  for(size_t i=0;i<idelcallback.size();i++)
  {
    if(idelcallback[i].callback==callback)
      return 1;
  }
  ioinfo info;
  info.callback=callback;
  info.userdata=userdata;
  idelcallback.push_back(info);
  return 0;
}
int IOLoop::DelIdelCallback(io_callback callback)
{
  for(size_t i=0;i<idelcallback.size();i++)
  {
    if(idelcallback[i].callback==callback)
    {
      idelcallback.erase(idelcallback.begin()+i);
      return 0;
    }
  }
  return 1;
}
int IOLoop::run_once()
{
  epoll_event events[20];
  int epollres = epoll_wait ( epfd, events, 20, -1 );
  if ( epollres == -1 )
  {
    for(auto i=idelcallback.begin();i!=idelcallback.end();i++)
    {
      i->callback(i->userdata,0,0);
    }
  }
  else
  {
    for ( int i = 0; i < epollres; i++ ) {
      epoll_event nowev = events[i];
      std::map<int,ioinfo>::iterator pos= fdmap.find(nowev.data.fd);
      if(pos!=fdmap.end())
      {
	pos->second.callback(pos->second.userdata,nowev.data.fd,nowev.events);
      }
    }
  }
}