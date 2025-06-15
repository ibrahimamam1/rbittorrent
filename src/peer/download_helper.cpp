#include "download_helper.hpp"
#include "../helpers/helpers.hpp" // for intToBigEndian conversion
#include "../message/message_helper.hpp"
#include "peer.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <cstdint>
#include <iostream>
#include <vector>

DownloadHelper::DownloadHelper(PeerList &peers_, const size_t total_size_,
                               const size_t number_of_pieces_)
    : peers(peers_), total_size(total_size_),
      number_of_pieces(number_of_pieces_), download_complete(false),
      num_active_peers(0), my_bitfield(number_of_pieces_, 0) {}

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

      // Validate minimum message size (4 bytes length + 1 byte type)
      if (response.size() < 5) {
        std::cout << "Invalid message size. Dropping connection.\n";
        peer->closeConnection();
        continue; // Process other peers
      }

      MESSAGE_TYPE type = mh.getResponseType(response);
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
          continue;
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
          continue; // Process other peers
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

        if (peerHasNeededPiece(*peer)) {
          mh.sendAmInterestedMessage(*peer->getStream());
        }
        break;
      }
      case INTERESTED:
        // Handle interested message
          // TODO: potentially unchock peer
        break;
      case UNCHOKE:{
          std::cout << peer->getIp() << " unchoked us\n";
          if(num_active_peers < 4 ){
            active_peers.push_back(peer);
            int piece_index = findBestPiece(peer->getBitfield());
            int piece_len = total_size/number_of_pieces;
            mh.sendRequestMessage(*peer->getStream(), piece_index, 0, piece_len);
          }
          break;
        }
      case PIECE:{
          std::cout << "Hell yeah we received a piece\n";
        }
      default:
        break;
      }
    }
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
        if (bit_field[i] == 0) continue;
        
        // Count how many peers have this piece
        int count = 0;
        for (const auto& peer : peers) {
            const auto& peer_bitfield = peer->getBitfield();
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
