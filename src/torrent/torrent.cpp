#include "torrent.hpp"
#include "../bencode/decode.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

Torrent parse_torrent_file(std::string filepath){
  std::ifstream file(filepath, std::ios::ate);
  if(!file.is_open()){
    throw std::runtime_error("Failed to open file " + filepath);
  }

  //get file size
  auto size = file.tellg();
  file.seekg(0);

  //read entire file
  std::string content;
  content.resize(size);
  file.read(&content[0], size);

  json decoded = decode_bencoded_value(content);
  std::cout << decoded << std::endl;
}
