#include <cctype>
#include <cstdlib>
#include <iostream>
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
    std::string number_str = encoded_value.substr(1, end_index-1);
    return json(number_str);
  } else {
    throw std::runtime_error("Invalid encoded value: " + encoded_value);
  }
}

json decode_bencoded_value(const std::string &encoded_value) {
  if (std::isdigit(encoded_value[0])) {
    return decode_string(encoded_value);
  } else if (encoded_value[0] == 'i') {
    return decode_int(encoded_value);
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
    json decoded_value = decode_bencoded_value(encoded_value);
    
    // Print without quotes based on the type
    if (decoded_value.is_number_integer()) {
      std::cout << decoded_value.get<int>() << std::endl;
    } else {
      // Fallback to dump() for other types (e.g., arrays, objects)
      std::cout << decoded_value.dump() << std::endl;
    }
  } else {
    std::cerr << "unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
