#pragma once

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#ifdef __unix

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#endif

namespace gk {
    class TCP {
        int socket_;

        std::thread accpet_thread_;

    protected:
        virtual void OnReceive(const int socket, const std::string &message) {
        }

    public:
        std::mutex clients_mutex;
        std::vector<int> clients_;

        bool running = true;

        void GetLocalName(std::string &addr, unsigned short &port) const {
            sockaddr_in real_addr{};
            socklen_t real_addr_len = sizeof(real_addr);
            getsockname(socket_, (sockaddr *) &real_addr, &real_addr_len);

            char real_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &real_addr.sin_addr, real_ip, INET_ADDRSTRLEN);
            addr = std::string(real_ip);

            port = ntohs(real_addr.sin_port);
        }

        explicit TCP(const unsigned short port = 0) {
            socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            //addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(port);

            bind(socket_, (sockaddr *) &addr, sizeof(addr));

            listen(socket_, SOMAXCONN);

            accpet_thread_ = std::thread([this]() {
                while (running) {
                    sockaddr_in client_addr{};
                    socklen_t client_addr_len = sizeof(client_addr);
                    auto client = accept(socket_, (sockaddr *) &client_addr, &client_addr_len);

                    clients_.push_back(client);

                    std::thread([this, client] {
                        while (running) {
                            char buffer[1024] = {};

                            if (const auto len = recv(client, buffer, sizeof(buffer), 0); len > 0) {
                                OnReceive(client, std::string(buffer, len));
                            } else {
                                break;
                            }
                        }

                        std::lock_guard lock(clients_mutex);

                        clients_.erase(
                            std::remove_if(clients_.begin(), clients_.end(), [client](const int socket) {
                                return socket == client;
                            }),
                            clients_.end()
                        );

                        close(client);
                    }).detach();
                }
            });
        }

        virtual ~TCP() {
            close(socket_);

            accpet_thread_.join();
        }

        static void Send(const int &socket, const std::string &message) {
            send(socket, message.c_str(), static_cast<int>(message.length()), 0);
        }
    };
}
