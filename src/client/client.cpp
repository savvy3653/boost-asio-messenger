#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <Windows.h>
#include <codecvt>

#include "../../include/client.h"
#include "../../include/utils.h"

#define BLACK_BACKGROUND 0
#define RED_COLOR        12
#define GREEN_COLOR      10
#define GREY_COLOR       7

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

    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    const boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    try {
        socket->connect(endpoint);
        std::cout << "Connected." << '\n';
        Beep(1000, 200);
    } catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
        return;
    }

    std::thread rg(&Client::read_get, this,  socket);
    std::thread sm(&Client::send_msg, this, socket);

    sm.join();
    rg.join();
}
void Client::read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
    try {
        while (true) {
            char buffer[2048];
            std::uint16_t bytes = socket->read_some(boost::asio::buffer(buffer, sizeof(buffer)));

            std::string timestamp = get_time();
            std::string full_msg = timestamp + std::string(buffer, bytes);
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                if (std::string(buffer, bytes) == "New client connected!\n")
                    messages.emplace_back(full_msg, GREY_COLOR);
                else
                    messages.emplace_back(full_msg, GREEN_COLOR);
            }
            draw_msg();
            draw_input(msg);
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
void Client::send_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
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
                /*
                 * If symbol is UNICODE it has a tail which starts with 0x80 (10xxxxxx)
                 * The symbol itself starts with 0xC0 (110xxxxx)
                 * So we check if we need to erase 2 bytes of unicode instead of one.
                 */
                size_t i = msg.size() - 1;
                while (i > 0 && (msg[i] & 0xC0) == 0x80) {
                    i--;
                }
                msg.erase(i);
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
                    messages.emplace_back(full_msg, RED_COLOR);
                }
                socket->write_some(boost::asio::buffer(full_msg_for_send));
                msg.clear();
            }
            draw_msg();
            draw_input(msg);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

const void Client::draw_msg() {
    system("cls");
    for (const auto& i : messages) {
        SetConsoleTextAttribute(hOut, i.second | BLACK_BACKGROUND);
        std::cout << i.first;
    }
}
const void Client::draw_input(const std::string& msg) {
    SetConsoleTextAttribute(hOut, GREY_COLOR | BLACK_BACKGROUND);
    std::cout << "\n> " << msg;
}
const void Client::draw_raw_input() {
    SetConsoleTextAttribute(hOut, GREY_COLOR | BLACK_BACKGROUND);
    std::cout << "\n> ";
}


