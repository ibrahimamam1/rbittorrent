#include "download_helper.hpp"
#include "../helpers/helpers.hpp" // for intToBigEndian conversion
#include "../message/message_helper.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <cstdint>
#include <vector>

DownloadHelper::DownloadHelper(PeerList &peers_, const size_t total_size_,
                               const size_t number_of_pieces_)
    : peers(peers_), total_size(total_size_),
      number_of_pieces(number_of_pieces_), download_complete(false),
      active_peers(0) {}

void DownloadHelper::startDownloadLoop() {

  while (!download_complete) {
    handlePeerMessages(); // read and process peer messages in an async manner
  }
}

void DownloadHelper::handlePeerMessages() {
  MessageHelper mh;
  for (auto &peer : peers) {
    if (mh.hasMessage(*peer->getStream())) {
      std::vector<unsigned char> response = mh.readResponse(*peer->getStream());
      MESSAGE_TYPE type = mh.getResponseType(response);

      switch (type) {
      case BITFIELD:
        // convert length of bitfield message from big endian to int
        size_t payload_len = (response[0] << 24) | (response[1] << 16) |
                             (response[2] << 8) | response[3];
        size_t bitfield_len = payload_len - 1;
        if (bitfield_len != number_of_pieces) {
          std::cout << "Invalid Bitfield length. Dropping Connection.\n";
          peer->closeConnection();
          return;
        }

        // read the bitfield of size len
        for (int byte = 5; byte < payload_len; byte++) {
          // start reading from msb
          for (int bit = 7; bit >= 0; bit--) {
            size_t piece_index = (byte - 5) * 8 + (7 - bit);
            bool has_piece = (response[byte] >> bit) & 1;
            peer->setHasPiece(
                piece_index,
                has_piece); // sets the value in the bitfield structure
          }
        }
        if (peerHasPieceINeed(peer))
          mh.sendAmInterestedMessage(*peer->getStream());
        break;
      case INTERESTED:
        break;
      }
    }
  }
}
