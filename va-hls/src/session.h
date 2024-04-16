#pragma once

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

constexpr size_t buffer_length = 1024;

namespace va {
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(tcp::socket socket) : socket_(std::move(socket)) {
        }

        tcp::socket &socket() {
            return socket_;
        }

        void start() {
            auto self(shared_from_this());
            boost::asio::async_read_until(
                socket_, buffer_, "\r\n\r\n", [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        std::string data{std::istreambuf_iterator<char>(&buffer_), std::istreambuf_iterator<char>()};
                        start();
                    } else {
                        BOOST_LOG_TRIVIAL(error) << ec;
                    }
                });
            const char *msg = "Hello World\n";
            auto buffer = boost::asio::buffer(msg, strlen(msg));
            boost::asio::async_write(socket_, buffer, [this, self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    BOOST_LOG_TRIVIAL(error) << ec;
                }
            });
        }

        tcp::socket socket_;
        boost::asio::streambuf buffer_;
    };
} // namespace va