#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>

#include "client.h"

void Client::client_init() {
    std::string address{};

    std::cout << "Enter nickname: ";
    std::getline(std::cin, nickname);
    std::cout << "Enter address: ";
    std::getline(std::cin, address);

    auto parser = [&]() {
        auto pos = address.find(':');
        ip = address.substr(0, pos);
        port = static_cast<std::uint16_t>(std::stoi(address.substr(pos + 1)));
    };
    parser();

    boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_context);

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    try {
        socket->connect(endpoint);
        std::cout << "Connected." << '\n';
    } catch (const std::exception& e) {
        std::cout << "Error connecting to server: " << e.what() << std::endl;
    }

    std::thread rg(Client::read_get, this,  socket);
    std::thread sm(Client::send_msg, this, socket);

    sm.join();
    rg.join();
}
void Client::read_get(boost::asio::ip::tcp::socket* socket) {
    try {
        while (true) {
            char buffer[2048];
            std::uint16_t bytes = socket->read_some(boost::asio::buffer(buffer, sizeof(buffer)));
            std::cout << std::string(buffer, bytes);
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
void Client::send_msg(boost::asio::ip::tcp::socket* socket) {
    try {
        while (true) {
            std::string msg;
            std::getline(std::cin, msg);
            socket->write_some(boost::asio::buffer(nickname + ": " + msg + '\n'));
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

