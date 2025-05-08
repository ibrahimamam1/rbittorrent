#include "peer_helper.hpp"
#include <algorithm>
#include <atomic>
#include <exception>
#include <execution>
#include <iostream>
#include <mutex>

PeerDownloadHelper::PeerDownloadHelper() {}

PeerDownloadHelper::PeerDownloadHelper(json data) {
  for (auto peer : data) {
    Peer p(peer["ip"], peer["port"]);
    peerList.push_back(p);
  }
}

// perform bittorrent handshake with client
// return number of successfull connections
size_t PeerDownloadHelper::performBitTorrentHandshakeWithPeers(
    NetworkManager &nw, const std::string &info_hash) {
  std::string pstr = "BitTorrent protocol";
  size_t pstrlen = pstr.length();
  std::string peer_id = PEER_ID;

  std::for_each(
      std::execution::par, peerList.begin(), peerList.end(), [&](auto &&peer) {
        peer.connect(); // perform tcp connection
        if (peer.isTcpConnected()) {
          peer.performBitTorrentHandshake(pstr, pstrlen, info_hash, peer_id);
        }
      });
  return 0;
}
