#include "message_helper.hpp"
#include "../helpers/helpers.hpp"
#include "../network/network_manager.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

void MessageHelper::sendMessage(beast::tcp_stream &stream,
                                const uint32_t len_prefix, const uint16_t id,
                                std::vector<unsigned char> payload) {
  // prepare data to transfer
  std::vector<unsigned char> data;

  std::vector<unsigned char> prefix_bytes = intToBigEndianBytes(len_prefix);
  data.insert(data.end(), prefix_bytes.begin(), prefix_bytes.end());

  if (id != 0) { // keep alive has no id
    std::vector<unsigned char> id_bytes = intToBigEndianBytes(id);
    data.insert(data.end(), id_bytes.begin(), id_bytes.end());
  }

  data.insert(data.end(), payload.begin(), payload.end());

  NetworkManager nm;
  try {
    nm.writeToStream(stream, data);
  } catch (std::exception e) {
    std::cout << e.what() << std::endl;
  }
}

void MessageHelper::sendKeepAliveMessage(beast::tcp_stream &stream) {
  // length prefix = 0000
  // no messgae id
  // no payload
  sendMessage(stream, 0, 0);
}

void MessageHelper::sendChokeMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 1
  // no payload
  sendMessage(stream, 1, 0);
}

void MessageHelper::sendUnchokeMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 1
  // no payload
  sendMessage(stream, 0, 1);
}

void MessageHelper::sendAmInterestedMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 2
  // no payload
  sendMessage(stream, 1, 2);
}

void MessageHelper::sendAmNotInterestedMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 3
  // no payload
  sendMessage(stream, 1, 3);
}

void MessageHelper::sendHaveMessage(beast::tcp_stream &stream,
                                    const uint32_t piece_index) {
  // length prefix = 0005
  // id = 4
  // payload = zero based piece index
  std::vector<unsigned char> payload = intToBigEndianBytes(piece_index);
  sendMessage(stream, 5, 4, payload);
}

void MessageHelper::sendRequestMessage(beast::tcp_stream &stream,
                                       const uint32_t piece_index,
                                       const uint32_t offset,
                                       const uint32_t piece_length) {
  // length prefix = 0013
  // id = 6
  // payload = <zero based piece index><offset within the piece><length of
  // requested piece>
  //
  std::vector<unsigned char> payload;

  std::vector<unsigned char> piece_bytes = intToBigEndianBytes(piece_index);
  std::vector<unsigned char> offset_bytes = intToBigEndianBytes(offset);
  std::vector<unsigned char> length_bytes = intToBigEndianBytes(piece_length);

  payload.insert(payload.end(), piece_bytes.begin(), piece_bytes.end());
  payload.insert(payload.end(), offset_bytes.begin(), offset_bytes.end());
  payload.insert(payload.end(), length_bytes.begin(), length_bytes.end());

  sendMessage(stream, 12, 6, payload);
}

void MessageHelper::sendPiece(beast::tcp_stream &stream,
                              const uint32_t piece_index, const uint32_t offset,
                              const std::vector<unsigned char> data) {
  // length prefix = variable
  // id = 7
  // payload = <zero based piece index><offset within the piece><data>

  std::vector<unsigned char> payload;

  std::vector<unsigned char> index_bytes = intToBigEndianBytes(piece_index);
  std::vector<unsigned char> offset_bytes = intToBigEndianBytes(offset);

  payload.insert(payload.end(), index_bytes.begin(), index_bytes.end());
  payload.insert(payload.end(), offset_bytes.begin(), offset_bytes.end());
  payload.insert(payload.end(), data.begin(), data.end());

  sendMessage(stream, 9 + data.size(), 7, payload);
}

void MessageHelper::sendCancelMessage(beast::tcp_stream &stream,
                                      uint32_t pieceIndex, uint32_t offset,
                                      uint32_t length) {
  // length prefix = 13 (1 byte for ID + 4 + 4 + 4 bytes for payload)
  // id = 8
  // payload = <piece index><offset><length> (each 4 bytes, big-endian)
  std::vector<unsigned char> payload;

  std::vector<unsigned char> pieceBytes = intToBigEndianBytes(pieceIndex);
  std::vector<unsigned char> offsetBytes = intToBigEndianBytes(offset);
  std::vector<unsigned char> lengthBytes = intToBigEndianBytes(length);

  payload.insert(payload.end(), pieceBytes.begin(), pieceBytes.end());
  payload.insert(payload.end(), offsetBytes.begin(), offsetBytes.end());
  payload.insert(payload.end(), lengthBytes.begin(), lengthBytes.end());

  sendMessage(stream, 13, 8, payload);
}

std::vector<unsigned char>
MessageHelper::readResponse(beast::tcp_stream &stream) {
  NetworkManager nm;
  std::vector<unsigned char> response;
  // read length prefix
  std::vector<unsigned char> length_buffer =
      nm.readFromStream(stream, 4); // bytes to read = 4

  // Parse message length from big-endian 4-byte integer
  uint32_t message_length = 0;
  message_length |= (static_cast<uint32_t>(length_buffer[0]) << 24);
  message_length |= (static_cast<uint32_t>(length_buffer[1]) << 16);
  message_length |= (static_cast<uint32_t>(length_buffer[2]) << 8);
  message_length |= static_cast<uint32_t>(length_buffer[3]);

  // Add length prefix to response
  response.insert(response.end(), length_buffer.begin(), length_buffer.end());

  // Read the actual message content
  std::vector<unsigned char> message_buffer =
      nm.readFromStream(stream, message_length);

  // Add message content to response
  response.insert(response.end(), message_buffer.begin(), message_buffer.end());
  return response;
}

MESSAGE_TYPE
MessageHelper::getResponseType(std::vector<unsigned char> response) {
  // Handle keep-alive message (length = 0)
  if (response.size() < 4) {
    throw std::invalid_argument("Invalid Message: too short");
  }
  
  // Extract length from first 4 bytes (big-endian)
  uint32_t length = (response[0] << 24) | (response[1] << 16) |
                    (response[2] << 8) | response[3];
  
  // Keep-alive message has length 0
  if (length == 0) {
    return KEEP_ALIVE;
  }
  
  // Check if we have enough bytes for the message type
  if (response.size() < 5) {
    throw std::invalid_argument("Invalid Message: missing id");
  }
  
  // Message type is the 5th byte (index 4)
  unsigned char messageId = response[4];
  
  switch (messageId) {
  case 0:
    return CHOKE;
  case 1:
    return UNCHOKE;
  case 2:
    return INTERESTED;
  case 3:
    return NOT_INTERESTED;
  case 4:
    return HAVE;
  case 5:
    return BITFIELD;
  case 6:
    return REQUEST;
  case 7:
    return PIECE;
  case 8:
    return CANCEL;
  default:
    std::cout << "type = unknown (ID: " << static_cast<int>(messageId) << ")\n";
    throw std::invalid_argument("Unknown message type: " +
                                std::to_string(messageId));
  }
}
bool MessageHelper::hasMessage(beast::tcp_stream &stream) {
  return stream.socket().available() > 0;
}
