#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

json decode_string(const std::string &encoded_value) {
  // Example: "5:hello" -> "hello"
  size_t colon_index = encoded_value.find(':');
  if (colon_index != std::string::npos) {
    std::string number_string = encoded_value.substr(0, colon_index);
    int64_t number = std::atoll(number_string.c_str());
    std::string str = encoded_value.substr(colon_index + 1, number);
    return json(str);
  } else {
    throw std::runtime_error("Invalid encoded value: " + encoded_value);
  }
}

json decode_int(const std::string &encoded_value) {
  // example i52e -> 52
  size_t end_index = encoded_value.find('e');
  if (end_index != std::string::npos) {
    std::string number_str = encoded_value.substr(1, end_index - 1);
    return number_str;
  } else {
    throw std::runtime_error("Invalid encoded value: " + encoded_value);
  }
}

std::string decode_list(const std::string &encode_value) {
  std::cerr << "Decoding list\n";
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
      std::cerr << "found a string\n";
      first = false;
      size_t colon_index = encode_value.find(':', cursor);
      std::string number_str = encode_value.substr(cursor, colon_index);
      size_t number = std::atoll(number_str.c_str());
      json str = decode_string(encode_value.substr(cursor, cursor + number + 1));
      list_str += str.dump();
                cursor = colon_index + number + 1;

    } else if (encode_value[cursor] == 'i') {
      std::cerr << "found an int\n";
      first = false;
      size_t end_index = encode_value.find('e', cursor);
      std::string number = decode_int(encode_value.substr(cursor, end_index));
      list_str += number;
      cursor = end_index + 1;
    } else if (encode_value[cursor] == 'l') {
      std::cerr << "found a list\n";
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
    std::cerr << "list: " << list_str << " \n";
    std::cerr << "cursor: " << cursor << " \n";
  }
  if (types_encountered.size() != 0)
    throw std::runtime_error("Invalid Bencode value: " + encode_value);

  if (list_str.length() == 1)
    list_str = "[]"; // special case of empty list
  return list_str;
}

std::string decode_bencoded_value(const std::string &encoded_value) {
  if (std::isdigit(encoded_value[0])) {
    json decoded_str = decode_string(encoded_value);
    return decoded_str.dump();
  } else if (encoded_value[0] == 'i') {
    return decode_int(encoded_value);
  } else if (encoded_value[0] == 'l') {
    return decode_list(encoded_value);
  } else {
    throw std::runtime_error("Unhandled encoded value: " + encoded_value);
  }
}

int main(int argc, char *argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
    return 1;
  }

  std::string command = argv[1];

  if (command == "decode") {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " decode <encoded_value>"
                << std::endl;
      return 1;
    }

    std::string encoded_value = argv[2];
    std::string decoded_value = decode_bencoded_value(encoded_value);
    std::cout << decoded_value << std::endl;
  } else {
    std::cerr << "unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
