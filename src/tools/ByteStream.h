/*
 * ByteStream.h
 *
 *  Created on: Jan 14, 2012
 *      Author: sa
 */

#ifndef BYTESTREAM_H_
#define BYTESTREAM_H_
#include "IPtr.h"

class ByteStream: public IPtrBase<ByteStream>
{
protected:
    char* bytes;
    size_t nowsize;
    size_t nowpos;
protected:
    ByteStream();
public:
    ~ByteStream();
    size_t Read ( void* pv, size_t cb );
    size_t Write ( void const* pv, size_t cb );
    bool SetSize ( size_t sz );
    inline char* GetBuffer() {
        return bytes;
    }
    inline size_t GetBufferSize() {
        return nowsize;
    }
    typedef CIPtr<ByteStream> LPByteStream;
    static LPByteStream CreateInstanse();
};
#endif /* BYTESTREAM_H_ */
