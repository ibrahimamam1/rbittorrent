#pragma once

#include "../torrent/torrent.hpp"
#include "peer.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>

class PeerConnectionHelper {
  boost::asio::io_context ioc;
  std::vector<std::shared_ptr<Peer>> peerList;

public:
  PeerConnectionHelper();
  PeerConnectionHelper(json data);
  size_t performBitTorrentHandshakeWithPeers(
      const std::string
          &info_hash); // returns the number of succesfull connections
  void cleanupFailedConnections();
  std::vector<std::shared_ptr<Peer>>& getPeerList() {return peerList;}
};
