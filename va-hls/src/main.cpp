#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <csignal>
#include <iostream>
#include <thread>

#include "../../common/state.h"
#include "../../common/utils.h"
#include "../../version.h"
#include "../../watcher/watcher.h"
#include "playlist.h"
#include "server.h"
#include "settings.h"
#include "utils.h"
#include <map>
#include <queue>
#include <shared_mutex>

namespace opt = boost::program_options;
namespace logging = boost::log;

std::map<std::string, va::PlayList> playlists;
std::shared_mutex mutex_playlists;

volatile static std::sig_atomic_t signal_num = -1;
void siginthandler(int param) {
    signal_num = param;
    va::StateApp::instance().stop_app();
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
        desc.add_options()("log-level", opt::value<std::string>()->default_value("info"),
                           "log level (debug, info, warning, error), default 'info'");
        desc.add_options()("help,h", "help");
        opt::variables_map vm;
        opt::store(opt::parse_command_line(argc, argv, desc), vm);
        opt::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return EXIT_FAILURE;
        }
        init_logging(vm["log-level"].as<std::string>());
        va::Settings::instance().prefix_archvie_path(vm["prefix-archive-path"].as<std::string>());
        va::Settings::instance().port(vm["port"].as<uint16_t>());
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(fatal) << "parse params error: " << ex.what();
        return EXIT_FAILURE;
    }
    signal(SIGINT, siginthandler);
    BOOST_LOG_TRIVIAL(info) << "module va-hls has been started "
                            << "ver." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    BOOST_LOG_TRIVIAL(info) << "boost version " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "."
                            << BOOST_VERSION % 100;

    std::jthread th_watcher([&](std::stop_token stoken) {
        try {
            va::Watcher watcher(va::Settings::instance().prefix_archive_path().c_str());
            watcher.run(stoken, [&](std::string &path) {
                if (path.ends_with(".ts")) {
                    auto tmp = std::string(path.begin(), path.end());
                    auto path_without_prefix =
                        tmp.replace(0, va::Settings::instance().prefix_archive_path().size() + 1, "");
                    auto parts = va::utils::split(path_without_prefix, '/');
                    if (!parts.empty()) {
                        auto stream_id = parts[0];
                        std::unique_lock lock(mutex_playlists);
                        auto it = playlists.find(stream_id);
                        if (it == playlists.end()) {
                            playlists[stream_id] = {0, {path_without_prefix}};
                        } else {
                            auto &queue_ref = playlists[stream_id].queue;
                            auto it = std::find(queue_ref.begin(), queue_ref.end(), path_without_prefix);
                            if (it == queue_ref.end()) {
                                if (queue_ref.size() > 3) {
                                    ++playlists[stream_id].index;
                                    queue_ref.pop_front();
                                }
                                queue_ref.push_back(std::move(path_without_prefix));
                            }
                        }
                    }
                }
            });
        } catch (std::exception &ex) {
            BOOST_LOG_TRIVIAL(fatal) << ex.what();
            BOOST_LOG_TRIVIAL(fatal) << "application will be stopped";
            signal_num = SIGABRT;
            va::StateApp::instance().stop_app();
        }
    });

    boost::asio::io_context io_context;
    std::thread th([&]() {
        va::Server server(io_context, va::Settings::instance().port());
        io_context.run();
    });
    va::StateApp::instance().wait_stop_app();
    io_context.stop();
    if (th.joinable()) {
        th.join();
    }
    th_watcher.request_stop();
    if (th_watcher.joinable()) {
        th_watcher.join();
    }
    BOOST_LOG_TRIVIAL(info) << "HLS has been stopped";
    BOOST_LOG_TRIVIAL(info) << "application has been stopped";
    return EXIT_SUCCESS;
}