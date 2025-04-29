#pragma once

#include <string>
#include <utility>
#include <vector>
#include "../network/network_manager.hpp"

typedef struct file{
  std::string name;
  size_t length;
}File;

class TorrentHelper{
  std::string tracker_url;
  std::string info;
  std::string piece_length;
  std::string name;
  std::string pieces;
  size_t length = 0;
  std::vector<File>files;
  
  //A Peer is represented as a pair <IP_ADDRES, PORT_NUMBER>
  std::vector<std::pair<std::string, std::string>>Peers;
public:
  bool parseTorrentFile(std::string filepath);
  bool getPeers(NetworkManager& nw);
};

