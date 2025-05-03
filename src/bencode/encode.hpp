#pragma once
#include "bencode.hpp"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <sstream>   // For string building


class BencodeEncoder {
public:
    // Encodes a string
    static std::string encode(const std::string& str) {
        std::stringstream ss;
        ss << str.length() << ":" << str;
        return ss.str();
    }

    // Encodes an integer
    static std::string encode(long long num) {
        std::stringstream ss;
        ss << "i" << num << "e";
        return ss.str();
    }

    // Encodes a list (vector of BencodeValue)
    static std::string encode(const BencodeList& list) {
        std::stringstream ss;
        ss << "l";
        for (const auto& item : list) {
            ss << encode(item); // Recursive call for each item
        }
        ss << "e";
        return ss.str();
    }

    // Encodes a dictionary (map<string, BencodeValue>)
    static std::string encode(const BencodeDict& dict) {
        std::stringstream ss;
        ss << "d";
        // std::map automatically keeps keys sorted
        for (const auto& pair : dict) {
            ss << encode(pair.first);  // Encode the string key
            ss << encode(pair.second); // Encode the value (recursive)
        }
        ss << "e";
        return ss.str();
    }

    // Main encode function using std::visit on the variant
    static std::string encode(const BencodeValue& value) {
        return std::visit([](const auto& val) -> std::string {
            // Call the appropriate overload based on the variant's current type
            return encode(val);
        }, value);
    }
};


