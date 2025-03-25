#include "../lib/nlohmann/json.hpp"
#include <cctype>
#include <cstdlib>
#include <string>

using json = nlohmann::json;


std::string decode_string(const std::string &encoded_value);
std::string decode_int(const std::string &encoded_value);
std::string decode_list(const std::string &encoded_value);
std::string decode_bencoded_value(const std::string &encoded_value);
