/*
 * CBuffer.cpp
 *
 *  Created on: Jan 7, 2012
 *      Author: sa
 */

#include "CBuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

CBuffer::CBuffer()
{
}

LPCBUFFER CBuffer::getBuffer ( unsigned int size )
{
    int alloc_size=sizeof ( CBuffer ) +size;
    char* classbuf= ( char* ) malloc ( alloc_size );
    bzero ( classbuf,sizeof ( CBuffer ) );
    ( ( CBuffer* ) classbuf )->len=size;
    return ( CBuffer* ) classbuf;
}

void CBuffer::operator delete ( void* arg )
{
    free ( arg );
}
