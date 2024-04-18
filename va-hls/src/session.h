#pragma once

#include "utils.h"
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

using boost::asio::ip::tcp;

constexpr size_t buffer_length = 4 * 1024;

const char *BAD_REUEST =
    "HTTP/1.1 400\r\nContent-Type: text/plain\r\nContent-Length: 0\r\nAccept-Ranges: bytes\r\n\r\n";
const size_t BAD_REUEST_SIZE = strlen(BAD_REUEST);
const char *NOT_FOUND_REUEST =
    "HTTP/1.1 404\r\nContent-Type: text/plain\r\nContent-Length: 0\r\nAccept-Ranges: bytes\r\n\r\n";
const size_t NOT_FOUND_REUEST_SIZE = strlen(NOT_FOUND_REUEST);

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
                socket_, in_buffer_, "\r\n\r\n", [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        std::string req{std::istreambuf_iterator<char>(&in_buffer_), std::istreambuf_iterator<char>()};
                        try {
                            auto uri = fetch_uri(req);
                            if (uri.ends_with(".m3u8")) {
                                // Запрос playlist
                                auto playlist_opt = make_playlist(uri);
                                if (playlist_opt.has_value()) {
                                    auto playlist = playlist_opt.value();
                                    auto resp = std::format(
                                        "HTTP/1.1 200\r\nContent-Type: application/x-mpegURL\r\nContent-Length: "
                                        "{}\r\nAccept-Ranges: bytes\r\n\r\n{}",
                                        playlist.size(), playlist);
                                    std::ostream out(&out_buffer_);
                                    std::copy(resp.begin(), resp.end(), std::ostream_iterator<char>(out));
                                    boost::asio::async_write(socket_, out_buffer_,
                                                             [this, self](boost::system::error_code ec, std::size_t) {
                                                                 if (ec) {
                                                                     BOOST_LOG_TRIVIAL(error) << ec;
                                                                 }
                                                             });
                                } else {
                                    std::ostream out(&out_buffer_);
                                    std::copy(NOT_FOUND_REUEST, NOT_FOUND_REUEST + NOT_FOUND_REUEST_SIZE,
                                              std::ostream_iterator<char>(out));
                                    boost::asio::async_write(socket_, out_buffer_,
                                                             [this, self](boost::system::error_code ec, std::size_t) {
                                                                 if (ec) {
                                                                     BOOST_LOG_TRIVIAL(error) << ec;
                                                                 }
                                                             });
                                }
                            } else if (uri.ends_with(".ts")) {
                                // Запрос сегмента
                                auto path = std::format("/tmp/va{}", uri);
                                std::ifstream file(path, std::ios::binary);
                                std::vector<char> data;
                                std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),
                                          std::back_insert_iterator(data));
                                auto resp = std::format("HTTP/1.1 200\r\nContent-Type: video/mp2t\r\nContent-Length: "
                                                        "{}\r\nAccept-Ranges: bytes\r\n\r\n",
                                                        data.size());
                                std::ostream out(&out_buffer_);
                                std::copy(resp.begin(), resp.end(), std::ostream_iterator<char>(out));
                                std::copy(data.begin(), data.end(), std::ostream_iterator<char>(out));
                                boost::asio::async_write(socket_, out_buffer_,
                                                         [this, self](boost::system::error_code ec, std::size_t) {
                                                             if (ec) {
                                                                 BOOST_LOG_TRIVIAL(error) << ec;
                                                             }
                                                         });
                            } else {
                                // Не валидный запрос
                                BOOST_LOG_TRIVIAL(error) << "Invalid request";
                                std::ostream out(&out_buffer_);
                                std::copy(BAD_REUEST, BAD_REUEST + BAD_REUEST_SIZE, std::ostream_iterator<char>(out));
                                boost::asio::async_write(socket_, out_buffer_,
                                                         [this, self](boost::system::error_code ec, std::size_t) {
                                                             if (ec) {
                                                                 BOOST_LOG_TRIVIAL(error) << ec;
                                                             }
                                                         });
                            }
                        } catch (fs::filesystem_error const &ex) {
                            BOOST_LOG_TRIVIAL(error) << ex.what();
                            std::ostream out(&out_buffer_);
                            std::copy(NOT_FOUND_REUEST, NOT_FOUND_REUEST + NOT_FOUND_REUEST_SIZE,
                                      std::ostream_iterator<char>(out));
                            boost::asio::async_write(socket_, out_buffer_,
                                                     [this, self](boost::system::error_code ec, std::size_t) {
                                                         if (ec) {
                                                             BOOST_LOG_TRIVIAL(error) << ec;
                                                         }
                                                     });

                        } catch (std::exception &ex) {
                            BOOST_LOG_TRIVIAL(error) << ex.what();
                            std::ostream out(&out_buffer_);
                            std::copy(BAD_REUEST, BAD_REUEST + BAD_REUEST_SIZE, std::ostream_iterator<char>(out));
                            boost::asio::async_write(socket_, out_buffer_,
                                                     [this, self](boost::system::error_code ec, std::size_t) {
                                                         if (ec) {
                                                             BOOST_LOG_TRIVIAL(error) << ec;
                                                         }
                                                     });
                        }
                    } else {
                        BOOST_LOG_TRIVIAL(error) << ec;
                    }
                });
        }

        tcp::socket socket_;
        boost::asio::streambuf in_buffer_;
        boost::asio::streambuf out_buffer_;
    };
} // namespace va