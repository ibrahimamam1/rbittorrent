#include "peer_helper.hpp"
#include <algorithm>
#include <atomic>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <exception>
#include <execution>
#include <iostream>
#include <memory>
#include <mutex>

PeerDownloadHelper::PeerDownloadHelper() {}

PeerDownloadHelper::PeerDownloadHelper(json data) {
  for (auto &peer : data) {
    Peer p(ioc, peer["ip"], peer["port"]);
    peerList.push_back(std::make_shared<Peer>(std::move(p)));
  }
}

// perform bittorrent handshake with client
// return number of successfull connections
void PeerDownloadHelper::performBitTorrentHandshakeWithPeers(
    const std::string &info_hash) {
  // keep track of active operations for synchronization
  std::shared_ptr<size_t> active_ops =
      std::make_shared<size_t>(peerList.size());

  for (auto &peer : peerList) {
    // we need a shared pointer to the peer object to extends it's lifetime
    // this avoid dangling references when we do asynchronous network operations
    // using the object
    peer->connectWithRetries(
        3, // maximum retry attemps
        info_hash, [this, peer, active_ops](CONNECTION_STATE state) {
          if (state == CONNECTED) {
            peer->setState(CONNECTED);
            std::cout << "Succesfully Connected with: " << peer->getIp()
                      << std::endl;
          }
          if (state == HANDSHAKE_COMPLETE) {
            peer->setState(HANDSHAKE_COMPLETE);
            std::cout << "Handshake complete with: " << peer->getIp()
                      << std::endl;
          } else if (state == HANDSHAKE_FAILED) {
            peer->setState(HANDSHAKE_FAILED);
            std::cout << "Handshake Failed with: " << peer->getIp()
                      << std::endl;
          }

          if (state == HANDSHAKE_COMPLETE || state == HANDSHAKE_FAILED ||
              state == FAILED) {
            (*active_ops)--;
            if (*active_ops == 0)
              ioc.stop();
          }
        });
  }

  // We set a reasonable timeout for the entire operation
  // can't have this thing run forever
  asio::steady_timer timeout(ioc, std::chrono::seconds(30));
  timeout.async_wait([this](boost::system::error_code error) {
    if (!error) {
      std::cout << "Operation timeout after 30 seconds\n";
      ioc.stop();
    }
  });

  // run the schedules operations
  ioc.run();
  cleanupFailedConnections();
}

void PeerDownloadHelper::cleanupFailedConnections() {
  std::cout << "Cleaning up failed connnections\n";
  size_t count = 0;
  for (std::vector<std::shared_ptr<Peer>>::iterator peer = peerList.begin();
       peer != peerList.end();) {
    if ((*peer)->getState() == FAILED ||
        (*peer)->getState() == HANDSHAKE_FAILED ||
        (*peer)->getState() == NOT_CONNECTED
    ) {
      peer = peerList.erase(peer);
      count++;
    } else
      ++peer;
  }
  std::cout << "Cleaned " << count << " Connections\n";
 
}

void PeerDownloadHelper::startDownloadLoop() {}
