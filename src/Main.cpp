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
      size_t totalSize = torrent_helper.getTotalSize();
      size_t numberOfPieces = torrent_helper.getNumberOfPieces();

      PeerConnectionHelper peer_helper(peer_data);
      size_t success = peer_helper.performBitTorrentHandshakeWithPeers(
          torrent_helper.getInfoHash());
      peer_helper.cleanupFailedConnections();

      DownloadHelper dlh(peer_helper.getPeerList(), totalSize, numberOfPieces);
      dlh.startDownloadLoop();
    }
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }
  return 0;
}
