#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <functional>
#include <vector>

#define HOST "rbittorent"

namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
using parameterList = std::vector<std::pair<std::string, std::string>>;

class NetworkManager {
  asio::io_context ioc;
  beast::tcp_stream stream{ioc};
  tcp::resolver dns_resolver{ioc};
  boost::beast::error_code ec;

public:
  http::response<http::dynamic_body> makeGetRequest(const std::string &uri,
                                                    parameterList params);
  void
  writeToStream(beast::tcp_stream &stream,
                const std::vector<unsigned char> &data);
  void writeToStreamNoResponse(beast::tcp_stream& stream_,
                               const std::vector<unsigned char> &data);

  std::vector<unsigned char> readFromStream(beast::tcp_stream& stream_, size_t bytes_to_read);
  void clearStreamBuffer(beast::tcp_stream& stream);
};
