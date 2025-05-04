#include "peer_helper.hpp"
#include <exception>
#include <iostream>

PeerDownloadHelper::PeerDownloadHelper() {}

PeerDownloadHelper::PeerDownloadHelper(json data) {
  for (auto peer : data) {
    Peer p(peer["ip"], peer["port"]);
    peerList.push_back(p);
  }
}

// make tcp handshake with peers
// returns number of succesfull connections
size_t
PeerDownloadHelper::inititateTcpConnectionsWithPeers(NetworkManager &nw) {
  size_t success = 0;
  try {
    for (auto &peer : peerList) {
      peer.connect();
      success++;
    }
  }catch(std::exception e){
    std::cerr << e.what() << std::endl;
    return success;
  }
  return success;
}

//perform bittorrent handshake with client
//return number of successfull connections
size_t PeerDownloadHelper::performBitTorrentHanshakeWithPeers(NetworkManager& nw, const std::string& info_hash){
  std::string pstr = "BitTorrent protocol";
  size_t pstrlen = pstr.length();
  std::string peer_id = PEER_ID;

  for(const auto& peer : peerList){
    peer.performBitTorrentHanshake(pstr, pstrlen, peer_id, info_hash);
  }
}
