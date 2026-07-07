#pragma once

class Client {
public:
    Client()
        : io_context(new boost::asio::io_context) {}
    void client_init();
    void read_get(boost::asio::ip::tcp::socket* socket);
    void send_msg(boost::asio::ip::tcp::socket* socket);
    void draw_msg();
    void draw_input(const std::string& msg);
    void draw_raw_input();

private:
    std::string nickname{};
    std::string ip{};
    std::uint16_t port{};
    boost::asio::io_context* io_context;
    std::string msg;
    std::vector<std::pair<std::string, std::uint8_t>> messages; // message, color
    std::mutex messages_mutex;
    HANDLE hOut;
};
