#include <iostream>

#include "gk/udp/socket.h"

int main() {
    gk::UDP::Socket socket;

    while (true) {
        if (auto message = socket.Poll(); !message.empty()) {
            std::cout << message << std::endl;
        }
    }
}
