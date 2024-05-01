#pragma once

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace gk {
    class TCP {
        SOCKET socket_;

        std::thread accpet_thread_;

    protected:
        virtual void OnReceive(const SOCKET socket, const std::string &message) {
        }

    public:
        std::mutex clients_mutex;
        std::vector<SOCKET> clients_;

        bool running = true;

        void GetLocalName(std::string &addr, unsigned short &port) const {
            sockaddr_in real_addr{};
            int real_addr_len = sizeof(real_addr);
            getsockname(socket_, (sockaddr *) &real_addr, &real_addr_len);

            char real_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &real_addr.sin_addr, real_ip, INET_ADDRSTRLEN);
            addr = std::string(real_ip);

            port = ntohs(real_addr.sin_port);
        }

        explicit TCP(const unsigned short port = 0) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);

            socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            //addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(port);

            if (const auto result = bind(socket_, (sockaddr *) &addr, sizeof(addr));
                result == SOCKET_ERROR) {
            }

            listen(socket_, SOMAXCONN);

            accpet_thread_ = std::thread([this]() {
                while (running) {
                    sockaddr_in client_addr{};
                    int client_addr_len = sizeof(client_addr);
                    auto client = accept(socket_, (sockaddr *) &client_addr, &client_addr_len);

                    clients_.push_back(client);

                    std::thread([this, client] {
                        while (running) {
                            char buffer[1024] = {};

                            if (const int len = recv(client, buffer, sizeof(buffer), 0); len > 0) {
                                OnReceive(client, std::string(buffer, len));
                            } else {
                                break;
                            }
                        }

                        std::lock_guard lock(clients_mutex);

                        clients_.erase(
                            std::remove_if(clients_.begin(), clients_.end(), [client](const SOCKET socket) {
                                return socket == client;
                            }),
                            clients_.end()
                        );
                    }).detach();
                }
            });
        }

        virtual ~TCP() {
            closesocket(socket_);

            WSACleanup();

            accpet_thread_.join();
        }

        static void Send(const SOCKET &socket, const std::string &message) {
            send(socket, message.c_str(), static_cast<int>(message.length()), 0);
        }
    };
}
