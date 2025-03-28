#include "torrent.hpp"
#include "../bencode/decode.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>

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
  json info = decoded["info"];
  
  Torrent torrent;
  torrent.tracker_url = decoded["announce"];
  torrent.piece_length = info["piece length"];
  torrent.name = info["name"];
  torrent.pieces = info["pieces"];
  if(info.contains("length")){
  std::string l = info["length"];
    torrent.length = std::atoll(l.c_str());
  }


  return torrent;
}
