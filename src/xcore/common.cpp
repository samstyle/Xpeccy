#include <string>
#include <sstream>

std::string int2str(int num) {
	std::stringstream str;
	str<<num;
	return str.str();
}

std::string getTimeString(int tsec) {
	int tmin = tsec / 60;
	tsec -= tmin * 60;
	std::string res = int2str(tmin);
	res += ":";
	if (tsec < 10) res += "0";
	res += int2str(tsec);
	return res;
}
