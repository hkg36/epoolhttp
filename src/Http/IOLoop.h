#include <sys/epoll.h>
#include <map>
#include <vector>
#ifndef _IOLOOP_H
#define _IOLOOP_H
class IOLoop
{
public:
  typedef int (*io_callback)(IOLoop* looper,long userdata,int fd,int event);
private:
  struct ioinfo
  {
    long userdata;
    io_callback callback;
  };
  int epfd;
  std::map<int,ioinfo> fdmap;
  std::vector<ioinfo> idelcallback;
public:
  static const int EVENT_READ=EPOLLOUT;
  static const int EVENT_WRITE=EPOLLIN;
  IOLoop();
  ~IOLoop();
  int AddFD(int fd,io_callback callback,int events,long userdata=0);
  int DelFD(int fd);
  int AddIdelCallback(io_callback callback,long userdata=0);
  int DelIdelCallback(io_callback callback);
  int run_once();
};
#endif