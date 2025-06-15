#include "peer.hpp"
#include "../torrent/torrent.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

Peer::Peer(asio::io_context &ioc)
    : ip(""), port(0), am_choking(true), am_interested(false),
      peer_choking(true), peer_interested(false), state(NOT_CONNECTED),
      stream(std::make_unique<beast::tcp_stream>(ioc)) {}

Peer::Peer(asio::io_context &ioc, const std::string &ip_, const size_t &port_)
    : ip(ip_), port(port_), am_choking(true), am_interested(false),
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
void Peer::connectWithRetries(
    size_t retries_left, const std::string &info_hash,
    const std::function<void(CONNECTION_STATE state)> callback) {
  try {
    // Parse IP address (throws boost::system::system_error if invalid)
    auto const ip_address = asio::ip::make_address(ip);
    tcp::endpoint endpoint(ip_address, port);

    // make a shared pointer of this object
    // to extend it's lifetime
    // and ensure it is valid during async operation
    auto self = shared_from_this();
    stream->async_connect(
        endpoint, [this, self, retries_left, callback,
                   info_hash](const boost::system::error_code &error) {
          // Check if operation was cancelled
          if (error == boost::asio::error::operation_aborted) {
            state = FAILED;
            if (callback) {
              callback(FAILED);
            }
            return;
          }

          if (!error) {
            state = CONNECTED;
            if (callback) {
              callback(CONNECTED);
            }
            performBitTorrentHandshake(info_hash, callback);
            return;
          }

          if (retries_left > 0) {
            // wait a tad before retrying
            // that's basic etiquette
            // maybe the other peer is busy who knows
            auto timer = std::make_shared<asio::steady_timer>(
                stream->get_executor(), std::chrono::milliseconds(500));
            timer->async_wait(
                [this, self, retries_left, info_hash,
                 callback](const boost::system::error_code &timer_error) {
                  // Check if timer was cancelled
                  if (timer_error == boost::asio::error::operation_aborted) {
                    state = FAILED;
                    if (callback) {
                      callback(FAILED);
                    }
                    return;
                  }

                  if (!timer_error) {
                    connectWithRetries(retries_left - 1, info_hash, callback);
                  }
                });
          } else {
            state = FAILED;
            if (callback) {
              callback(FAILED);
            }
          }
        });
  } catch (const boost::system::system_error &e) {
    state = FAILED;
    if (callback) {
      callback(FAILED);
    }
  } catch (const std::exception &e) {
    state = FAILED;
    if (callback) {
      callback(FAILED);
    }
  }
}

// Perform BitTorrent Handshake
void Peer::performBitTorrentHandshake(
    const std::string &info_hash,
    std::function<void(CONNECTION_STATE)> callback) {
  size_t handshake_len;
  auto handshake_msg = std::make_shared<std::vector<unsigned char>>(
      makeHandshakeMessage(info_hash, handshake_len));

  auto self = shared_from_this();

  // Write the handshake message to the stream
  asio::async_write(
      *(self->stream), asio::buffer(*handshake_msg),
      [self, callback, handshake_len, handshake_msg, info_hash](
          const boost::system::error_code &error, size_t bytes_received) {
        // Check for cancellation
        if (error == boost::asio::error::operation_aborted) {
          self->state = HANDSHAKE_FAILED;
          if (callback)
            callback(HANDSHAKE_FAILED);
          return;
        }

        if (error) {
          self->state = HANDSHAKE_FAILED;
          if (callback)
            callback(HANDSHAKE_FAILED);
          return;
        }

        // Read the peer's handshake response
        auto response_buffer_ptr =
            std::make_shared<std::vector<unsigned char>>(handshake_len);

        asio::async_read(
            *(self->stream), asio::buffer(*response_buffer_ptr),
            [self, info_hash, response_buffer_ptr, callback](
                const boost::system::error_code &error, size_t bytes_received) {
              // Check for cancellation
              if (error == boost::asio::error::operation_aborted) {
                self->state = HANDSHAKE_FAILED;
                if (callback) {
                  callback(HANDSHAKE_FAILED);
                }
                return;
              }

              if (error) {
                self->state = HANDSHAKE_FAILED;
                if (callback) {
                  callback(HANDSHAKE_FAILED);
                }
              } else {
                std::string response(response_buffer_ptr->begin(),
                                     response_buffer_ptr->end());
                std::string received_info_hash(
                    response_buffer_ptr->begin() + 1 + 19 + 8,
                    response_buffer_ptr->begin() + 1 + 19 + 8 + 20);
                if (received_info_hash != info_hash) {
                  self->state = HANDSHAKE_FAILED;
                  if (callback)
                    callback(HANDSHAKE_FAILED);
                } else {
                  self->state = HANDSHAKE_COMPLETE;
                  if (callback)
                    callback(HANDSHAKE_COMPLETE);
                }
              }
            });
      });
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

void Peer::closeConnection(){
  stream->close();
  state = NOT_CONNECTED;
}
