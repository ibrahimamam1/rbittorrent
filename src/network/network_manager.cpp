#include "network_manager.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
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
