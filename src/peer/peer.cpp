#include "peer.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <exception>
#include <iostream>
#include <iomanip>
asio::io_context Peer::ioc;

Peer::Peer()
    : ip(""), port(0), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false) {}

Peer::Peer(const std::string &ip_, const size_t &port_)
    : ip(ip_), port(port_), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false), tcp_connected(false) {}

Peer::Peer(const Peer &p)
    : ip(p.ip), port(p.port), am_choking(p.am_choking),
      am_interested(p.am_interested), peer_choking(p.peer_choking),
      peer_interested(p.peer_interested), tcp_connected(p.tcp_connected) {}

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
    tcp_connected = true;
  } catch (const boost::system::system_error &e) {
    std::cerr << "Network error: " << e.what() << std::endl;
    throw; // Re-throw to let the caller handle it
  } catch (const std::exception &e) {
    // Catch other unexpected errors
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    throw;
  }
}

bool Peer::isTcpConnected()const {return tcp_connected;}
// Perform BitTorrent Handshake
void Peer::performBitTorrentHandshake(const std::string &pstr,
                                      const size_t &pstrlen,
                                      const std::string &info_hash,
                                      const std::string &peer_id) {
  // Handshake format: <pstrlen><pstr><reserved><info_hash><peer_id>
  // <pstrlen>: 1 byte
  // <pstr>: string of length pstrlen (usually "BitTorrent protocol")
  // <reserved>: 8 bytes, all zero
  // <info_hash>: 20 bytes
  // <peer_id>: 20 bytes

  if (info_hash.length() != 20) {
    throw std::invalid_argument("Info hash must be 20 bytes long.");
  }
  if (peer_id.length() != 20) {
    throw std::invalid_argument("Peer ID must be 20 bytes long.");
  }
  if (pstr.length() != pstrlen) {
    throw std::invalid_argument(
        "pstrlen does not match actual length of pstr.");
  }

  // Total handshake length = 1 + pstrlen + 8 + 20 + 20
  size_t handshake_len = 1 + pstrlen + 8 + 20 + 20;
  std::vector<unsigned char> handshake_msg(handshake_len);

  size_t offset = 0;

  // 1. pstrlen
  handshake_msg[offset++] = static_cast<unsigned char>(pstrlen);

  // 2. pstr
  std::copy(pstr.begin(), pstr.end(), handshake_msg.begin() + offset);
  offset += pstrlen;

  // 3. reserved bytes (8 zeros)
  std::fill(handshake_msg.begin() + offset, handshake_msg.begin() + offset + 8,
            0);
  offset += 8;

  // 4. info_hash (20 bytes)
  std::copy(info_hash.begin(), info_hash.end(), handshake_msg.begin() + offset);
  offset += 20;

  // 5. peer_id (20 bytes)
  std::copy(peer_id.begin(), peer_id.end(), handshake_msg.begin() + offset);

  try {
    // Write the handshake message to the stream
    asio::write(stream, asio::buffer(handshake_msg));

    // -- -Read the peer's handshake response ---
    std::cout << "Waiting for handshake response from" << ip << "..." << std::endl;
    std::vector<unsigned char> response_buffer(
        handshake_len); // Response should have the same length

    // Read exactly handshake_len bytes
    size_t bytes_read = asio::read(stream, asio::buffer(response_buffer),
                                   asio::transfer_exactly(handshake_len));

    if (bytes_read != handshake_len) {
      // This case should ideally not happen with transfer_exactly unless an
      // error occurs before completion
      std::cerr
          << "Error: Did not receive complete handshake response. Expected "
          << handshake_len << " bytes, got " << bytes_read << std::endl;
      // Decide how to handle incomplete read (e.g., throw, return error code)
      throw std::runtime_error("Incomplete handshake response received.");
    }

    std::cout << "Received handshake response (" << bytes_read
              << " bytes):" << std::endl;

    // Print the received response bytes (e.g., in hex)
    for (size_t i = 0; i < response_buffer.size(); ++i) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(response_buffer[i]) << " ";
      if ((i + 1) % 16 == 0) { // Print 16 bytes per line
        std::cout << std::endl;
      }
    }
    std::cout << std::dec
              << std::endl; // Reset formatting and add final newline

  } catch (const boost::system::system_error &e) {
    std::cerr << "Network error during handshake write: " << e.what() << " ("
              << ip << ":" << port << ")" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Unexpected error during handshake write: " << e.what() << " ("
              << ip << ":" << port << ")" << std::endl;
  }
}
