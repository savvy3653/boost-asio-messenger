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
    void draw_msg();
    void draw_input(const std::string& msg);
    void draw_raw_input();

private:
    std::string nickname{};
    std::uint16_t port{};
    boost::asio::io_context* io_context;
    boost::asio::ip::tcp::acceptor* acceptor;
    std::string msg;
    std::vector<std::string> messages;
    std::mutex messages_mutex;
};
