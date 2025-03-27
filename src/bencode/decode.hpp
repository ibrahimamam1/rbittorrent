#include "../lib/nlohmann/json.hpp"
#include <cctype>
#include <cstdlib>
#include <string>

using json = nlohmann::json;


json decode_string(const std::string &encoded_value, size_t& cursor);
json decode_int(const std::string &encoded_value, size_t& cursor);
json decode_list(const std::string &encoded_value, size_t& cursor);
json decode_dictionary(const std::string &encoded_value, size_t& cursor);
json decode_bencoded_value(const std::string &encoded_value);
