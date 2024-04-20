#pragma once

#include "utils.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace va {
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(tcp::socket socket) : socket_(std::move(socket)) {
        }

        tcp::socket &socket() {
            return socket_;
        }

        void start();

        tcp::socket socket_;
        boost::asio::streambuf in_buffer_;
        boost::asio::streambuf out_buffer_;
    };
} // namespace va