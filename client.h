#pragma once

class Client {
public:
    Client()
        : io_context(new boost::asio::io_context), hOut(GetStdHandle(STD_OUTPUT_HANDLE)) {}
    ~Client() {
        delete io_context;
    }

    void client_init();
    void read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void send_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

    const void draw_msg();
    const void draw_input(const std::string& msg);
    const void draw_raw_input();

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
