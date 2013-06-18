/*
 * ByteStream.cpp
 *
 *  Created on: Jan 14, 2012
 *      Author: sa
 */

#include "ByteStream.h"
#include <stdlib.h>
#include "stdext.h"
#include <sys/param.h>
const int uproundbase = 512;
ByteStream::ByteStream() :
		bytes(NULL), nowsize(0), nowpos(0) {

}

ByteStream::~ByteStream() {
	if (bytes)
		free(bytes);
}

size_t ByteStream::Read(void* pv, size_t cb) {
	size_t toread = (size_t) MIN(nowsize - nowpos, cb);
	if (toread == 0) {
		return 0;
	}
	memcpy(pv, bytes + nowpos, toread);
	nowpos += toread;
	return toread;
}
size_t ByteStream::Write(void const* pv, size_t cb) {
	if (nowsize - nowpos < cb) {
		int newsize = nowpos + cb;
		char *newbytes = (char*) realloc(bytes, UPROUND(newsize,uproundbase));
		if (newbytes != NULL) {
			bytes = newbytes;
			nowsize = newsize;
		} else {
			return 0;
		}
	}
	memcpy(bytes + nowpos, pv, cb);
	nowpos += cb;
	return cb;
}
bool ByteStream::SetSize(size_t size) {
	if (size == 0) {
		nowsize = 0;
		nowpos = 0;
		if (bytes) {
			free(bytes);
			bytes = NULL;
		}
		return true;
	}
	size_t newsize = size;
	char* newbytes = (char*) realloc(bytes, UPROUND(newsize,uproundbase));
	if (newbytes) {
		bytes = newbytes;
		nowsize = newsize;
		nowpos = MIN(nowsize - 1, nowpos);
		return true;
	}
	return false;
}
ByteStream::LPByteStream ByteStream::CreateInstanse()
{
	ByteStream::LPByteStream stream=new ByteStream();
	return stream;
}
