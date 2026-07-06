#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <codecvt>

#include "server.h"
#include "additions.h"

void Server::server_init() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

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
    socket->close();
    delete socket;
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
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                messages.emplace_back(std::string(buffer, bytes));
            }
            draw_msg();
            draw_input(msg);
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
void Server::send_msg(boost::asio::ip::tcp::socket* socket) {
    try {
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        INPUT_RECORD record;
        DWORD events;

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        while (true) {
            ReadConsoleInputW(hIn, &record, 1, &events);

            if (record.EventType != KEY_EVENT)
                continue;

            KEY_EVENT_RECORD key = record.Event.KeyEvent;

            if (!key.bKeyDown)
                continue;

            wchar_t wc = key.uChar.UnicodeChar;
            if (wc >= 32) {
                msg += to_utf8(wc);
            }
            if (key.wVirtualKeyCode == VK_BACK) {
                if (!msg.empty())
                    msg.pop_back();
            }
            if (key.wVirtualKeyCode == VK_RETURN) {
                {
                    std::lock_guard<std::mutex> lock(messages_mutex);
                    messages.emplace_back(nickname + ": " + msg + '\n');
                }
                socket->write_some(boost::asio::buffer(nickname + ": " + msg + '\n'));
                msg.clear();
            }
            draw_msg();
            draw_input(msg);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void Server::draw_msg() {
    system("cls");
    for (const auto& i : messages) {
        std::cout << i;
    }
}
void Server::draw_input(const std::string& msg) {
    std::cout << "\n> " << msg;
}
void Server::draw_raw_input() {
    std::cout << "\n> ";
}

