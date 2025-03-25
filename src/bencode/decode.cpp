#include "decode.hpp"
#include "../helpers/helpers.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

std::string decode_string(const std::string &encoded_value) {
  // Example: "5:hello" -> "hello"
  size_t colon_index = encoded_value.find(':');
  if (colon_index != std::string::npos) {
    std::string number_string = encoded_value.substr(0, colon_index);
    if(!isValidNumber(number_string)){
      throw std::runtime_error("Invalid bencode: Not a valid length");
    }

    int64_t number = std::atoll(number_string.c_str());
    std::string str = encoded_value.substr(colon_index + 1, number);
    if(str.length() != number){
      throw std::runtime_error("Invalid bencode: Length does not match");
    }

    json decoded_str = json(str);
    return decoded_str.dump();
  } else {
    throw std::runtime_error("Invalid encoded value: " + encoded_value);
  }
}

std::string decode_int(const std::string &encoded_value) {
  // example i52e -> 52
  size_t end_index = encoded_value.find('e');
  if(end_index != encoded_value.length()-1 || end_index == std::string::npos){
    throw std::runtime_error("Invalid Bencode: integers must end with 'e'");
  } 
  std::string number_str = encoded_value.substr(1, end_index - 1);
  if(!isValidNumber(number_str)){
    throw std::runtime_error("Invalid Bencode: Not a valid integer");
  }

  return number_str; 
}

std::string decode_list(const std::string &encode_value) {
  std::string list_str = "[";
  size_t len = encode_value.length();
  size_t cursor = 1;
  bool first = true;
  std::stack<char> types_encountered;
  types_encountered.push('l');
  while (cursor < len) {
    if (!first)
      list_str += ",";
    if (std::isdigit(encode_value[cursor])) {
      first = false;
      size_t colon_index = encode_value.find(':', cursor);
      std::string number_str = encode_value.substr(cursor, colon_index);
      size_t number = std::atoll(number_str.c_str());
      std::string str = decode_string(encode_value.substr(cursor, cursor + number + 1));
      list_str += str;
                cursor = colon_index + number + 1;

    } else if (encode_value[cursor] == 'i') {
      first = false;
      size_t end_index = encode_value.find('e', cursor);
      std::string number = decode_int(encode_value.substr(cursor, end_index+1));
      list_str += number;
      cursor = end_index + 2;
    } else if (encode_value[cursor] == 'l') {
      types_encountered.push('l');
      list_str += "[";
      first = true;
      cursor++;
    } else if (encode_value[cursor] == 'e') {
      if (types_encountered.size() == 0)
        throw std::runtime_error("Invalid bencode value: " + encode_value);
      else if (types_encountered.top() == 'l')
        list_str[list_str.length() - 1] = ']';
      types_encountered.pop();
      cursor++;
    } else {
      throw std::runtime_error("Invalid bencode value: " + encode_value);
    }
  }
  if (types_encountered.size() != 0)
    throw std::runtime_error("Invalid Bencode value: " + encode_value);

  if (list_str.length() == 1)
    list_str = "[]"; // special case of empty list
  return list_str;
}

std::string decode_dictionary(const std::string &encoded_value){

}

std::string decode_bencoded_value(const std::string &encoded_value) {
  if (std::isdigit(encoded_value[0])) {
    std::string decoded_str = decode_string(encoded_value);
    return decoded_str;
  } else if (encoded_value[0] == 'i') {
    return decode_int(encoded_value);
  } else if (encoded_value[0] == 'l') {
    return decode_list(encoded_value);
  } else {
    throw std::runtime_error("Unhandled encoded value: " + encoded_value);
  }
}


