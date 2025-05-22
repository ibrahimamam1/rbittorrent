#include "../torrent/torrent.hpp"
#include "peer.hpp"
#include <boost/asio/io_context.hpp>
#include <vector>
#include <atomic>

class PeerDownloadHelper{
  std::vector<std::shared_ptr<Peer>>peerList;
  boost::asio::io_context ioc;

public:
  PeerDownloadHelper();
  PeerDownloadHelper(json data);
  void performBitTorrentHandshakeWithPeers(const std::string& info_hash); //returns the number of succesfull connections
  void cleanupFailedConnections();
  void startDownloadLoop();
};
