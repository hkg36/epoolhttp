#include "geofunction.h"

unsigned long long GeoFun::Combine ( unsigned int first,unsigned second )
{
    unsigned long long result=0;
    for ( int i=0; i<sizeof ( unsigned int ) *8; i++ ) {
        unsigned long long mid=first& ( 0x1<<i );
        result|=mid<<i;
    }
    for ( int i=0; i<sizeof ( unsigned int ) *8; i++ ) {
        unsigned long long mid=second& ( 0x1<<i );
        result|=mid<< ( i+1 );
    }
    return result;
}
void GeoFun::Split ( unsigned long long src,unsigned int* first,unsigned int* second )
{
    unsigned int resfirst=0;
    unsigned int ressecond=0;
    for ( int i=0; i<sizeof ( unsigned int ) *8; i++ ) {
        unsigned long long mid=src& ( 0x1ll<< ( i*2 ) );
        resfirst|= ( unsigned int ) ( mid>>i );
    }
    for ( int i=0; i<sizeof ( unsigned int ) *8; i++ ) {
        unsigned long long mid=src& ( 0x1ll<< ( i*2+1 ) );
        ressecond|= ( unsigned int ) ( mid>> ( i+1 ) );
    }
    if ( first ) {
        *first=resfirst;
    }
    if ( second ) {
        *second=ressecond;
    }
}
void GeoFun::LongToByte ( unsigned long long src,unsigned char des[8] )
{
    for ( int i=0; i<8; i++ ) {
        des[i]= ( src>> ( ( 7-i ) *8 ) ) &0xFF;
    }
}
unsigned long long GeoFun::ByteToLong ( unsigned char src[8] )
{
    unsigned long long res=0;
    for ( int i=0; i<8; i++ ) {
        res|= ( ( unsigned long long ) src[i] ) << ( ( 7-i ) *8 );
    }
    return res;
}
