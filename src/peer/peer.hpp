#pragma once

#include "../network/network_manager.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace beast = boost::beast;
namespace asio = boost::asio;

enum CONNECTION_STATE{FAILED, NOT_CONNECTED, CONNECTED, HANDSHAKE_COMPLETE, HANDSHAKE_FAILED,TRANSFERING, WAITING};

class Peer : public std::enable_shared_from_this<Peer> {
  std::string ip;
  size_t port;
  bool am_choking;
  bool am_interested;
  bool peer_choking;
  bool peer_interested;
  CONNECTION_STATE state;
  std::unique_ptr<beast::tcp_stream> stream;// tcp socket with peer
  std::vector<int>bit_field;

public:
  Peer(asio::io_context& ioc);
  Peer(asio::io_context& ioc, const std::string &ip_, const size_t &port_);
  Peer(Peer&& other) noexcept; // noexcept is important for vector optimizations
  Peer& operator=(Peer&& other) noexcept;
  std::string getIp() const;
  size_t getPort() const;
  bool getAmChoking() const;
  bool getAmInterested() const;
  bool getPeerChocking() const;
  bool getInterested() const;
  std::vector<int> getBitfield() const {return bit_field; }
  bool hasPiece(const size_t idx) const{ return bit_field[idx]; }
  beast::tcp_stream* getStream() const{ return stream.get(); }
  void setAmInterested(bool value){ am_interested = value;}
  void setPeerInterested(bool value){ peer_interested = value;}
  void setAmChoking(bool value){ am_choking = value;}
  void setPeerChoking(bool value){ peer_choking = value;}
  void setState(const CONNECTION_STATE state_){ state = state_; }
  void setHasPiece(const size_t idx, const size_t value) {bit_field[idx] = value;}
  void initBitfield(const size_t number_of_pieces){bit_field.resize(number_of_pieces);}
  void closeConnection();
  void connectWithRetries(size_t retries_left,
                          const std::string& info_hash,
                          const std::function<void(CONNECTION_STATE)> callback); // establish tcp connection with peer
  void performBitTorrentHandshake(const std::string& info_hash, const std::function<void(CONNECTION_STATE)>callback);
  std::vector<unsigned char> makeHandshakeMessage(const std::string& info_hash, size_t& handshake_len);
  CONNECTION_STATE getState()const;
};
