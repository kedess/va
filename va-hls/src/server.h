#pragma once

#include <boost/asio.hpp>
#include <boost/asio/detail/push_options.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/trivial.hpp>

#include "session.h"

using boost::asio::ip::tcp;

namespace va {
    class Server {
    public:
        Server(boost::asio::io_context &io_contex, unsigned short port)
            : acceptor_(io_contex, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
            BOOST_LOG_TRIVIAL(info) << "started HLS Server on port " << port;
            do_accept();
        }
        void do_accept();

    private:
        boost::asio::ip::tcp::acceptor acceptor_;
    };
} // namespace va