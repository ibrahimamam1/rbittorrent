#include "download_helper.hpp"
#include "../helpers/helpers.hpp" // for intToBigEndian conversion
#include "../message/message_helper.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <cstdint>
#include <iostream>
#include <vector>

DownloadHelper::DownloadHelper(PeerList &peers_, const size_t total_size_,
                               const size_t number_of_pieces_)
    : peers(peers_), total_size(total_size_),
      number_of_pieces(number_of_pieces_) {}

void DownloadHelper::startDownloadLoop() {
  std::vector<size_t> piece_rarity = extractBitfieldInformation();
  std::cout << piece_rarity[10] << std::endl;
}

std::vector<size_t> DownloadHelper::extractBitfieldInformation() {
  std::cout << "Extracting Peer Bitfield information...\n";
  NetworkManager nm;
  std::vector<size_t> piece_rarity(number_of_pieces, 0);

  for (auto &peer : peers) {
    // read their bitfield send after handshake
    auto stream = peer->getStream();
    std::vector<unsigned char> res = nm.readFromStream(*stream);

    if (res.size() >= 5) { // bitfield message
      uint8_t message_id = res[4];
      if (message_id == 5) {
        // Bitfield starts at byte 5
        size_t bitfield_length = res.size() - 5; //
        for (size_t byte_index = 0; byte_index < bitfield_length;
             byte_index++) {
          uint8_t byte_value = res[5 + byte_index];

          // process each bit in the byte(MSB first)
          for (int bit_index = 7; bit_index >= 0; bit_index--) {
            size_t piece_index = (byte_index * 8) + (7 - bit_index);

            if (piece_index < number_of_pieces &&
                (byte_value >> bit_index) & 1) {
              piece_rarity[piece_index]++;
            }
          }
        }
      }
    }
  }
  return piece_rarity;
}
