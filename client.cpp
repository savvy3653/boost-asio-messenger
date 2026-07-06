#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <Windows.h>
#include <codecvt>

#include "client.h"
#include "additions.h"

void Client::client_init() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

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
    socket->close();
    delete socket;
}
void Client::read_get(boost::asio::ip::tcp::socket* socket) {
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
void Client::send_msg(boost::asio::ip::tcp::socket* socket) {
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

void Client::draw_msg() {
    system("cls");
    for (const auto& i : messages) {
        std::cout << i;
    }
}
void Client::draw_input(const std::string& msg) {
    std::cout << "\n> " << msg;
}

void Client::draw_raw_input() {
    std::cout << "\n> ";
}


