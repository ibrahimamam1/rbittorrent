#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <vector>

namespace beast = boost::beast;
enum MESSAGE_TYPE{KEEP_ALIVE, BITFIELD,CHOKE, UNCHOKE, INTERESTED, NOT_INTERESTED, HAVE, REQUEST, CANCEL};
class MessageHelper{
public:
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
  bool hasMessage(beast::tcp_stream& stream); 
  std::vector<unsigned char>readResponse(beast::tcp_stream& stream);
  MESSAGE_TYPE getResponseType(std::vector<unsigned char> response);

private:
void
  sendMessage(beast::tcp_stream &stream, const uint32_t len_prefix,
              const uint16_t id,
              std::vector<unsigned char> payload =
                  std::vector<unsigned char>() = std::vector<unsigned char>());


};
