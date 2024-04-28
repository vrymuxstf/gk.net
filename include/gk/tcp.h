#pragma once

#include <iostream>
#include <thread>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace gk {
    class Tcp {
        SOCKET socket_;
        std::vector<SOCKET> clients_;

        std::thread accpet_thread_;
        std::vector<std::thread> read_threads_;

    public:
        bool running = true;

        Tcp() { {
                WSADATA wsaData;
                int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            } {
                socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            } {
                sockaddr_in serverAddress{};
                serverAddress.sin_family = AF_INET;
                serverAddress.sin_addr.s_addr = INADDR_ANY;
                serverAddress.sin_port = htons(8080);

                int result = bind(socket_, (sockaddr *) &serverAddress, sizeof(serverAddress));
            } {
                int result = listen(socket_, SOMAXCONN);
            } {
                accpet_thread_ = std::thread([this]() {
                    while (running) {
                        sockaddr_in client_addr{};
                        int client_addr_len = sizeof(client_addr);
                        auto client = accept(socket_, (sockaddr *) &client_addr, &client_addr_len);
                        clients_.push_back(client);

                        read_threads_.emplace_back([this, client]() {
                            while (running) {
                                char buffer[1024] = {};
                                int bytesReceived = recv(client, buffer, sizeof(buffer), 0);

                                std::string httpResponse = "HTTP/1.1 200 OK\r\n";
                                httpResponse += "Content-Type: text/plain\r\n";
                                httpResponse += "Content-Length: 13\r\n\r\n";
                                httpResponse += "Hello, world!";

                                send(client, httpResponse.c_str(), httpResponse.length(), 0);
                            }
                        });
                    }
                });
            }
        }

        ~Tcp() {
            accpet_thread_.join();
            for (auto &thread: read_threads_) {
                thread.join();
            }

            WSACleanup();
        }
    };
}
