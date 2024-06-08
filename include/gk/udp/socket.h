#pragma once

#include <iostream>

#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#endif

namespace gk::UDP {
#ifdef __unix
    typedef int SOCKET_TYPE;
#endif

#ifdef WIN32
    typedef SOCKET SOCKET_TYPE;

    class Winsock {
    public:
        Winsock() {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
        }

        ~Winsock() {
            WSACleanup();
        }

        static void Singleton() {
            static Winsock winsock;
        }
    };

#endif

    class Socket {
        SOCKET_TYPE socket_;

    public:
        explicit Socket(const unsigned short port = 0) {
#ifdef WIN32
            Winsock::Singleton();
#endif
            socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

            u_long nonblocking = 1;
            ioctlsocket(socket_, FIONBIO, &nonblocking);

            sockaddr_in addr{};
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);

            if (bind(socket_, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR) {
                std::cerr << "GK UDP SOCKET BIND ERROR!\n";
            }
        }

        void Send(const std::string &message, const std::string &ip, const unsigned short port) const {
            sockaddr_in addr{};
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

            sendto(socket_, message.c_str(), (int) message.length(), 0, (sockaddr *) &addr, sizeof(addr));
        }

        [[nodiscard]] std::string Poll() const {
            char buffer[1024];
            sockaddr_in addr{};
            int addr_len = sizeof(addr);
            const int recv_bytes = recvfrom(socket_, buffer, sizeof(buffer) - 1, 0, (sockaddr *) &addr, &addr_len);

            if (recv_bytes == SOCKET_ERROR) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    std::cerr << "recvfrom failed: " << WSAGetLastError() << "\n";
                }
            }

            if (recv_bytes > 0) {
                return {buffer, buffer + recv_bytes};
            }

            return {};
        }
    };
}
