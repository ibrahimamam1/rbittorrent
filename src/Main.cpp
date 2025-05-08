#include "bencode/decode.hpp"
#include "network/network_manager.hpp"
#include "peer/peer_helper.hpp"
#include "torrent/torrent.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "<torrent file path>" << std::endl;
    return 1;
  }
  TorrentHelper helper;
  try{
    bool success = helper.parseTorrentFile(argv[1]);
    if(success){
      NetworkManager nw;
      json peer_data = helper.getPeers(nw);
      size_t interval = helper.getInterval();
      
      PeerDownloadHelper peer_helper(peer_data);
      peer_helper.performBitTorrentHandshakeWithPeers(nw, helper.getInfoHash());
      
    }
  }catch(std::runtime_error e){
    std::cerr << e.what() << std::endl;
    exit(1);
  }
  return 0;
}
