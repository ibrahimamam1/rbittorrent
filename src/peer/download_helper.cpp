#include "download_helper.hpp"
#include "../helpers/helpers.hpp" // for intToBigEndian conversion
#include "../message/message_helper.hpp"
#include "peer.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <iostream>
#include <memory>
#include <vector>

DownloadHelper::DownloadHelper(PeerList &peers_, const size_t total_size_,
                               const size_t number_of_pieces_)
    : peers(peers_), total_size(total_size_),
      number_of_pieces(number_of_pieces_), download_complete(false),
      my_bitfield(number_of_pieces_, 0) {}

void DownloadHelper::startDownloadLoop() {
  MessageHelper mh;
  asio::io_context ioc;
  for (auto peer : peers) {
    mh.readMessage(peer, [this, &peer](MESSAGE_TYPE type,
                                       std::vector<unsigned char> &response) {
      handlePeerMessage(peer, type, response);
    });
  }
  ioc.run();
}

void DownloadHelper::handlePeerMessage(std::shared_ptr<Peer> &peer,
                                       MESSAGE_TYPE type,
                                       std::vector<unsigned char> &response) {
  MessageHelper mh;

  switch (type) {
  case BITFIELD: {
    peer->initBitfield(number_of_pieces);
    // Convert length of bitfield message from big endian to int
    size_t payload_len = (response[0] << 24) | (response[1] << 16) |
                         (response[2] << 8) | response[3];

    // Validate message size matches declared payload length
    if (response.size() < payload_len + 4) {
      std::cout << "Message size doesn't match payload length. Dropping "
                   "connection.\n";
      peer->closeConnection();
    }

    size_t bitfield_len = payload_len - 1;

    // Calculate expected bitfield length in bytes
    size_t expected_bitfield_bytes =
        (number_of_pieces + 7) / 8; // Ceiling division

    if (bitfield_len != expected_bitfield_bytes) {
      std::cout << "Invalid Bitfield length. Expected: "
                << expected_bitfield_bytes << ", Got: " << bitfield_len
                << ". Dropping Connection.\n";
      peer->closeConnection();
    }

    // Read the bitfield
    for (size_t byte = 5; byte < payload_len + 4; byte++) {
      // Start reading from MSB
      for (int bit = 7; bit >= 0; bit--) {
        size_t piece_index = (byte - 5) * 8 + (7 - bit);

        // Shouldn't process bits beyond the actual number of pieces
        if (piece_index >= number_of_pieces) {
          break;
        }

        bool has_piece = (response[byte] >> bit) & 1;
        peer->setHasPiece(piece_index, has_piece);
      }

      // Break if we've processed all pieces
      if ((byte - 5 + 1) * 8 >= number_of_pieces) {
        break;
      }
    }

    if (active_peers.size() < 4 && peerHasNeededPiece(*peer)) {
      mh.sendAmInterestedMessage(*peer->getStream());
    }
    break;
  }
  case INTERESTED:
    // Handle interested message
    // TODO: potentially unchock peer
    break;
  case UNCHOKE: {
    std::cout << peer->getIp() << " unchoked us\n";
    if (active_peers.size() < 4) {
      active_peers.push_back(peer);
      int piece_index = findBestPiece(peer->getBitfield());
      int piece_len = total_size / number_of_pieces;
      mh.sendRequestMessage(*peer->getStream(), piece_index, 0, piece_len);
    }
    break;
  }
  case PIECE: {
    std::cout << "Hell yeah we received a piece\n";
  }
  default:
    break;
  }
}

bool DownloadHelper::peerHasNeededPiece(Peer &peer) {
  for (size_t i = 0; i < number_of_pieces; i++) {
    if (peer.hasPiece(i) && my_bitfield[i] == 0) {
      return true;
    }
  }
  return false;
}

int DownloadHelper::findBestPiece(std::vector<int> bit_field) {
  int best_piece = -1;
  int min_count = INT_MAX;

  for (size_t i = 0; i < bit_field.size(); ++i) {
    if (bit_field[i] == 0)
      continue;

    // Count how many peers have this piece
    int count = 0;
    for (const auto &peer : peers) {
      const auto &peer_bitfield = peer->getBitfield();
      if (i < peer_bitfield.size() && peer_bitfield[i] == 1) {
        count++;
      }
    }

    // Update best piece if this one is rarer
    if (count < min_count) {
      min_count = count;
      best_piece = static_cast<int>(i);
    }
  }

  return best_piece;
}
