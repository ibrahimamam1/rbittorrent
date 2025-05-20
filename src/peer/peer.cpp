#include "peer.hpp"
#include "../torrent/torrent.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

Peer::Peer(asio::io_context &ioc)
    : ip(""), port(0), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false), state(NOT_CONNECTED),
      stream(std::make_unique<beast::tcp_stream>(ioc)) {}

Peer::Peer(asio::io_context &ioc, const std::string &ip_, const size_t &port_)
    : ip(ip_), port(port_), am_choking(false), am_interested(false),
      peer_choking(true), peer_interested(false), state(NOT_CONNECTED),
      stream(std::make_unique<beast::tcp_stream>(ioc)) {}

// move constructor
Peer::Peer(Peer &&other) noexcept
    : ip(std::move(other.ip)), port(other.port), am_choking(other.am_choking),
      am_interested(other.am_interested), peer_choking(other.peer_choking),
      peer_interested(other.peer_interested), state(other.state),
      stream(std::move(other.stream)) {
  other.port = 0;
  other.state = CONNECTION_STATE::NOT_CONNECTED;
}

Peer &Peer::operator=(Peer &&other) noexcept {
  if (this != &other) {
    ip = std::move(other.ip);
    port = other.port;
    am_choking = other.am_choking;
    am_interested = other.am_interested;
    peer_choking = other.peer_choking;
    peer_interested = other.peer_interested;
    state = other.state;
    stream = std::move(other.stream);

    other.port = 0;
    other.state = CONNECTION_STATE::FAILED;
  }
  return *this;
}

std::string Peer::getIp() const { return ip; }
size_t Peer::getPort() const { return port; }
bool Peer::getAmChoking() const { return am_choking; }
bool Peer::getAmInterested() const { return am_interested; }
bool Peer::getPeerChocking() const { return peer_choking; }
bool Peer::getInterested() const { return peer_interested; }
CONNECTION_STATE Peer::getState() const { return state; }

// establish tcp connection with peer
void Peer::connectWithRetries(size_t retries_left,
                              const std::function<void()> onComplete) {
  try {
    // Parse IP address (throws boost::system::system_error if invalid)
    auto const ip_address = asio::ip::make_address(ip);
    tcp::endpoint endpoint(ip_address, port);
    // Connect (throws boost::system::system_error on failure)
    stream->async_connect(endpoint,
                          [&](const boost::system::error_code &error) {
                            if (!error) {
                              state = CONNECTED;
                              onComplete();
                              return;
                            }
                            if (retries_left > 0)
                              connectWithRetries(retries_left - 1, onComplete);
                          });
  } catch (const boost::system::system_error &e) {
    state = FAILED;
  } catch (const std::exception &e) {
    state = FAILED;
  }
}

// Perform BitTorrent Handshake
void Peer::performBitTorrentHandshake(const std::string &info_hash) {
  size_t handshake_len;
  std::vector<unsigned char> handshake_msg =
      makeHandshakeMessage(info_hash, handshake_len);

  try {
    // Write the handshake message to the stream
    asio::write(*stream, asio::buffer(handshake_msg));

    // -- -Read the peer's handshake response ---
    std::vector<unsigned char> response_buffer(
        handshake_len); // Response should have the same length

    // Read exactly handshake_len bytes
    size_t bytes_read = asio::read(*stream, asio::buffer(response_buffer),
                                   asio::transfer_exactly(handshake_len));
    if (bytes_read != handshake_len) {
      // This case should ideally not happen with transfer_exactly unless an
      // error occurs before completion
      std::cerr
          << "Error: Did not receive complete handshake response. Expected "
          << handshake_len << " bytes, got " << bytes_read << std::endl;
      // Decide how to handle incomplete read (e.g., throw, return error code)
      throw std::runtime_error("Incomplete handshake response received.");
      state = HANDSHAKE_FAILED;
    }
    std::string received_info_hash(response_buffer.begin() + 1 + 19 + 8,
                                   response_buffer.begin() + 1 + 19 + 8 + 20);
    if (received_info_hash != info_hash) {
      state = HANDSHAKE_FAILED;
    } else {
      state = HANDSHAKE_COMPLETE;
    }
  } catch (const boost::system::system_error &e) {
    std::cerr << "Network error during handshake write: " << e.what() << " ("
              << ip << ":" << port << ")" << std::endl;
    state = HANDSHAKE_FAILED;
  } catch (const std::exception &e) {
    std::cerr << "Unexpected error during handshake write: " << e.what() << " ("
              << ip << ":" << port << ")" << std::endl;
    state = HANDSHAKE_FAILED;
  }
}

std::vector<unsigned char>
Peer::makeHandshakeMessage(const std::string &info_hash,
                           size_t &handshake_len) {
  // Handshake format: <pstrlen><pstr><reserved><info_hash><peer_id>
  // <pstrlen>: 1 byte
  // <pstr>: string of length pstrlen (usually "BitTorrent protocol")
  // <reserved>: 8 bytes, all zero
  // <info_hash>: 20 bytes
  // <peer_id>: 20 bytes

  std::string pstr = "BitTorrent protocol";
  size_t pstrlen = pstr.length();
  std::string peer_id = PEER_ID;

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
  handshake_len = 1 + pstrlen + 8 + 20 + 20;
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
  return handshake_msg;
}
