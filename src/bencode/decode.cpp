#include "decode.hpp"
#include "../helpers/helpers.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

std::string decode_string(const std::string &encoded_value, size_t &cursor) {
  // Example: "5:hello" -> "hello"
  size_t colon_index =
      encoded_value.find(':', cursor); // Example: "5:hello" -> "hello"
  if (colon_index != std::string::npos) {
    std::string number_string =
        encoded_value.substr(cursor, (colon_index - cursor));
    if (!isValidNumber(number_string)) {
      throw std::runtime_error("Invalid bencode: " + number_string +
                               "is Not a valid length");
    }

    int64_t number = std::atoll(number_string.c_str());
    std::string str = encoded_value.substr(colon_index + 1, number);
    if (str.length() != number) {
      throw std::runtime_error("Invalid bencode: Length does not match");
    }
    json json_str = json(str);
    cursor = colon_index + number + 1;
    return json_str.dump();
  } else {
    throw std::runtime_error(
        "Invalid encoded value: missing column after length in" +
        encoded_value);
  }
}

std::string decode_int(const std::string &encoded_value, size_t &cursor) {
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
  return number_str;
}


std::string decode_bencoded_value(const std::string &encoded_value) {
  std::string decoded_value = "";
  size_t cursor = 0;
  size_t len = encoded_value.length();

  if (len == 0)
    throw std::runtime_error("No input");

  std::stack<char> types_encountered;
  bool firstListElement = true;
  bool isValueElement = false;
  std::string lastDictionarykey = "";

  while (cursor < len) {
    // decode string
    if (std::isdigit(encoded_value[cursor])) {
      std::string str = decode_string(encoded_value, cursor);

      if (types_encountered.size() > 0 && types_encountered.top() == 'd'){
        if(isValueElement)  decoded_value += ":";
        else{
          if(!firstListElement) decoded_value += ",";
          if(str < lastDictionarykey) throw std::runtime_error("Invalid Bencode: Dictionary keys must be sorted in ascending order => " + str + " > " + lastDictionarykey);
          lastDictionarykey = str;
          firstListElement = false;
        }
        isValueElement = !isValueElement;
      }
      else if (types_encountered.size() > 0 &&
          !firstListElement)
        decoded_value += ",";


      decoded_value += str;
    } // decode integer
    else if (encoded_value[cursor] == 'i') {
      if (types_encountered.size() > 0 && types_encountered.top() == 'd'){
        if(isValueElement)
          decoded_value += ":";
        else 
          throw std::runtime_error("Invalid Bencode: dictionary Key must be string");
        
        isValueElement = !isValueElement;
      }
      else if (types_encountered.size() > 0 &&
          !firstListElement)
        decoded_value += ",";
      decoded_value += decode_int(encoded_value, cursor);
    } // start list
    else if (encoded_value[cursor] == 'l') {
      types_encountered.push('l');
      if (!firstListElement)
        decoded_value += ",";

      decoded_value += "[";
      firstListElement = true;
      cursor++;
    }//start dictionary
    else if (encoded_value[cursor] == 'd') {
      types_encountered.push('d');
      decoded_value += "{";
      isValueElement = false;
      cursor++;
    } else if (encoded_value[cursor] == 'e') {
      if (types_encountered.size() == 0)
        throw std::runtime_error("Invalid bencode value: " + encoded_value);
      else if (types_encountered.top() == 'l')
        decoded_value += "]";
      else if (types_encountered.top() == 'd')
        decoded_value += "}";

      types_encountered.pop();
      cursor++;
    } else {
      throw std::runtime_error("Invalid Bencode: " + encoded_value);
    }
    if(types_encountered.size() > 0 && types_encountered.top() == 'l')
      firstListElement = false;
  }
  if (types_encountered.size() > 0)
    throw std::runtime_error("Invalid Bencode: Missing 'e'");
  return decoded_value;
}
