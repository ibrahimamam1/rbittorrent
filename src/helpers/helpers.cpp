#include <iostream>
#include <string>
#include <algorithm>

bool isValidNumber(const std::string& str) {
    if (str.empty()) return false;
    
    // Allow optional leading '+' or '-'
    size_t start = (str[0] == '+' || str[0] == '-') ? 1 : 0;
    
    return start < str.size() && std::all_of(str.begin() + start, str.end(), ::isdigit);
}
