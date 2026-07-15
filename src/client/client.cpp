#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <Windows.h>
#include <codecvt>
#include <fstream>

#include "../../include/client.h"
#include "../../include/utils.h"

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
    auto work_guard = boost::asio::make_work_guard(*io_context);

    std::thread io([this]() {
       io_context->run();
    });
    io.detach();

    std::thread rg(&Client::read_get, this,  socket);
    std::thread sm(&Client::send_msg, this, socket);

    sm.join();
    rg.join();
}

void Client::read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
    try {
        while (true) {
            char header[HEADER_SIZE];
            std::uint16_t bytes = socket->read_some(boost::asio::buffer(header, sizeof(header)));

            // checking header for msg type
            switch (header[0]) {
                case M_TXT:
                    read_msg(socket);
                    break;
                case M_FILE:
                    read_file(socket, header);
                    break;
            }
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
                    // std::thread sf(&Client::send_file, this, socket, filepath);
                    // sf.detach();
                    send_file(socket, filepath);
                } else {
                    std::string full_msg_for_send = "<" + nickname + "> : " + msg + '\n';
                    std::string full_msg = get_time() + full_msg_for_send;
                    char header[1];
                    header[0] = static_cast<char>(MessageType::M_TXT);
                    {
                        std::lock_guard<std::mutex> lock(messages_mutex);
                        messages.emplace_back(full_msg, RED_COLOR);
                    }
                    socket->write_some(boost::asio::buffer(header));
                    socket->write_some(boost::asio::buffer(full_msg_for_send));
                    msg.clear();
                }
            }
            draw_msg();
            draw_input(msg);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void Client::send_file(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, const std::string& filepath) {
    if (filepath.find('.') == std::string::npos) {
        std::cerr << "No file in path!\n";
        return;
    }
    char header[HEADER_SIZE] = {};

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    const std::string filename = filepath.substr(filepath.find_last_of('\\') + 1);
    const std::string extension = filename.substr(filename.find_last_of('.'));
    const std::uint32_t file_size = static_cast<std::uint32_t>(file.tellg());
    std::shared_ptr<char[]> data = std::make_shared<char[]>(file_size);
    file.seekg(0);
    file.read(data.get(), file_size);

    header[0] = MessageType::M_FILE;
    const auto converted = convert_to_big_endian(file_size);
    memcpy(&header[1], &converted[0], 4);
    memcpy(&header[5], extension.data(), extension.size());

    socket->write_some(boost::asio::buffer(header, sizeof(header)));
    async_send_file_data(socket, data, 0, file_size);
}

void Client::async_send_file_data(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, std::shared_ptr<char[]> data, std::uint32_t offset, std::uint32_t file_size) {
    if (offset >= file_size) {
        draw_console_msg("File sent.\n");
        return;
    }

    auto self = shared_from_this();
    std::uint32_t chunk = std::min<std::uint32_t>(8192, file_size - offset);
    boost::asio::async_write(*socket, boost::asio::buffer(&data[offset], chunk),
    [self, data, offset, file_size, socket](boost::system::error_code ec, std::size_t bytes_sent) {
        if (ec) {
            self->draw_console_msg("Error occurred while sending.\n");
            return;
        }
        std::uint32_t new_offset = offset + static_cast<std::uint32_t>(bytes_sent);
        self->async_send_file_data(socket, data, new_offset, file_size);
    });
}

void Client::read_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
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

void Client::read_file(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, char header[]) {
    const std::uint32_t file_size =
                    (static_cast<uint32_t>(static_cast<uint8_t>(header[1])) << 24) |
                    (static_cast<uint32_t>(static_cast<uint8_t>(header[2])) << 16) |
                    (static_cast<uint32_t>(static_cast<uint8_t>(header[3])) << 8)  |
                    (static_cast<uint32_t>(static_cast<uint8_t>(header[4])));
    std::uint32_t remaining = file_size;
    std::string extension;
    for (int i = 5; i < HEADER_SIZE; i++) {
        if (!header[i]) break;
        if (header[i])
            extension += header[i];
    }

    char buffer[8192];
    const size_t BLOCK_SIZE = 8192;
    std::ofstream file("received_" + std::to_string(file_count) + extension, std::ios::binary);

    draw_console_msg("Receiving received_" + std::to_string(file_count) + extension + "(" + std::to_string(file_size) + " bytes)...\n");

    while (remaining > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        size_t to_read = std::min(BLOCK_SIZE, static_cast<size_t>(remaining));

        try {
            size_t bytes_read = socket->read_some(boost::asio::buffer(buffer, to_read));

            if (bytes_read == 0) {
                std::cerr << "Connection closed during transfer.\n";
                break;
            }

            file.write(buffer, bytes_read);
            remaining -= bytes_read;

        } catch (const std::exception& e) {
            std::cerr << "Error receiving file: " << e.what() << std::endl;
            break;
        }
    }
    file.close();

    draw_console_msg("You received a file.\n");
    file_count++;
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

const void Client::draw_console_msg(const std::string& message) {
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


