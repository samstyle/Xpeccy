#include <string>
#include <vector>
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

void setFlagBit(bool cond, int* val, int mask) {
	if (cond) {
		*val |= mask;
	} else {
		*val &= ~mask;
	}
}

bool str2bool(std::string v) {
	return !(v=="n" || v=="N" || v=="0" || v=="no" || v=="NO" || v=="false" || v=="FALSE");
}

std::vector<std::string> splitstr(std::string str,const char* spl) {
	size_t pos;
	std::vector<std::string> res;
	pos = str.find_first_of(spl);
	while (pos != std::string::npos) {
		res.push_back(str.substr(0,pos));
		str = str.substr(pos+1);
		pos = str.find_first_of(spl);
	}
	res.push_back(str);
	return res;

}

std::pair<std::string,std::string> splitline(std::string line) {
	size_t pos;
	std::pair<std::string,std::string> res;
	do {pos = line.find("\r"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	do {pos = line.find("\n"); if (pos!=std::string::npos) line.erase(pos);} while (pos!=std::string::npos);
	res.first = "";
	res.second = "";
	pos = line.find("=");
	if (pos!=std::string::npos) {
		res.first = std::string(line,0,pos);
		res.second = std::string(line,pos+1);
		pos = res.first.find_last_not_of(" ");
		if (pos != std::string::npos) res.first = std::string(res.first,0,pos+1);	// delete last spaces
		pos = res.second.find_first_not_of(" ");
		if (pos != std::string::npos) res.second = std::string(res.second,pos);		// delete first spaces
	} else {
		res.first = line;
		res.second = "";
	}
	return res;
}
