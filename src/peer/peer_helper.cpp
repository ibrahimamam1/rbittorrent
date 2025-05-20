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
    Peer p(ioc, peer["ip"], peer["port"]);
    peerList.push_back(std::move(p));
  }
}

// perform bittorrent handshake with client
// return number of successfull connections
void PeerDownloadHelper::performBitTorrentHandshakeWithPeers(
    const std::string &info_hash) {
  for (auto &peer : peerList) {
    peer.connectWithRetries(3, [&]() {
      if (peer.getState() == CONNECTED)
        peer.performBitTorrentHandshake(info_hash);
      if (peer.getState() == HANDSHAKE_COMPLETE)
        std::cout << "Handshake complete with: " << peer.getIp() << std::endl;
      else if (peer.getState() == HANDSHAKE_FAILED)
        std::cout << "Handshake Failed with: " << peer.getIp() << std::endl;
    });
  } // perform tcp connection

  std::cout << "will run scheduled async ops\n";
  ioc.run();
  std::cout << "after will run scheduled async ops\n";
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
