/*
 * gtools.cpp
 *
 *  Created on: May 21, 2012
 *      Author: amen
 */

#include "gtools.h"
#include <stdio.h>
#include <unistd.h>
const std::string& SelfPath() {
	static std::string sp;
	if (sp.empty()) {
		char link[35], path[256];
		sprintf(link, "/proc/%d/exe", getpid());
		size_t len = readlink(link, path, sizeof(path));
		sp.append(path,len);
		len=sp.rfind('/');
		sp.erase(len+1,256);
	}
	return sp;
}
