/*
 * ConfigReader.h
 *
 *  Created on: May 21, 2012
 *      Author: amen
 */
#include <map>
#include <string>
#include <boost/regex.hpp>
#include "../tools/stdext.h"
#ifndef CONFIGREADER_H_
#define CONFIGREADER_H_

class ConfigReader:public std::map<std::string,std::string,string_less_nocase>
{
public:
    void LoadFile ( std::string path );
};

#endif /* CONFIGREADER_H_ */
