#include "network_manager.hpp"
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url.hpp>
#include <iostream>

http::response<http::dynamic_body> NetworkManager::makeGetRequest(const std::string& uri) {
    http::response<http::dynamic_body> res;
    try {
        // Parse URI using Boost.URL
        boost::urls::url_view parsed_url(uri);

        // Resolve DNS
        auto const results = dns_resolver.resolve(parsed_url.host(), 
            parsed_url.has_port() ? parsed_url.port() : "80");

        // Connect to endpoint
        stream.connect(results, ec);
        if (ec) {
            throw boost::beast::system_error{ec};
        }

        // Create and configure HTTP request
        http::request<http::string_body> req{http::verb::get, parsed_url.path(), 11};
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
            throw boost::beast::system_error{ec};
        }

        // Shutdown connection
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::beast::errc::not_connected) {
            throw boost::beast::system_error{ec};
        }
    }
    catch (const std::exception& e) {
        throw; // Re-throw the exception
    }

    return res;
}
