#include "bencode/decode.hpp"
#include "torrent/torrent.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
    return 1;
  }

  std::string command = argv[1];

  if (command == "decode") {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " decode <encoded_value>"
                << std::endl;
      return 1;
    }

    std::string encoded_value = argv[2];
    std::string decoded_value = decode_bencoded_value(encoded_value);
    std::cout << decoded_value << std::endl;
  } else if (command == "torrent") {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " torrent <filepath>" << std::endl;
      return 1;
    }
    Torrent torrent =
        parse_torrent_file("/home/rgb/Desktop/projects/"
                           "codecrafters-bittorrent-cpp/sample.torrent");
    std::cout << "tracker: " << torrent.tracker_url << std::endl;
    std::cout << "name: " << torrent.name << std::endl;
    std::cout << "pieces: " << torrent.pieces << std::endl;
    std::cout << "piece_length: " << torrent.piece_length << std::endl;
    std::cout << "length: " << torrent.length << std::endl;
  } else {
    std::cerr << "unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
