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
    void accept_client();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void read_get(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void send_msg();
    void draw_msg();
    void draw_input(const std::string& msg);
    void draw_raw_input();

private:
    std::string nickname{};
    std::uint16_t port{};
    
    boost::asio::io_context* io_context;
    boost::asio::ip::tcp::acceptor* acceptor;
    std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> clients;
    std::mutex clients_mutex;

    std::string msg;
    std::vector<std::pair<std::string, std::uint8_t>> messages; // message, color
    std::mutex messages_mutex;
    HANDLE hOut;
};