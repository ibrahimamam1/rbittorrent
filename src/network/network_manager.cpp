#include "network_manager.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/url.hpp>
#include <boost/url/encode.hpp>
#include <iostream>

// perforsm tcp hanshake with target
// returns stream object representing the connection

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

http::response<http::dynamic_body>
NetworkManager::makeGetRequest(const std::string &uri, parameterList params) {
  http::response<http::dynamic_body> res;
  try {
    // Parse URI using Boost.URL
    boost::urls::url_view parsed_url(uri);

    // prepare query string from parameters
    std::string query_string;
    if (!params.empty()) {
      std::vector<std::string> query_parts;
      for (const auto &param : params) {
        // URL-encode key and value
        std::string encoded_key =
            boost::urls::encode(param.first, boost::urls::pchars);
        std::string encoded_value =
            boost::urls::encode(param.second, boost::urls::pchars);
        query_parts.push_back(encoded_key + "=" + encoded_value);
      }
      // Join query parts with '&'
      query_string = "?" + boost::algorithm::join(query_parts, "&");
    }

    // Combine path and query string
    std::string target = parsed_url.path();
    if (!query_string.empty()) {
      target += query_string;
    }

    // Resolve DNS
    auto const results = dns_resolver.resolve(
        parsed_url.host(), parsed_url.has_port() ? parsed_url.port() : "80");

    // Connect to endpoint
    stream.connect(results, ec);
    if (ec) {
      throw boost::beast::system_error{ec};
    }

    // Create and configure HTTP request
    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, parsed_url.host());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::connection, "close");

    // Send request
    http::write(stream, req, ec);
    if (ec) {
      throw boost::beast::system_error{ec};
    }
    // Receive response
    boost::beast::flat_buffer buffer;
    http::read(stream, buffer, res, ec);
    if (ec) {
      std::cerr << "Error reading response: " << ec.message() << std::endl;
      std::cerr << "Response status: " << res.result() << std::endl;
      std::cerr << "Response headers: " << res.base() << std::endl;
      throw boost::beast::system_error{ec};
    }

    // Shutdown connection
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != boost::beast::errc::not_connected) {
      throw boost::beast::system_error{ec};
    }
  } catch (const std::exception &e) {
    throw; // Re-throw the exception
  }

  return res;
}
std::vector<unsigned char>
NetworkManager::writeToStream(beast::tcp_stream &stream_,
                              const std::vector<unsigned char> &data) {

  boost::system::error_code ec;
  std::vector<unsigned char> response;

  // Write data to stream
  std::size_t bytes_written =
      boost::asio::write(stream_, boost::asio::buffer(data), ec);

  if (ec) {
    std::cerr << "Error writing to stream: " << ec.message() << std::endl;
    return response; // Return empty response on write error
  }

  std::cout << "Successfully wrote " << bytes_written << " bytes to stream"
            << std::endl;

  // Read response from stream
  // First, try to read the length prefix (4 bytes for BitTorrent messages)
  std::vector<unsigned char> length_buffer(4);
  std::size_t bytes_read =
      boost::asio::read(stream_, boost::asio::buffer(length_buffer), ec);

  if (ec) {
    if (ec == boost::asio::error::eof) {
      std::cout << "Connection closed by peer" << std::endl;
    } else {
      std::cerr << "Error reading length prefix: " << ec.message() << std::endl;
    }
    return response; // Return empty response on read error
  }

  // Parse message length from big-endian 4-byte integer
  uint32_t message_length = 0;
  message_length |= (static_cast<uint32_t>(length_buffer[0]) << 24);
  message_length |= (static_cast<uint32_t>(length_buffer[1]) << 16);
  message_length |= (static_cast<uint32_t>(length_buffer[2]) << 8);
  message_length |= static_cast<uint32_t>(length_buffer[3]);

  std::cout << "Expected message length: " << message_length << " bytes"
            << std::endl;

  // Add length prefix to response
  response.insert(response.end(), length_buffer.begin(), length_buffer.end());

  // If message_length is 0, it's a keep-alive message (no additional data)
  if (message_length == 0) {
    std::cout << "Received keep-alive message" << std::endl;
    return response;
  }

  // Read the actual message content
  std::vector<unsigned char> message_buffer(message_length);
  bytes_read =
      boost::asio::read(stream_, boost::asio::buffer(message_buffer), ec);

  if (ec) {
    if (ec == boost::asio::error::eof) {
      std::cout << "Connection closed while reading message body" << std::endl;
    } else {
      std::cerr << "Error reading message body: " << ec.message() << std::endl;
    }
    return response; // Return partial response (just length prefix)
  }

  // Add message content to response
  response.insert(response.end(), message_buffer.begin(), message_buffer.end());

  std::cout << "Successfully read " << bytes_read << " bytes of message content"
            << std::endl;
  std::cout << "Total response size: " << response.size() << " bytes"
            << std::endl;

  return response;
}

// Alternative method for cases where you don't expect a response
void NetworkManager::writeToStreamNoResponse(
    beast::tcp_stream &stream_, const std::vector<unsigned char> &data) {

  boost::system::error_code ec;

  // Write data to stream
  std::size_t bytes_written =
      boost::asio::write(stream_, boost::asio::buffer(data), ec);

  if (ec) {
    std::cerr << "Error writing to stream: " << ec.message() << std::endl;
    return;
  }

  std::cout << "Successfully wrote " << bytes_written << " bytes to stream"
            << std::endl;
}

// Method to read a single message from stream
std::vector<unsigned char>
NetworkManager::readFromStream(beast::tcp_stream &stream_) {
  boost::system::error_code ec;
  std::vector<unsigned char> response;

  // Read the length prefix (4 bytes for BitTorrent messages)
  std::vector<unsigned char> length_buffer(4);
  std::size_t bytes_read =
      boost::asio::read(stream_, boost::asio::buffer(length_buffer), ec);

  if (ec) {
    if (ec == boost::asio::error::eof) {
      std::cout << "Connection closed by peer" << std::endl;
    } else {
      std::cerr << "Error reading length prefix: " << ec.message() << std::endl;
    }
    return response; // Return empty response on read error
  }

  // Parse message length from big-endian 4-byte integer
  uint32_t message_length = 0;
  message_length |= (static_cast<uint32_t>(length_buffer[0]) << 24);
  message_length |= (static_cast<uint32_t>(length_buffer[1]) << 16);
  message_length |= (static_cast<uint32_t>(length_buffer[2]) << 8);
  message_length |= static_cast<uint32_t>(length_buffer[3]);

  // Add length prefix to response
  response.insert(response.end(), length_buffer.begin(), length_buffer.end());

  // If message_length is 0, it's a keep-alive message (no additional data)
  if (message_length == 0) {
    std::cout << "Received keep-alive message" << std::endl;
    return response;
  }

  // Read the actual message content
  std::vector<unsigned char> message_buffer(message_length);
  bytes_read =
      boost::asio::read(stream_, boost::asio::buffer(message_buffer), ec);

  if (ec) {
    if (ec == boost::asio::error::eof) {
      std::cout << "Connection closed while reading message body" << std::endl;
    } else {
      std::cerr << "Error reading message body: " << ec.message() << std::endl;
    }
    return response; // Return partial response (just length prefix)
  }

  // Add message content to response
  response.insert(response.end(), message_buffer.begin(), message_buffer.end());
  return response;
}
