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
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

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
        Beep(1000, 200);
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

            std::string timestamp = get_time();
            std::string full_msg = timestamp + std::string(buffer, bytes);
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                if (std::string(buffer, bytes) == "New client connected!\n")
                    messages.emplace_back(full_msg, 7);
                else
                    messages.emplace_back(full_msg, 10);
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
            /*
              Here I separate messages in 'full_msg' and 'full_msg_for_send' to make
              time indicator depend on each person's time zone
            */
            if (key.wVirtualKeyCode == VK_RETURN) {
                std::string full_msg_for_send = "<" + nickname + "> : " + msg + '\n';
                std::string full_msg = get_time() + full_msg_for_send;
                {
                    std::lock_guard<std::mutex> lock(messages_mutex);
                    messages.emplace_back(full_msg, 12);
                }
                socket->write_some(boost::asio::buffer(full_msg_for_send));
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
        SetConsoleTextAttribute(hOut, i.second | 0);
        std::cout << i.first;
    }
}
void Client::draw_input(const std::string& msg) {
    SetConsoleTextAttribute(hOut, 7 | 0);
    std::cout << "\n> " << msg;
}
void Client::draw_raw_input() {
    SetConsoleTextAttribute(hOut, 7 | 0);
    std::cout << "\n> ";
}


