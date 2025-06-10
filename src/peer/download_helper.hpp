#include "peer.hpp"
#include <cstdint>
#include <vector>

using PeerList = std::vector<std::shared_ptr<Peer>>;

class DownloadHelper {
public:
  DownloadHelper(PeerList &peers_, const size_t total_size_, const size_t number_of_pieces_);
  void startDownloadLoop();
  void sendInterestedMessageToAllPeers();
  void handlePeerMessages();
private:
  PeerList peers;
  size_t total_size;
  size_t number_of_pieces;
  bool download_complete;
  size_t active_peers;
  };
