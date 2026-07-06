#pragma once

class Client {
public:
    Client()
        : io_context(new boost::asio::io_context) {}
    void client_init();
    void read_get(boost::asio::ip::tcp::socket* socket);
    void send_msg(boost::asio::ip::tcp::socket* socket);

private:
    std::string nickname{};
    std::string ip{};
    std::uint16_t port{};
    boost::asio::io_context* io_context;
};
