#ifndef _UTILS_H_
#define _UTILS_H_

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <sstream>
#include <vector>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

static inline std::vector<std::string> &split(const std::string &s,const char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
	   elems.push_back(item);
    }
    return elems;
}

static inline std::vector<std::string> split(const std::string &s,const char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

static inline bool startsWith(const std::string& s1, const std::string& s2) {
    return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}

#endif
