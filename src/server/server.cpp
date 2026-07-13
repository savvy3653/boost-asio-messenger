#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <codecvt>
#include <chrono>
#include <algorithm>
#include <fstream>

#include "../../include/server.h"
#include "../../include/utils.h"

#define BLACK_BACKGROUND 0
#define RED_COLOR        12
#define GREEN_COLOR      10
#define GREY_COLOR       7

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
                messages.emplace_back(get_time() + connect_message, GREY_COLOR);
            }
            draw_msg();
            draw_input(msg);
            /*
            * Send everyone message about new client:
            */
            for (const auto& client : clients) {
                client->write_some(boost::asio::buffer(connect_message));
            }

            std::thread hc(&Server::handle_client, this, socket);
            hc.detach();
            Beep(1000, 200);
        }
    } catch (std::exception& e) {
        std::cerr << "Client couldn't connect: " << e.what() << '\n';
    }
}

void Server::handle_client(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
    try {
        std::thread rg(&Server::read_get, this, socket);
        rg.join();
    } catch (std::exception& e) {
        std::erase_if(clients, [&](const auto& client) {
            return client == socket;
        });
        socket->close();
        std::cout << "Client disconnected: " << e.what() << '\n';
    }
}

void Server::read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
    try {
        while (true) {
            char header[5];
            socket->read_some(boost::asio::buffer(header, sizeof(header)));

            // checking header for msg type
            switch (header[0]) {
                case M_TXT:
                    read_msg(socket);
                    break;
                case M_FILE:
                    read_file(header);
                    break;
            }
        }
    } catch (std::exception& e) {
        std::erase_if(clients, [&](const auto& client) {
            return client == socket;
        });
        socket->close();
        std::cout << "Client disconnected: " << e.what() << '\n';
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
                if (!msg.size()) continue;
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
                if (msg.find("&send&") != std::string::npos) {
                    std::string filepath = msg.substr(msg.find_last_of("&") + 1);
                    msg.clear();
                    send_file(filepath);
                } else {
                    std::string full_msg_for_send = "<" + nickname + "> : " + msg + '\n';
                    std::string full_msg = get_time() + full_msg_for_send;
                    char header[1];
                    header[0] = static_cast<char>(MessageType::M_TXT);
                    {
                        std::lock_guard<std::mutex> lock(messages_mutex);
                        messages.emplace_back(full_msg, RED_COLOR);
                    }
                    for (const auto& client : clients) {
                        client->write_some(boost::asio::buffer(header));
                    }
                    for (const auto& client : clients) {
                        client->write_some(boost::asio::buffer(full_msg_for_send));
                    }
                    msg.clear();
                }
            }
            draw_msg();
            draw_input(msg);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void Server::send_file(const std::string& filepath) {
    if (filepath.find('.') == std::string::npos) {
        std::cerr << "No file in path!\n";
        return;
    }
    char header[5] = {};
    char buffer[1024] = {};

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    std::string filename = filepath.substr(filepath.find_last_of("\\") + 1);
    std::uint32_t file_size = static_cast<std::uint32_t>(file.tellg());
    file.seekg(0);

    const int block_size = 1024;

    // header
    header[0] = MessageType::M_FILE;
    header[1] = (file_size >> 24) & 0xFF;
    header[2] = (file_size >> 16) & 0xFF;
    header[3] = (file_size >> 8)  & 0xFF;
    header[4] = file_size & 0xFF;
    for (const auto& client : clients) {
        client->write_some(boost::asio::buffer(header, sizeof(header)));
    }

    // while (file) {
        file.read(buffer, block_size);
        std::streamsize bytes_read = file.gcount();

        if (bytes_read > 0) {
            for (const auto& client : clients) {
                client->write_some(boost::asio::buffer(buffer, sizeof(buffer)));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    // }
}

void Server::read_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
    char buffer[2048];
    std::uint16_t bytes = socket->read_some(boost::asio::buffer(buffer, sizeof(buffer)));
    std::string timestamp = get_time();
    std::string full_msg_for_send = std::string(buffer, bytes);
    std::string full_msg = timestamp + std::string(buffer, bytes);
    {
        std::lock_guard<std::mutex> lock(messages_mutex);
        messages.emplace_back(full_msg, GREEN_COLOR);
    }
    /*
     * I redirect every client message to other clients
     */
    for (const auto& client : clients) {
        if (client != socket) {
            client->write_some(boost::asio::buffer(full_msg_for_send));
        }
    }
    draw_msg();
    draw_input(msg);
    Beep(1000, 200);
}

void Server::read_file(char header[]) {
    std::string message = "You recieved a file.\n";
    std::string timestamp = get_time();
    std::string full_msg = timestamp + message;
    {
        std::lock_guard<std::mutex> lock(messages_mutex);
        messages.emplace_back(full_msg, GREY_COLOR);
    }
    draw_msg();
    draw_input(msg);
    Beep(1000, 200);
}

const void Server::draw_msg() {
    system("cls");
    for (const auto& i : messages) {
        SetConsoleTextAttribute(hOut, i.second | BLACK_BACKGROUND);
        std::cout << i.first;
    }
}
const void Server::draw_input(const std::string& msg) {
    SetConsoleTextAttribute(hOut, GREY_COLOR | BLACK_BACKGROUND);
    std::cout << "\n> " << msg;
}
const void Server::draw_raw_input() {
    SetConsoleTextAttribute(hOut, GREY_COLOR | BLACK_BACKGROUND);
    std::cout << "\n> ";
}