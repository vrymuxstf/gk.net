#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace gk {
    class Net {
    public:
        Net() {
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        }

        ~Net() {
            WSACleanup();
        }
    };
}
