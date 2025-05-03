#pragma once

#include <string>
#include <utility>
#include <vector>
#include "../network/network_manager.hpp"
#include "../lib/nlohmann/json.hpp"

using json = nlohmann::json;
typedef struct file{
  std::string name;
  size_t length;
}File;

class TorrentHelper{
  std::string tracker_url;
  json info;
  std::vector<unsigned char>raw_info;
  size_t piece_length;
  std::string name;
  std::string pieces;
  size_t length = 0;
  std::vector<File>files;
  
  //A Peer is represented as a pair <IP_ADDRES, PORT_NUMBER>
  std::vector<std::pair<std::string, std::string>>Peers;
public:
  bool parseTorrentFile(std::string filepath);
  bool getPeers(NetworkManager& nw);
  std::string generatePeerId();
};

