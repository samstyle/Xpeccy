#ifndef _XPCOMMON
#define _XPCOMMON

#include <stdint.h>
#include <string>
#include <vector>

void shitHappens(const char*);
bool areSure(const char*);
void showInfo(const char*);

void setFlagBit(bool, int32_t*, int32_t);
// std::string int2str(int);
bool str2bool(std::string);
// std::string getTimeString(int32_t);

std::vector<std::string> splitstr(std::string,const char*);
std::pair<std::string,std::string> splitline(std::string);

#endif
