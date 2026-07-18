#pragma once
#include "utils.h"

struct Client_ {
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::string ip;
    std::string nickname;
};

class Server : public std::enable_shared_from_this<Server> {
public:
    Server()
#ifdef _WIN32
        : io_context(new boost::asio::io_context), hOut(GetStdHandle(STD_OUTPUT_HANDLE)) {}
#else
        : io_context(new boost::asio::io_context) {}
#endif
    ~Server() {
        delete acceptor;
        delete io_context;
    }

    void server_init();
    void accept_client();
    void handle_client(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void read_user_data(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, char header[]);
    void send_msg();

    void send_file(const std::string& filepath);
    void async_send_file_data(std::shared_ptr<Client_> Client, std::shared_ptr<char[]> data, std::uint32_t offset, std::uint32_t file_size);

    void read_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void read_file(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, char header[]);

    const void draw_msg();
    const void draw_input(const std::string& msg);
    const void draw_raw_input();
    const void draw_console_msg(const std::string& message);

private:
    std::string nickname{};
    std::uint16_t port{};
    
    boost::asio::io_context* io_context;
    boost::asio::ip::tcp::acceptor* acceptor;
    std::vector<std::shared_ptr<Client_>> clients;
    std::mutex clients_mutex;

    std::string msg{};
    std::vector<std::pair<std::string, std::uint8_t>> messages; // message, color
    std::mutex messages_mutex;
#ifdef _WIN32
    HANDLE hOut;
#endif

    std::uint32_t file_count = 0;
};
