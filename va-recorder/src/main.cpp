#include "../../common/state.h"
#include "app.h"
#include "source/source.h"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <csignal>
#include <cstdlib>
#include <iostream>

extern "C" {
#include <libavdevice/avdevice.h>
}

namespace opt = boost::program_options;
namespace logging = boost::log;

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
    avdevice_register_all();
    avformat_network_init();
    av_log_set_level(AV_LOG_QUIET);

    va::App app;

    try {
        opt::options_description desc("all options");
        desc.add_options()("prefix-archive-path", opt::value<std::string>()->default_value("/tmp/va"),
                           "prefix of the path for saving video files, default '/tmp/va'");
        desc.add_options()("duration-file", opt::value<int64_t>()->default_value(10),
                           "duration file seconds, default 10 secs");
        desc.add_options()("log-level", opt::value<std::string>()->default_value("info"),
                           "log level (debug, info, warning, error), default 'info'");
        desc.add_options()("source-file", opt::value<std::string>()->default_value("sources.json"),
                           "path to source file, default sources.json in current directory");
        desc.add_options()("help,h", "help");
        opt::variables_map vm;
        opt::store(opt::parse_command_line(argc, argv, desc), vm);
        opt::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return EXIT_FAILURE;
        }
        init_logging(vm["log-level"].as<std::string>());
        app.prefix_archvie_path(vm["prefix-archive-path"].as<std::string>());
        app.source_file(vm["source-file"].as<std::string>());
        app.duration_file(vm["duration-file"].as<int64_t>());
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(fatal) << "parse params error: " << ex.what();
        return EXIT_FAILURE;
    }

    signal(SIGINT, siginthandler);

    std::vector<va::Source> sources;
    try {
        sources = va::load_sources_from_file(app.source_file().c_str());
        size_t nstreams_success = 0;
        for (const auto &source : sources) {
            BOOST_LOG_TRIVIAL(info) << "loaded source"
                                    << "(id: '" << source.id() << "', url: '" << source.url() << "')";
            app.run_in_stream(source);
            ++nstreams_success;
        }
        if (nstreams_success) {
            BOOST_LOG_TRIVIAL(info) << nstreams_success << "/" << sources.size() << " video sources will be launched";
            va::StateApp::instance().wait_stop_app();
        } else {
            BOOST_LOG_TRIVIAL(info) << "there are no video sources to launch";
        }
    } catch (std::exception &ex) {
        BOOST_LOG_TRIVIAL(fatal) << ex.what();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}