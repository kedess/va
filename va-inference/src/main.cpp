#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "../../common/state.h"
#include "../../common/thread-safe-queue.h"
#include "../../common/utils.h"
#include "../../version.h"
#include "../../watcher/watcher.h"
#include "executor.h"
#include "settings.h"

namespace opt = boost::program_options;
namespace logging = boost::log;

va::StateApp state;
va::Settings settings;
va::ThreadSafeQueue<std::string> queue;

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
        desc.add_options()("inference-server-address", opt::value<std::string>()->default_value("localhost"),
                           "inference server address, default localhost");
        desc.add_options()("inference-server-port", opt::value<int16_t>()->default_value(3030),
                           "inference server port, default 3030");
        desc.add_options()("num-threads", opt::value<size_t>()->default_value(1),
                           "number of parallel threads, default 1");
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
        settings.inference_server_address(vm["inference-server-address"].as<std::string>());
        settings.inference_server_port(vm["inference-server-port"].as<int16_t>());
        settings.num_threads(vm["num-threads"].as<size_t>());
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(fatal) << "parse params error: " << ex.what();
        return EXIT_FAILURE;
    }
    signal(SIGINT, siginthandler);
    BOOST_LOG_TRIVIAL(info) << "module va-inference has been started "
                            << "ver." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    BOOST_LOG_TRIVIAL(info) << "boost version " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
                            << BOOST_VERSION % 100;
    std::jthread th_watcher([&](std::stop_token stoken) {
        try {
            va::Watcher watcher(settings.prefix_archive_path().c_str());
            watcher.run(stoken, [&](std::string &path) {
                if (path.ends_with(".ts")) {
                    queue.push(path);
                }
            });
        } catch (std::exception &ex) {
            BOOST_LOG_TRIVIAL(fatal) << ex.what();
            BOOST_LOG_TRIVIAL(fatal) << "application will be stopped";
            signal_num = SIGABRT;
            state.stop_app();
        }
    });
    std::jthread th_executor([&](std::stop_token stoken) { va::Executor executor(settings, stoken); });

    state.wait_stop_app();
    th_watcher.request_stop();
    th_executor.request_stop();
    BOOST_LOG_TRIVIAL(info) << "application has been stopped";
    return EXIT_SUCCESS;
}