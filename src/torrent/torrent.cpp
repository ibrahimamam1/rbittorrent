#include "torrent.hpp"
#include "../bencode/decode.hpp"
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;
namespace http = boost::beast::http;

bool TorrentHelper::parseTorrentFile(std::string filepath) {
  std::ifstream file(filepath, std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file " + filepath);
  }

  // find file size
  auto size = file.tellg();
  file.seekg(0);

  // read entire file
  std::string content;
  content.resize(size);
  file.read(&content[0], size);

  try {
    json decoded = BencodeDecoder::decode_bencoded_value(content);
    
    //extract top level information
    if(decoded.contains("announce"))
      tracker_url = decoded["announce"];
    else
     throw std::runtime_error("Invalid Torrent File: Missing announce field");
    
    json infoDictionary;
    if(decoded.contains("info"))
      infoDictionary = decoded["info"];
    else
     throw std::runtime_error("Invalid Torrent File: Missing info dictionary");

    this->info = infoDictionary.dump(); // will be used to calculate info hash in get request
    
    //extract info dictinary content
    if (infoDictionary.contains("piece length")) 
      piece_length = infoDictionary["piece length"];
    else 
      throw std::runtime_error("Invalid Torrent File: Missing piece length field");
    
    if (infoDictionary.contains("name")) 
      name = infoDictionary["name"];
    else 
      throw std::runtime_error("Invalid Torrent File: Missing name field");
    
    if (infoDictionary.contains("pieces")) 
      pieces = infoDictionary["pieces"];
    else 
      throw std::runtime_error("Invalid Torrent File: Missing pieces field");
    
    if (infoDictionary.contains("length")) {
      std::string l = infoDictionary["length"];
      length = std::atoll(l.c_str());
    }

  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}
  
bool TorrentHelper::getPeers(NetworkManager& nw){
  http::response<http::dynamic_body>res = nw.makeGetRequest(tracker_url);
  std::cout <<res <<std::endl;
}
