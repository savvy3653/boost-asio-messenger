#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <locale>
#include <codecvt>

#include "../include/server.h"
#include "../include/client.h"

void server_logic() {
    std::shared_ptr<Server> server = std::make_shared<Server>();
    server->server_init();
}

void client_logic() {
    std::shared_ptr<Client> client = std::make_shared<Client>();
    client->client_init();
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