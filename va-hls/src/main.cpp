#include <boost/version.hpp>
#include <csignal>
#include <thread>

#include "server.h"
#include "state/state.h"
#include "version.h"

va::StateApp state;

volatile static std::sig_atomic_t signal_num = -1;
void siginthandler(int param) {
    signal_num = param;
    state.stop_app();
    BOOST_LOG_TRIVIAL(info) << "stop signal has been received (" << param << ")";
}

int main(/*int argc, char *argv[]*/) {
    signal(SIGINT, siginthandler);
    BOOST_LOG_TRIVIAL(info) << "application " << PROJECT_NAME << " has been started "
                            << "ver." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    BOOST_LOG_TRIVIAL(info) << "boost version " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
                            << BOOST_VERSION % 100;
    boost::asio::io_context io_context;
    std::thread th([&]() {
        va::Server server(io_context, 8888);
        io_context.run();
    });
    state.wait_stop_app();
    io_context.stop();
    th.join();
    BOOST_LOG_TRIVIAL(info) << "HLS has been stopped";
    BOOST_LOG_TRIVIAL(info) << "application has been stopped";
    return 0;
}