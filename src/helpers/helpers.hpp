#include <cstdint>
#include <string>
#include <vector>
#include "../bencode/encode.hpp"
#include "../lib/nlohmann/json.hpp"

using json = nlohmann::json;


bool isValidNumber(const std::string& str);
std::string base64_encode(const std::string& in);
BencodeValue json_to_bencode(const json& j);
std::vector<unsigned char> compute_sha1(const std::string& val);
std::vector<unsigned char> intToBigEndianBytes(uint32_t value);
