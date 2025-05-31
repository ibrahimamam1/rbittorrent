#pragma once

#include "../torrent/torrent.hpp"
#include "peer.hpp"
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>

class PeerDownloadHelper {
  boost::asio::io_context ioc;
  std::vector<std::shared_ptr<Peer>> peerList;

public:
  PeerDownloadHelper();
  PeerDownloadHelper(json data);
  void performBitTorrentHandshakeWithPeers(
      const std::string
          &info_hash); // returns the number of succesfull connections
  void cleanupFailedConnections();
  void startFileTransferLoop(const std::string &info_hash);
};
