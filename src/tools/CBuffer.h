/*
 * CBuffer.h
 *
 *  Created on: Jan 7, 2012
 *      Author: sa
 */
#include "IPtr.h"
#ifndef CBUFFER_H_
#define CBUFFER_H_

class CBuffer;
typedef CIPtr<CBuffer> LPCBUFFER;

class CBuffer:public IPtrBase<CBuffer> {
private:
	unsigned long long len;
	CBuffer();
public:
	unsigned int datalen;
	int exdata;
private:
	unsigned char buffer[0];
public:
	static void operator delete( void * arg );
	inline unsigned char* Buffer(){return buffer;};
	inline unsigned int BufLen(){return len;}
	static LPCBUFFER getBuffer(unsigned int size);
};


#endif /* CBUFFER_H_ */
