#include "peer.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>
#include "../message/message_helper.hpp"

using PeerList = std::vector<std::shared_ptr<Peer>>;

class DownloadHelper {
public:
  DownloadHelper(PeerList &peers_, const size_t total_size_, const size_t number_of_pieces_);
  void startDownloadLoop();
  void sendInterestedMessageToAllPeers();
  void handlePeerMessage(std::shared_ptr<Peer>& peer, MESSAGE_TYPE type, std::vector<unsigned char>& response);
  bool peerHasNeededPiece(Peer&);
  int findBestPiece(std::vector<int>bit_field);
private:
  PeerList peers;
  size_t total_size;
  size_t number_of_pieces;
  bool download_complete;
  PeerList active_peers;
  std::vector<int>my_bitfield;
  };
