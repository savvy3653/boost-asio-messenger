#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "server.h"
#include "client.h"

void server_logic() {
    Server server;
    server.server_init();
}

void client_logic() {
    Client client;
    client.client_init();
}

int main(int agrc, char* argv[]) {
    std::string answer;
    while (true) {
        std::cout << "Server/Client?: ";
        std::getline(std::cin, answer);
        if (answer == "Server" || answer == "server") {
            server_logic();
            break;
        } else if (answer == "Client" || answer == "client") {
            client_logic();
            break;
        } else continue;
    }

    return 0;
}