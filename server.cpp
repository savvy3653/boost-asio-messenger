#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>

#include "server.h"

void Server::server_init() {
    std::cout << "Enter nickname: ";
    std::getline(std::cin, nickname);
    std::cout << "Enter port: ";
    std::cin >> port;

    acceptor = new boost::asio::ip::tcp::acceptor(
    *io_context,
    boost::asio::ip::tcp::endpoint(
        boost::asio::ip::tcp::v4(),
        port
    )
    );
    boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_context);

    connect(acceptor, socket);
    std::thread rg(Server::read_get, this, socket);
    std::thread sm(Server::send_msg, this, socket);

    sm.join();
    rg.join();
}
void Server::connect(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::ip::tcp::socket* socket) {
    try {
        std::cout << "Waiting for connection..." << '\n';
        acceptor->accept(*socket);
        std::cout << "Connection established." << '\n';
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
void Server::read_get(boost::asio::ip::tcp::socket* socket) {
    try {
        while (true) {
            char buffer[2048];
            std::uint16_t bytes = socket->read_some(boost::asio::buffer(buffer, sizeof(buffer)));
            std::cout << std::string(buffer, bytes);
            Beep(1000, 200);
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
void Server::send_msg(boost::asio::ip::tcp::socket* socket) {
    try {
        while (true) {
            std::string msg;
            std::getline(std::cin, msg);
            socket->write_some(boost::asio::buffer(nickname + ": " + msg + '\n'));
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

