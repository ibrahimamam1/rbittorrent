#include "../network/network_manager.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace asio = boost::asio;

class Peer {
  std::string ip;
  size_t port;
  bool am_choking;
  bool am_interested;
  bool peer_choking;
  bool peer_interested;
  bool tcp_connected;
  static asio::io_context ioc;
  beast::tcp_stream stream{ioc}; // tcp socket with peer

public:
  Peer();
  Peer(const std::string &ip_, const size_t &port_);
  Peer(const Peer& p);

  std::string getIp() const;
  size_t getPort() const;
  bool getAmChoking() const;
  bool getAmInterested() const;
  bool getPeerChocking() const;
  bool getInterested() const;

  void connect(); // establish tcp connection with peer
  void performBitTorrentHandshake(const std::string& pstr, const size_t& pstrlen,
                                  const std::string& peer_id,
                                  const std::string& info_hash);
  bool isTcpConnected()const;
};
