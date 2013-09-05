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
    unsigned long userdata;
    io_callback callback;
  };
  int epfd;
  std::map<int,ioinfo> fdmap;
  std::vector<ioinfo> idelcallback;
public:
  IOLoop();
  ~IOLoop();
  int AddFD(int fd,io_callback callback,int events,unsigned long userdata=0);
  int DelFD(int fd);
  int AddIdelCallback(io_callback callback,unsigned long userdata=0);
  int DelIdelCallback(io_callback callback);
  int run_once();
};
#endif