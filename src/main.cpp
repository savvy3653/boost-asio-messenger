#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <locale>
#include <codecvt>

#include "../include/server.h"
#include "../include/client.h"

void server_logic() {
    Server server;
    server.server_init();
}

void client_logic() {
    Client client;
    client.client_init();
}

int main(int agrc, char* argv[]) {
    std::locale::global(std::locale(""));

    std::string answer;
    while (true) {
        std::cout << "Server/Client?: ";
        std::getline(std::cin, answer);
        if (answer == "Server" || answer == "server") {
            try {
                server_logic();
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            break;
        } else if (answer == "Client" || answer == "client") {
            try {
                client_logic();
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            break;
        } else continue;
    }

    return 0;
}