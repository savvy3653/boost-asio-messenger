#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <codecvt>
#include <chrono>

#include "server.h"
#include "additions.h"

void Server::server_init() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

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

    std::thread ac(&Server::accept_client, this);
    std::thread sm(&Server::send_msg, this);

    ac.detach();
    sm.detach();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
void Server::accept_client() {
    try {
        while (true) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);
            acceptor->accept(*socket);
            clients.emplace_back(socket);
            std::string connect_message = "New client connected!\n";
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                messages.emplace_back(get_time() + connect_message, 7);
            }
            draw_msg();
            draw_input(msg);
            /*
            * Send everyone message about new client:
            */
            for (auto client : clients) {
                client->write_some(boost::asio::buffer(connect_message));
            }

            std::thread hc(Server::handle_client, this, socket);
            hc.detach();
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
void Server::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    try {
        std::thread rg(Server::read_get, this, socket);
        rg.join();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
void Server::read_get(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    try {
        while (true) {
            char buffer[2048];
            std::uint16_t bytes = socket->read_some(boost::asio::buffer(buffer, sizeof(buffer)));

            std::string timestamp = get_time();
            std::string full_msg_for_send = std::string(buffer, bytes);
            std::string full_msg = timestamp + std::string(buffer, bytes);
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                messages.emplace_back(full_msg, 10);
            }
            /*
             * I redirect every client message to another clients
             */
            for (auto client : clients) {
                if (client != socket) {
                    client->write_some(boost::asio::buffer(full_msg_for_send));
                }
            }
            draw_msg();
            draw_input(msg);
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
void Server::send_msg() {
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
                for (auto client : clients) {
                    client->write_some(boost::asio::buffer(full_msg_for_send));
                }
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
        SetConsoleTextAttribute(hOut, i.second | 0);
        std::cout << i.first;
    }
}
void Server::draw_input(const std::string& msg) {
    SetConsoleTextAttribute(hOut, 7 | 0);
    std::cout << "\n> " << msg;
}
void Server::draw_raw_input() {
    SetConsoleTextAttribute(hOut, 7 | 0);
    std::cout << "\n> ";
}