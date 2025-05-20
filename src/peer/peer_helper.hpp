#include "../torrent/torrent.hpp"
#include "peer.hpp"
#include "../network/network_manager.hpp"
#include <boost/asio/io_context.hpp>
#include <vector>


class PeerDownloadHelper{
  std::vector<Peer>peerList;
  boost::asio::io_context ioc;  

public:
  PeerDownloadHelper();
  PeerDownloadHelper(json data);
  void performBitTorrentHandshakeWithPeers(const std::string& info_hash); //returns the number of succesfull connections
  void cleanupFailedConnections();
  void startDownloadLoop();
};
