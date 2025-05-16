#include "peer_helper.hpp"
#include <algorithm>
#include <atomic>
#include <exception>
#include <execution>
#include <iostream>
#include <mutex>

PeerDownloadHelper::PeerDownloadHelper() {}

PeerDownloadHelper::PeerDownloadHelper(json data) {
  for (auto &peer : data) {
    Peer p(peer["ip"], peer["port"]);
    peerList.push_back(std::move(p));
  }
}

// perform bittorrent handshake with client
// return number of successfull connections
void PeerDownloadHelper::performBitTorrentHandshakeWithPeers(
    const std::string &info_hash) {

  std::for_each(std::execution::par, peerList.begin(), peerList.end(),
                [&](auto &&peer) {
                  peer.connectWithRetries(3); // perform tcp connection
                  if (peer.getState() == CONNECTED) {
                    peer.performBitTorrentHandshake(info_hash);
                  }
                });
}

void PeerDownloadHelper::cleanupFailedConnections() {
  size_t count = 0;
  for (std::vector<Peer>::iterator peer = peerList.begin();
       peer != peerList.end();) {
    if (peer->getState() == FAILED || peer->getState() == HANDSHAKE_FAILED) {
      peer = peerList.erase(peer);
      count++;
    } else
      ++peer;
  }
  std::cout << "Cleaned " << count << " Connections\n";
  std::cout << "New states in peerList:\n";
  for (std::vector<Peer>::iterator peer = peerList.begin();
       peer != peerList.end();) {
    std::cout << peer->getState() << std::endl;
  }
}

void PeerDownloadHelper::startDownloadLoop() {}
