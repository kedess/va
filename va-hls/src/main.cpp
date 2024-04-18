#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <csignal>
#include <thread>

#include "server.h"
#include "settings.h"
#include "state/state.h"
#include "version.h"

namespace opt = boost::program_options;
namespace logging = boost::log;

va::StateApp state;
va::Settings settings;

volatile static std::sig_atomic_t signal_num = -1;
void siginthandler(int param) {
    signal_num = param;
    state.stop_app();
    BOOST_LOG_TRIVIAL(info) << "stop signal has been received (" << param << ")";
}
void init_logging(const std::string &level) {
    if (level == "debug") {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::debug);
    } else if (level == "info") {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    } else if (level == "warning") {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::warning);
    } else if (level == "error") {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error);
    } else {
        throw std::runtime_error("unknow logging level");
    }
}

int main(int argc, char *argv[]) {
    try {
        opt::options_description desc("all options");
        desc.add_options()("prefix-archive-path", opt::value<std::string>()->default_value("/tmp/va"),
                           "prefix of the path for saving video files, default '/tmp/va'");
        desc.add_options()("port", opt::value<uint16_t>()->default_value(8888),
                           "listening port of the server, default 8888");
        desc.add_options()("logging-level", opt::value<std::string>()->default_value("info"),
                           "logging level (debug, info, warning, error), default 'info'");
        desc.add_options()("help,h", "help");
        opt::variables_map vm;
        opt::store(opt::parse_command_line(argc, argv, desc), vm);
        opt::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return EXIT_FAILURE;
        }
        init_logging(vm["logging-level"].as<std::string>());
        settings.prefix_archvie_path(vm["prefix-archive-path"].as<std::string>());
        settings.port(vm["port"].as<uint16_t>());
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(fatal) << "parse params error: " << ex.what();
        return EXIT_FAILURE;
    }
    signal(SIGINT, siginthandler);
    BOOST_LOG_TRIVIAL(info) << "application " << PROJECT_NAME << " has been started "
                            << "ver." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    BOOST_LOG_TRIVIAL(info) << "boost version " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
                            << BOOST_VERSION % 100;
    boost::asio::io_context io_context;
    std::thread th([&]() {
        va::Server server(io_context, settings.port());
        io_context.run();
    });
    state.wait_stop_app();
    io_context.stop();
    th.join();
    BOOST_LOG_TRIVIAL(info) << "HLS has been stopped";
    BOOST_LOG_TRIVIAL(info) << "application has been stopped";
    return EXIT_SUCCESS;
}