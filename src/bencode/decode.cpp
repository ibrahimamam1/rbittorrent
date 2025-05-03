#include "decode.hpp"
#include "../helpers/helpers.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

json BencodeDecoder::decode_string(const std::string &encoded_value, size_t &cursor) {
  // Example: "5:hello" -> "hello"
  size_t colon_index = encoded_value.find(':', cursor);
  if (colon_index != std::string::npos) {
    std::string number_string =
        encoded_value.substr(cursor, (colon_index - cursor));
    if (!isValidNumber(number_string)) {
      throw std::runtime_error("Invalid bencode: " + number_string +
                               "is Not a valid length");
    }

    int64_t number = std::atoll(number_string.c_str());
    if (number < 0)
      throw std::runtime_error("Invalid bencode: Length must be positive");

    std::string str = encoded_value.substr(colon_index + 1, number);
    cursor = colon_index + number + 1;
    if (str.length() != number)
      throw std::runtime_error(
          "Invalid Bencode: length and string length do not match");
    return json(str);
  } else {
    throw std::runtime_error(
        "Invalid encoded value: missing column after length in" +
        encoded_value);
  }
}

json BencodeDecoder::decode_int(const std::string &encoded_value, size_t &cursor) {
  // example i52e -> 52
  size_t end_index = encoded_value.find('e', cursor);
  if (end_index == std::string::npos) {
    throw std::runtime_error("Invalid Bencode: integers must end with 'e'");
  }
  std::string number_str =
      encoded_value.substr(cursor + 1, (end_index - cursor - 1));
  if (!isValidNumber(number_str)) {
    throw std::runtime_error("Invalid Bencode: " + number_str +
                             " is Not a valid integer");
  }

  cursor = end_index + 1;
  long long num = atoll(number_str.c_str());
  return json(num);
}

json BencodeDecoder::decode_list(const std::string &encoded_value, size_t &cursor) {
  json list;

  size_t len = encoded_value.length();
  while (cursor < len) {
    if (std::isdigit(encoded_value[cursor])) {
      json str = decode_string(encoded_value, cursor);
      list.push_back(str);
    } else if (encoded_value[cursor] == 'i') {
      json num = decode_int(encoded_value, cursor);
      list.push_back(num);
    } else if (encoded_value[cursor] == 'l') {
      cursor++;
      json sublist = decode_list(encoded_value, cursor);
      list.push_back(sublist);
    } else if (encoded_value[cursor] == 'd') {
      json dic = decode_dictionary(encoded_value, cursor);
      list.push_back(dic);
    } else if (encoded_value[cursor] == 'e') {
      cursor++;
      return list;
    } else {
      throw std::runtime_error("Invalid bencode: unkonw character: " +
                               encoded_value[cursor]);
    }
  }
  throw std::runtime_error("Invalid bencode in " + encoded_value +
                           " : list must end with 'e'");
}


json BencodeDecoder::decode_dictionary(const std::string &encoded_value, size_t &cursor) {
  json dic;
  bool isKey = true;
  json key, value;
  size_t len = encoded_value.length();
  
  while (cursor < len) {
    json str;
    
    if (std::isdigit(encoded_value[cursor])) {
      str = decode_string(encoded_value, cursor);
    } else if (encoded_value[cursor] == 'i') {
      str = decode_int(encoded_value, cursor);
    } else if (encoded_value[cursor] == 'l') {
      cursor++;
      str = decode_list(encoded_value, cursor);
    } else if (encoded_value[cursor] == 'd') {
      cursor++;
      str = decode_dictionary(encoded_value, cursor);
    } else if (encoded_value[cursor] == 'e') {
      cursor++;
      return dic;
    } else {
      throw std::runtime_error("Invalid bencode: unknown character: " +
                               std::string(1, encoded_value[cursor]));
    }

    if (isKey) {
      key = str;
      isKey = false;  // Next should be a value
    } else {
      value = str;
      // Special handling for "pieces" field
      if (key.get<std::string>() == "pieces" && value.is_string()) {
        std::string binary_str = value.get<std::string>();
        dic[key] = binary_str;
      } else {
        dic[key] = value;
      }
      isKey = true;  // Next should be a key
    }
  }

  throw std::runtime_error("Invalid bencode in " + encoded_value +
                           " : dictionary must end with 'e'");
}

json BencodeDecoder::decode_bencoded_value(const std::string &encoded_value) {
  size_t cursor = 0;
  json decoded_value;
  if (std::isdigit(encoded_value[0])) {
    decoded_value = decode_string(encoded_value, cursor);
  } else if (encoded_value[0] == 'i') {
    decoded_value = decode_int(encoded_value, cursor);
      } else if (encoded_value[0] == 'l') {
    cursor++;
    decoded_value = decode_list(encoded_value, cursor);
  } else if (encoded_value[0] == 'd') {
    cursor++;
    decoded_value = decode_dictionary(encoded_value, cursor);
  } else {
    throw std::runtime_error("Invalid Bencode symbol");
  }
  if (cursor != encoded_value.length())
      throw std::runtime_error("Invalid bencode: " + encoded_value);
  
  return decoded_value;
}
