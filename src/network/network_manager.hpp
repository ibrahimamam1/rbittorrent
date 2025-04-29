#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>

#define HOST "rbittorent"

using tcp = boost::asio::ip::tcp; 
namespace http = boost::beast::http;

class NetworkManager{
  boost::asio::io_context ioc;
  boost::beast::tcp_stream stream{ioc};
  tcp::resolver dns_resolver{ioc};
  boost::beast::error_code ec;

public:
  http::response<http::dynamic_body> makeGetRequest(const std::string& uri);
};

