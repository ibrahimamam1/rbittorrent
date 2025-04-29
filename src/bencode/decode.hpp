#include "../lib/nlohmann/json.hpp"
#include <cctype>
#include <cstdlib>
#include <string>

using json = nlohmann::json;

class BencodeDecoder {
private:
  static json decode_string(const std::string &encoded_value, size_t &cursor);
  static json decode_int(const std::string &encoded_value, size_t &cursor);
  static json decode_list(const std::string &encoded_value, size_t &cursor);
  static json decode_dictionary(const std::string &encoded_value, size_t &cursor);

public:
  static json decode_bencoded_value(const std::string &encoded_value);
};
