#include "peer.hpp"
#include <cstdint>
#include <vector>

using PeerList = std::vector<std::shared_ptr<Peer>>;

class DownloadHelper {
public:
  void startDownloadLoop(PeerList &peers);

private:
  void
  sendMessage(beast::tcp_stream &stream, const uint32_t len_prefix,
              const uint16_t id,
              std::vector<unsigned char> payload =
                  std::vector<unsigned char>() = std::vector<unsigned char>());

  void sendKeepAliveMessage(beast::tcp_stream &stream);
  void sendChokeMessage(beast::tcp_stream &stream);
  void sendUnchokeMessage(beast::tcp_stream &stream);
  void sendAmInterestedMessage(beast::tcp_stream &stream);
  void sendAmNotInterestedMessage(beast::tcp_stream &stream);
  void sendHaveMessage(beast::tcp_stream &stream, uint32_t piece_index);
  void sendRequestMessage(beast::tcp_stream &stream, const uint32_t piece_index,
                          const uint32_t offset, const uint32_t piece_length);
  void sendCancelMessage(beast::tcp_stream &stream, uint32_t pieceIndex,
                         uint32_t offset, uint32_t length);

  void sendPiece(beast::tcp_stream &stream, const uint32_t piece_index,
                 const uint32_t offset, const std::vector<unsigned char> data);
};
