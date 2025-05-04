#include "peer.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <exception>
#include <iostream>
asio::io_context Peer::ioc;

Peer::Peer()
    : ip(""), port(0), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false) {}

Peer::Peer(const std::string &ip_, const size_t &port_)
    : ip(ip_), port(port_), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false) {}

std::string Peer::getIp() const { return ip; }
size_t Peer::getPort() const { return port; }
bool Peer::getAmChoking() const { return am_choking; }
bool Peer::getAmInterested() const { return am_interested; }
bool Peer::getPeerChocking() const { return peer_choking; }
bool Peer::getInterested() const { return peer_interested; }

// establish tcp connection with peer
void Peer::connect() {
  try {
    // Parse IP address (throws boost::system::system_error if invalid)
    auto const ip_address = asio::ip::make_address(ip);
    tcp::endpoint endpoint(ip_address, port);

    // Connect (throws boost::system::system_error on failure)
    stream.connect(endpoint);
  } catch (const boost::system::system_error &e) {
    std::cerr << "Network error: " << e.what() << std::endl;
    throw; // Re-throw to let the caller handle it
  } catch (const std::exception &e) {
    // Catch other unexpected errors
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    throw;
  }
}
