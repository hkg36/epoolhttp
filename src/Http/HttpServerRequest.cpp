/*
 * HttpServerRequest.cpp
 *
 *  Created on: Jan 14, 2012
 *      Author: sa
 */

#include "HttpServerRequest.h"
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/param.h>
std::string CHttpServerRequest::HttpDecodeUri ( std::string& srcuri )
{
    std::string res;
    const char* point = srcuri.c_str();
    while ( *point ) {
        if ( *point != '%' ) {
            res.push_back ( *point );
            point++;
        } else {
            unsigned int word;
            if ( 1 == sscanf ( point, "%%%02X", &word ) ) {
                res.push_back ( word );
            }
            point += 3;
        }
    }
    return res;
}
CHttpServerRequest::CHttpServerRequest ( void ) :
    resault ( false )
{
    bodystream = ByteStream::CreateInstanse();
    Init();
}
void CHttpServerRequest::Init()
{
    resault = false;
    state = RecvFirstLine;
    act.clear();
    uri.clear();
    vision.clear();
    headmap.clear();
    headlines.clear();
    inputbuffer.clear();
    bodystream->SetSize ( 0 );
}
std::string CHttpServerRequest::GetHead ( const std::string& key ) const
{
    HeadMap::const_iterator i = headmap.find ( key );
    if ( i != headmap.end() ) {
        return i->second;
    }
    return std::string();
}
bool CHttpServerRequest::getContentLength ( int& value ) const
{
    std::string res = GetHead ( "Content-Length" );
    if ( res.empty() ) {
        return false;
    } else {
        value = atoi ( res.c_str() );
        return true;
    }
}
std::string trim ( std::string& line,const char* chars=" " )
{
    if ( line.empty() ) {
        return "";
    }

    int string_size = ( int ) ( line.length() );
    int beginning_of_string = 0;

    // the minus 1 is needed to start at the first character
    // and skip the string delimiter
    int end_of_string = string_size - 1;

    bool encountered_characters = false;

    // find the start of chracters in the string
    while ( ( beginning_of_string < string_size ) && ( !encountered_characters ) ) {
        // if a space or tab was found then ignore it
        if ( strchr ( chars,line[beginning_of_string] ) ==NULL ) {
            encountered_characters = true;
        } else {
            beginning_of_string++;
        }
    }

    // test if no characters were found in the string
    if ( beginning_of_string == string_size ) {
        return "";
    }

    encountered_characters = false;

    // find the character in the string
    while ( ( end_of_string > beginning_of_string ) && ( !encountered_characters ) ) {
        // if a space or tab was found then ignore it
        if ( strchr ( chars,line[end_of_string] ) ==NULL ) {
            encountered_characters = true;
        } else {
            end_of_string--;
        }
    }
    // return the original string with all whitespace removed from its beginning and end
    // + 1 at the end to add the space for the string delimiter
    return line.substr ( beginning_of_string,
                         end_of_string - beginning_of_string + 1 );
}
bool CHttpServerRequest::InputBuffer ( char* buf,size_t size,size_t& proced )
{
    bool res;
    char* procbuf = buf;
    proced = 0;
    if ( state != RecvBody ) {
        while ( proced < size ) {
            proced++;
            if ( ! ( res = InputChar ( *procbuf ) ) ) {
                return false;
            }
            procbuf++;
            if ( state == RecvBody ) {
                break;
            }
        }
    }
    if ( state == RecvBody ) {
        size_t wrote=0;
        while ( true ) {
            size_t towrite = MIN ( ( size_t ) ( size-proced ), ( size_t ) torecvbody );
            if ( towrite == 0 ) {
                return true;
            }
            wrote=bodystream->Write ( procbuf, towrite );
            if ( wrote == 0 ) {
                return false;
            }
            procbuf += wrote;
            proced += wrote;
            torecvbody -= wrote;
            if ( torecvbody == 0 ) {
                resault = true;
                return false;
            }
        }
    }
    return true;
}
bool CHttpServerRequest::InputChar ( char one )
{
    switch ( state ) {
    case RecvFirstLine: {
        if ( one != '\n' ) {
            if ( inputbuffer.length() > 1024 ) {
                return false;
            }
            inputbuffer.push_back ( one );
            return true;
        }
        inputbuffer=trim ( inputbuffer,"\r\n " );
        if ( !inputbuffer.empty() ) {
            headlines.push_back ( inputbuffer );
            inputbuffer.clear();
            return true;
        } else {
            if ( headlines.size() < 2 ) {
                return false;
            }
            inputbuffer = headlines[0];

            size_t index = inputbuffer.find ( ' ' );
            if ( index ==std::string::npos ) {
                return false;
            }
            act = inputbuffer.substr ( 0, index );
            int indexold = index + 1;
            index = inputbuffer.find ( ' ', indexold );
            if ( index == std::string::npos ) {
                return false;
            }
            uri = inputbuffer.substr ( indexold, index - indexold );
            if ( uri.empty() ) {
                return false;
            }
            uri = HttpDecodeUri ( uri );
            vision = inputbuffer.substr ( index + 1 );

            for ( size_t i = 1; i < headlines.size(); i++ ) {
                inputbuffer = headlines[i];
                index = inputbuffer.find ( ':' );
                if ( index == std::string::npos ) {
                    return false;
                }
                std::string first, second;
                first = inputbuffer.substr ( 0, index );
                second = inputbuffer.substr ( index + 1 );
                headmap[trim ( first )] = trim ( second );
            }
            if ( strcasecmp ( act.c_str(),"GET" ) == 0 ) {
                resault = true;
                return false;
            } else if ( strcasecmp ( act.c_str(),"POST" ) == 0 ) {
                if ( getContentLength ( torecvbody ) ) {
                    if ( torecvbody ) {
                        state = RecvBody;
                        return true;
                    } else {
                        resault = true;
                        return false;
                    }
                } else {
                    resault = true;
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    break;
    }
    return true;
}
const std::string UriSplit::GetParam ( const std::string name ) const
{
    Params::const_iterator i=params.find ( name );
    if ( i==params.end() ) {
        return std::string();
    }
    return i->second;
}
bool UriSplit::ParamExist ( const std::string name ) const
{
    Params::const_iterator i=params.find ( name );
    return i!=params.end();
}
bool UriSplit::Decode ( const char* str )
{
    path.clear();
    params.clear();
    fragment.clear();

    if ( *str!='/' ) {
        return false;
    }
    str++;
    std::string temp;
    while ( true ) {
        if ( *str==0 ) {
            path.push_back ( temp );
            temp.clear();
            return true;
        } else if ( *str=='?' ) {
            path.push_back ( temp );
            temp.clear();
            break;
        } else if ( *str!='/' ) {
            temp.push_back ( *str );
        } else {
            path.push_back ( temp );
            temp.clear();
        }
        str++;
    }
    str++;
    std::string value;
    enum {ReadName,ReadValue};
    int stat=ReadName;
    while ( true ) {
        if ( *str==0 ) {
            if ( stat==ReadName ) {
                return false;
            } else {
                params[temp]=value;
                return true;
            }
        }
        if ( stat==ReadName ) {
            if ( *str=='=' ) {
                stat=ReadValue;
            } else {
                temp.push_back ( *str );
            }
        } else if ( stat==ReadValue ) {
            if ( *str=='&' || *str=='#' ) {
                params[temp]=value;
                temp.clear();
                value.clear();
                if ( *str=='#' ) {
                    str++;
                    break;
                }
                stat=ReadName;
            } else {
                if ( *str=='%' ) {
                    unsigned char tranc=0;
                    str++;
                    {
                        switch ( *str ) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            tranc|=*str-'0';
                            break;
                        case 'a':
                        case 'A':
                            tranc|=0xA;
                            break;
                        case 'b':
                        case 'B':
                            tranc|=0xB;
                            break;
                        case 'c':
                        case 'C':
                            tranc|=0xC;
                            break;
                        case 'd':
                        case 'D':
                            tranc|=0xD;
                            break;
                        case 'e':
                        case 'E':
                            tranc|=0xE;
                            break;
                        case 'f':
                        case 'F':
                            tranc|=0xF;
                            break;
                        default:
                            return false;
                        }
                        tranc=tranc<<4;
                        str++;
                        switch ( *str ) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            tranc|=*str-'0';
                            break;
                        case 'a':
                        case 'A':
                            tranc|=0xA;
                            break;
                        case 'b':
                        case 'B':
                            tranc|=0xB;
                            break;
                        case 'c':
                        case 'C':
                            tranc|=0xC;
                            break;
                        case 'd':
                        case 'D':
                            tranc|=0xD;
                            break;
                        case 'e':
                        case 'E':
                            tranc|=0xE;
                            break;
                        case 'f':
                        case 'F':
                            tranc|=0xF;
                            break;
                        default:
                            return false;
                        }
                    }
                    value.push_back ( tranc );
                } else {
                    value.push_back ( *str );
                }
            }
        }
        str++;
    }
    while ( *str!=0 ) {
        fragment.push_back ( *str );
        str++;
    }
    return true;
}

using namespace std;
void CHttpServerRequest::DebugPrint()
{
    cout<<"CHttpServerRequest:"<<endl;
    cout<<act<<" "<<uri<<" "<<vision<<endl;
    for ( HeadMap::const_iterator i=headmap.begin(); i!=headmap.end(); i++ ) {
        cout<<i->first<<" : "<<i->second<<endl;
    }
    cout<<"with data size:"<<bodystream->GetBufferSize() <<endl;
}

