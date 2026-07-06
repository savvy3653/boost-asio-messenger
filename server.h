#pragma once

class Server {
public:
    Server()
        : io_context(new boost::asio::io_context) {}
    ~Server() {
        delete acceptor;
        delete io_context;
    }

    void server_init();
    void connect(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::ip::tcp::socket* socket);
    void read_get(boost::asio::ip::tcp::socket* socket);
    void send_msg(boost::asio::ip::tcp::socket* socket);

private:
    std::string nickname{};
    std::uint16_t port{};
    boost::asio::io_context* io_context;
    boost::asio::ip::tcp::acceptor* acceptor;
};
