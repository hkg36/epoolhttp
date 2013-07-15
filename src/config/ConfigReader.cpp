/*
 * ConfigReader.cpp
 *
 *  Created on: May 21, 2012
 *      Author: amen
 */
#include <stdio.h>
#include "ConfigReader.h"
void ConfigReader::LoadFile ( std::string path )
{
    this->clear();
    boost::regex reg ( "\\s*(?<name>[^\\s=]*)\\s*=\\s*(?<value>[^\\s]*)\\s*" );
    FILE* f=fopen ( path.c_str(),"r" );
    if ( f ) {
        char buffer[256];
        while ( true ) {
            memset ( buffer,0,sizeof ( buffer ) );
            if ( NULL==fgets ( buffer,sizeof ( buffer ),f ) ) {
                if ( feof ( f ) ) {
                    break;
                }
                int ferr=ferror ( f );
                printf ( "file read error:%s(%d)",path.c_str(),ferr );
                break;
            }
            std::string linestr ( buffer );
            boost::smatch match;
            if ( boost::regex_match ( linestr,match,reg ) ) {
                std::string name=match["name"].str();
                std::string value=match["value"].str();
                ( *this ) [name]=value;
                printf ( "read config<%s : %s>\n",name.c_str(),value.c_str() );
            }
        }
        fclose ( f );
    } else {
        printf ( "file open error:%s\n",path.c_str() );
    }
}
