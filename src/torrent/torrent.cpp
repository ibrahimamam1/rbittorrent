#include "torrent.hpp"
#include "../bencode/decode.hpp"
#include "../bencode/encode.hpp"
#include "../helpers/helpers.hpp"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#define LISTEN_PORT "6123"

using json = nlohmann::json;
namespace http = boost::beast::http;

bool TorrentHelper::parseTorrentFile(std::string filepath) {
  std::ifstream file(filepath, std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file " + filepath);
  }

  // find file size
  auto size = file.tellg();
  file.seekg(0);

  // read entire file
  std::string content;
  content.resize(size);
  file.read(&content[0], size);

  try {
    json decoded = BencodeDecoder::decode_bencoded_value(content);
    // extract top level information
    if (decoded.contains("announce"))
      tracker_url = decoded["announce"];
    else
      throw std::runtime_error("Invalid Torrent File: Missing announce field");

    json infoDictionary;
    if (decoded.contains("info"))
      infoDictionary = decoded["info"];
    else
      throw std::runtime_error("Invalid Torrent File: Missing info dictionary");

    this->info =
        infoDictionary; // will be used to calculate info hash in get request

    // extract info dictinary content
    if (infoDictionary.contains("piece length"))
      piece_length = infoDictionary["piece length"];
    else
      throw std::runtime_error(
          "Invalid Torrent File: Missing piece length field");

    if (infoDictionary.contains("name"))
      name = infoDictionary["name"];
    else
      throw std::runtime_error("Invalid Torrent File: Missing name field");

    if (infoDictionary.contains("pieces"))
      pieces = infoDictionary["pieces"];
    else
      throw std::runtime_error("Invalid Torrent File: Missing pieces field");

    if (infoDictionary.contains("length")) {
      length = infoDictionary["length"];
    }

  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

PeerList TorrentHelper::getPeers(NetworkManager &nw) {
  // Compute SHA1 of info hash
  BencodeValue val = json_to_bencode(info);
  std::string encoded = BencodeEncoder::encode(val);

  std::vector<unsigned char> sha_hash = compute_sha1(encoded);
  std::string raw_hash_string(sha_hash.begin(), sha_hash.end());

  // prepare parameters
  parameterList params;
  params.push_back({"info_hash", raw_hash_string});
  params.push_back({"peer_id", generatePeerId()});
  params.push_back({"port", LISTEN_PORT});
  params.push_back({"uploaded", "0"});
  params.push_back({"downloaded", "0"});

  // make request
  try {
    http::response<http::dynamic_body> res =
        nw.makeGetRequest(tracker_url, params);

    if (res.result_int() != 200)
      return PeerList();
    
    // Extract the response body as a string
    std::string response_body = boost::beast::buffers_to_string(res.body().data());
    
    // decode bencoded response
    json peer_data = BencodeDecoder::decode_bencoded_value(response_body);

    //retreive interval
    if(!peer_data.contains("interval"))
      interval = 0;
    else interval = static_cast<size_t>(peer_data["interval"]);
    
    //retreive peer ip and port
    if(!peer_data.contains("peers")){
      return PeerList();
    }

    //serialise peers from json to PeerList
    json peers = peer_data["peers"];
    PeerList return_value;
    for(auto peer : peers){
      std::string ip = peer["ip"];
      size_t port = static_cast<size_t>(peer["port"]);
      Peer p{ip, port};
      return_value.push_back(p);
    }
    
    return return_value;
  } catch (const std::exception &e) {
    std::cerr << e.what();
    return PeerList();
  }
}

std::string TorrentHelper::generatePeerId() {
  return "rgb25v1.0-0123456789"; //<rgb><release_year><v><version_number><-0123456789>
}

size_t TorrentHelper::getInterval() const { return interval; }
