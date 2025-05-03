#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "helpers.hpp"
#include <openssl/sha.h>

bool isValidNumber(const std::string& str) {
    if (str.empty()) return false;
    
    // Allow optional leading '+' or '-'
    size_t start = (str[0] == '+' || str[0] == '-') ? 1 : 0;
    
    return start < str.size() && std::all_of(str.begin() + start, str.end(), ::isdigit);
}

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(const std::string& input) {
    std::string output;
    size_t i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(input.data());
    size_t in_len = input.size();

    while (i < in_len) {
        char_array_3[0] = (i < in_len) ? bytes[i++] : 0;
        char_array_3[1] = (i < in_len) ? bytes[i++] : 0;
        char_array_3[2] = (i < in_len) ? bytes[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) | ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) | ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < 4; j++) {
            output += base64_chars[char_array_4[j]];
        }
    }

    // Padding
    size_t padding = (3 - (in_len % 3)) % 3;
    for (size_t j = 0; j < padding; j++) {
        output[output.length() - 1 - j] = '=';
    }

    return output;
}

BencodeValue json_to_bencode(const json& j) {
    if (j.is_string()) {
        return BencodeValue(j.get<std::string>());
    } else if (j.is_number_integer()) {
        return BencodeValue(j.get<long long>());
    } else if (j.is_array()) {
        BencodeList list;
        for (const auto& item : j) {
            list.push_back(json_to_bencode(item));
        }
        return BencodeValue(list);
    } else if (j.is_object()) {
        BencodeDict dict;
        for (const auto& item : j.items()) {
            dict[item.key()] = json_to_bencode(item.value());
        }
        return BencodeValue(dict);
    } else {
        throw std::runtime_error("Unsupported JSON type for Bencode conversion");
    }
}

std::vector<unsigned char> compute_sha1(const std::string& val) {
    std::vector<unsigned char> hash(SHA_DIGEST_LENGTH); // SHA1 digest is 20 bytes
    SHA1(reinterpret_cast<const unsigned char*>(val.data()), val.length(), hash.data());
    return hash;
}
