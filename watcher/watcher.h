#pragma once

#include <boost/log/trivial.hpp>
#include <filesystem>
#include <format>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/inotify.h>
#include <sys/select.h>

namespace va {
    enum class FileStatus { created, modified, erased };

    struct EventWatcher {
        std::string filename;
        FileStatus kind;
    };

    class Watcher {
    public:
        Watcher(const char *path) {
            notifyfd_ = inotify_init();
            if (notifyfd_ < 0) {
                throw std::runtime_error("could not init notify");
            }
            fcntl(notifyfd_, F_SETFL, O_NONBLOCK);
            auto wd = inotify_add_watch(notifyfd_, path, IN_CREATE | IN_CLOSE_WRITE);
            if (wd < 0) {
                auto err_msg = std::format("could not add watch for \'{}\'", path);
                throw std::runtime_error(err_msg);
            }
            watchfds_.insert(std::make_pair(wd, std::string(path)));
            for (auto &file : std::filesystem::recursive_directory_iterator(path)) {
                if (file.is_directory()) {
                    wd = inotify_add_watch(notifyfd_, file.path().c_str(), IN_CREATE | IN_CLOSE_WRITE);
                    if (wd < 0) {
                        auto err_msg = std::format("could not add watch for \'{}\'", file.path().c_str());
                        throw std::runtime_error(err_msg);
                    }
                    watchfds_.insert(std::make_pair(wd, file.path().string()));
                }
            }
            BOOST_LOG_TRIVIAL(info) << "watch for path \"" << path << "\"";
        }
        ~Watcher() {
            if (notifyfd_ >= 0) {
                for (auto [wd, filename] : watchfds_) {
                    inotify_rm_watch(notifyfd_, wd);
                }
                close(notifyfd_);
            }
            BOOST_LOG_TRIVIAL(info) << "cleanup watch";
        }
        template <typename Func> void run(std::stop_token stoken, const Func &action) {
            while (!stoken.stop_requested()) {
                struct timeval timeout = {};
                timeout.tv_sec = 2;
                FD_ZERO(&readfds_);
                FD_SET(notifyfd_, &readfds_);
                int ret = select(notifyfd_ + 1, &readfds_, &writefds_, NULL, &timeout);
                if (-1 == ret) {
                    throw std::runtime_error("could not invoke select");
                } else if (0 == ret) {
                    continue;
                } else {
                    char buffer[sizeof(struct inotify_event) + NAME_MAX + 1] alignas(struct inotify_event);
                    ssize_t count = read(notifyfd_, buffer, sizeof(buffer));
                    if (count < 0) {
                        break;
                    }
                    ssize_t i = 0;
                    while (i < count) {
                        const struct inotify_event *event = reinterpret_cast<const struct inotify_event *>(&buffer[i]);
                        if (event->mask & IN_ISDIR) {
                            std::string filename(event->name);
                            auto prefix = watchfds_[event->wd];
                            auto path = std::format("{}/{}", prefix, filename);
                            auto wd = inotify_add_watch(notifyfd_, path.c_str(), IN_CREATE | IN_CLOSE_WRITE);
                            if (wd < 0) {
                                auto err_msg = std::format("could not add watch for \'{}\'", filename);
                                throw std::runtime_error(err_msg);
                            }
                            watchfds_.insert(std::make_pair(wd, path));
                            for (auto &file : std::filesystem::recursive_directory_iterator(path)) {
                                if (file.is_directory()) {
                                    wd = inotify_add_watch(notifyfd_, file.path().c_str(), IN_CREATE | IN_CLOSE_WRITE);
                                    if (wd < 0) {
                                        auto err_msg =
                                            std::format("could not add watch for \'{}\'", file.path().c_str());
                                        throw std::runtime_error(err_msg);
                                    }
                                    watchfds_.insert(std::make_pair(wd, file.path().string()));
                                }
                            }
                        }
                        if (event->mask & IN_CLOSE_WRITE) {
                            std::string filename(event->name);
                            auto prefix = watchfds_[event->wd];
                            auto path = std::format("{}/{}", prefix, filename);
                            BOOST_LOG_TRIVIAL(debug) << "new watch file \"" << path << "\"";
                            action(path);
                        }
                        i += static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
                    }
                }
            }
        }

    private:
        int notifyfd_ = -1;
        std::map<int, std::string> watchfds_;
        fd_set readfds_;
        fd_set writefds_;
    };
} // namespace va