#include "peer.hpp"
#include <cstdint>
#include <vector>

using PeerList = std::vector<std::shared_ptr<Peer>>;

class DownloadHelper {
public:
  DownloadHelper(PeerList &peers_, const size_t total_size_, const size_t number_of_pieces_);
  void startDownloadLoop();
  std::vector<size_t> extractBitfieldInformation();

private:
  PeerList peers;
  size_t total_size;
  size_t number_of_pieces;
  };
