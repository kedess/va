#include "server.h"

namespace va {
    void Server::do_accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                BOOST_LOG_TRIVIAL(debug) << "creating session on: " << socket.remote_endpoint().address().to_string()
                                         << ":" << socket.remote_endpoint().port();
                std::make_shared<Session>(std::move(socket))->start();
            } else {
                BOOST_LOG_TRIVIAL(error) << ec.message() << std::endl;
            }
            do_accept();
        });
    }
} // namespace va