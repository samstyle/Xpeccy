#ifndef _XPCOMMON
#define _XPCOMMON

#include <stdint.h>
#include <string>
#include <vector>

void shithappens(std::string);
bool areSure(std::string);
void filltabs();

void setFlagBit(bool, int32_t*, int32_t);
std::string int2str(int);
bool str2bool(std::string);
std::string getTimeString(int32_t);

std::vector<std::string> splitstr(std::string,const char*);
void splitline(std::string, std::string*, std::string*);

#endif
