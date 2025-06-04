#include "bencode/decode.hpp"
#include "peer/connection_helper.hpp"
#include "peer/download_helper.hpp"
#include "torrent/torrent.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "<torrent file path>" << std::endl;
    return 1;
  }

  TorrentHelper torrent_helper;
  try {
    bool success = torrent_helper.parseTorrentFile(argv[1]);
    if (success) {
      json peer_data = torrent_helper.getPeers();
      size_t interval = torrent_helper.getInterval();

      PeerConnectionHelper peer_helper(peer_data);
      size_t success = peer_helper.performBitTorrentHandshakeWithPeers(
          torrent_helper.getInfoHash());
      std::cout << "Succesfully Handshaked with " << success << " peers\n";
      peer_helper.cleanupFailedConnections();

      DownloadHelper dlh;
      dlh.startDownloadLoop(peer_helper.getPeerList());
    }
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }
  return 0;
}
