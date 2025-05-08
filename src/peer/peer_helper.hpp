#include "../torrent/torrent.hpp"
#include "peer.hpp"
#include "../network/network_manager.hpp"
#include <vector>


class PeerDownloadHelper{
  std::vector<Peer>peerList;
  

public:
  PeerDownloadHelper();
  PeerDownloadHelper(json data);
  size_t performBitTorrentHandshakeWithPeers(NetworkManager& nw, const std::string& info_hash); //returns the number of succesfull connections
};
