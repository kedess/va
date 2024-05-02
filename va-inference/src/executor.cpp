#include "executor.h"
#include "../../common/thread-safe-queue.h"
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/log/trivial.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

extern va::ThreadSafeQueue<std::string> queue;

namespace va {
    Executor::Executor(const Settings &settings, std::stop_token stoken)
        : inference_uri_(settings.inference_server_uri()), inference_port_(settings.inference_server_port()) {
        auto num_threads = settings.num_threads();
        while (num_threads > 0) {
            threads_.emplace_back(&Executor::inference, this, stoken);
            --num_threads;
        }
    }
    void Executor::send(std::vector<char> &buffer, std::string &&path) {
        using boost::asio::ip::tcp;
        boost::asio::io_context io_context;

        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);

        boost::asio::connect(socket, resolver.resolve(inference_uri_, std::to_string(inference_port_)));
        auto buffer_len = static_cast<int64_t>(buffer.size());
        char bytes[8] = {0};
        std::copy(static_cast<const char *>(static_cast<const void *>(&buffer_len)),
                  static_cast<const char *>(static_cast<const void *>(&buffer_len)) + sizeof(buffer_len), bytes);

        boost::asio::write(socket, boost::asio::buffer(bytes));
        boost::asio::write(socket, boost::asio::buffer(buffer));
        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\r\n\r\n");
        std::string s((std::istreambuf_iterator<char>(&buf)), std::istreambuf_iterator<char>());
        size_t extension_pos{path.find(".ts")};
        auto path_inference{path.replace(extension_pos, 3, ".infe")};
        std::ofstream ofs(path_inference);
        ofs << s;
        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close();
        if (!ofs) {
            auto err_msg = std::format("could not write to file {}", path_inference);
            throw std::runtime_error(err_msg);
        }
    }
    void Executor::inference(std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            auto path = queue.try_pop();
            if (!path) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            BOOST_LOG_TRIVIAL(debug) << "Start inference file (" << *path << ")";
            std::ifstream input(*path, std::ios::binary);
            if (input) {
                // copies all data into buffer
                std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
                try {
                    send(buffer, std::string(path->begin(), path->end()));
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    BOOST_LOG_TRIVIAL(debug)
                        << "Finish inference file (" << *path << "), elapsed time = "
                        << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms";
                } catch (std::exception &ex) {
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    BOOST_LOG_TRIVIAL(error)
                        << "Finish inference file (" << *path << "), elapsed time = "
                        << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
                        << " ms, with error " << ex.what();
                }
            } else {
                BOOST_LOG_TRIVIAL(error) << "Error inference file (" << *path << "), because could not open file";
            }
        }
    }
} // namespace va