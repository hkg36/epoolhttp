#include "../src/tools/CBuffer.h"

int main() {
  LPCBUFFER buffer=CBuffer::getBuffer(1024);
  for(int i=0;i<1024;i++)
  {
    *(buffer->Buffer()+i)=i;
  }
  buffer=NULL;
  return 0;
}