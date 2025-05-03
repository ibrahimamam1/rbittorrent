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

typedef struct{
  std::string ip;
  size_t port;
}Peer;
using PeerList = std::vector<Peer>;

class TorrentHelper{
  std::string tracker_url;
  json info;
  std::vector<unsigned char>raw_info;
  size_t piece_length;
  std::string name;
  std::string pieces;
  size_t length = 0;
  std::vector<File>files;
  size_t interval;
  
  //A Peer is represented as a pair <IP_ADDRES, PORT_NUMBER>
  std::vector<std::pair<std::string, std::string>>Peers;
public:
  bool parseTorrentFile(std::string filepath);
  PeerList getPeers(NetworkManager& nw);
  std::string generatePeerId();
  size_t getInterval() const;
};

