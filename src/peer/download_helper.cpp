#include "download_helper.hpp"
#include "../helpers/helpers.hpp" // for intToBigEndian conversion
#include <boost/beast/core/tcp_stream.hpp>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

void DownloadHelper::startDownloadLoop(PeerList &peers) {
  NetworkManager nm;
  std::cout << "Will try to read bitfield sent by each peer after handshake\n";
  for(auto& peer : peers){
    auto stream = peer->getStream();
    std::vector<unsigned char>res = nm.readFromStream(*stream);

    std::string str(res.begin(), res.end());
    std::cout << str << std::endl;
  }
}

// makes a bittorrent
void DownloadHelper::sendMessage(beast::tcp_stream& stream,
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
  std::vector<unsigned char> response = nm.writeToStream(stream, data);

  std::string str(response.begin(), response.end());
  std::cout << str << std::endl;
}

void DownloadHelper::sendKeepAliveMessage(beast::tcp_stream &stream) {
  // length prefix = 0000
  // no messgae id
  // no payload
  sendMessage(stream, 0, 0);
}

void DownloadHelper::sendChokeMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 1
  // no payload
  sendMessage(stream, 1, 0);
}

void DownloadHelper::sendUnchokeMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 1
  // no payload
  sendMessage(stream, 0, 1);
}

void DownloadHelper::sendAmInterestedMessage(beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 2
  // no payload
  sendMessage(stream, 1, 2);
}

void DownloadHelper::sendAmNotInterestedMessage(
    beast::tcp_stream &stream) {
  // length prefix = 0001
  // id = 3
  // no payload
  sendMessage(stream, 1, 3);
}

void DownloadHelper::sendHaveMessage(beast::tcp_stream &stream,
                                     const uint32_t piece_index) {
  // length prefix = 0005
  // id = 4
  // payload = zero based piece index
  std::vector<unsigned char> payload = intToBigEndianBytes(piece_index);
  sendMessage(stream, 5, 4, payload);
}

void DownloadHelper::sendRequestMessage(beast::tcp_stream &stream,
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

void DownloadHelper::sendPiece(beast::tcp_stream &stream,
                               const uint32_t piece_index,
                               const uint32_t offset,
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

void DownloadHelper::sendCancelMessage(beast::tcp_stream &stream,
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
