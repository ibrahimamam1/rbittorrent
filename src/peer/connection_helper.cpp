#include "connection_helper.hpp"
#include <atomic>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <iostream>
#include <memory>

    PeerConnectionHelper::PeerConnectionHelper() {
}

PeerConnectionHelper::PeerConnectionHelper(json data) {
  for (auto &peer : data) {
    Peer p(ioc, peer["ip"], peer["port"]);
    peerList.push_back(std::make_shared<Peer>(std::move(p)));
  }
}

// perform bittorrent handshake with
// all the clients in the PeerList
// returns number of sucesfull connections
size_t PeerConnectionHelper::performBitTorrentHandshakeWithPeers(
    const std::string &info_hash) {
  std::cout << "Performing Handshake with Peers...\n";
  // keep track of active operations for synchronization
  std::shared_ptr<size_t> active_ops =
      std::make_shared<size_t>(peerList.size());

  std::atomic<size_t> success = 0;

  // timeout timer for connection
  // can't wait indefinitely for a peer to connect
  auto timeout =
      std::make_shared<asio::steady_timer>(ioc, std::chrono::seconds(30));
  auto timeout_occured = std::make_shared<bool>(false);
  // connect to all peers in peerList
  for (auto &peer : peerList) {
    peer->connectWithRetries(
        3, // maximum retry attempts
        info_hash,
        [this, peer, active_ops, timeout, timeout_occured,
         &success](CONNECTION_STATE state) {
          if (state == CONNECTED) {
            peer->setState(CONNECTED);
          }
          if (state == HANDSHAKE_COMPLETE) {
            peer->setState(HANDSHAKE_COMPLETE);
            (*active_ops)--;
            success++;
          } else if (state == HANDSHAKE_FAILED) {
            peer->setState(HANDSHAKE_FAILED);
            (*active_ops)--;
          }

          if (*active_ops == 0 && !*timeout_occured) {
            std::cout << "=== ALL OPERATIONS COMPLETE - STOPPING IOC ==="
                      << std::endl;
            timeout->cancel();
            ioc.stop();
          }
        });
  }

  // set up the timeout
  timeout->async_wait(
      [this, active_ops, timeout_occured](boost::system::error_code error) {
        if (!error) {
          *timeout_occured = true;

          // Cancel all pending operations on peer streams
          int i = 0;
          for (auto &peer : peerList) {
            peer->getStream()->cancel();
          }

          *active_ops = 0;
          ioc.stop();
        }
      });
  // run the schedules operations
  ioc.run();
  return success;
}

void PeerConnectionHelper::cleanupFailedConnections() {
  std::cout << "Cleaning up failed connnections...\n";
  size_t count = 0;
  for (std::vector<std::shared_ptr<Peer>>::iterator peer = peerList.begin();
       peer != peerList.end();) {
    if ((*peer)->getState() == FAILED ||
        (*peer)->getState() == HANDSHAKE_FAILED ||
        (*peer)->getState() == NOT_CONNECTED) {
      peer = peerList.erase(peer);
      count++;
    } else
      ++peer;
  }
}

