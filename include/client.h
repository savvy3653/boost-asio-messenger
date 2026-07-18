#pragma once

class Client : public std::enable_shared_from_this<Client> {
public:
    Client()
#ifdef _WIN32
        : io_context(new boost::asio::io_context), hOut(GetStdHandle(STD_OUTPUT_HANDLE)) {}
#else
        : io_context(new boost::asio::io_context) {}
#endif
    ~Client() {
        delete io_context;
    }

    void client_init();
    void read_get(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void send_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

    void send_file(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, const std::string& filepath);
    void async_send_file_data(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, std::shared_ptr<char[]> data, std::uint32_t offset, std::uint32_t file_size);

    void read_msg(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    void read_file(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket, char header[]);

    const void draw_msg();
    const void draw_input(const std::string& msg);
    const void draw_raw_input();
    const void draw_console_msg(const std::string& message);

private:
    std::string nickname{};
    std::string ip{};
    std::uint16_t port{};
    boost::asio::io_context* io_context;

    std::string msg;
    std::vector<std::pair<std::string, std::uint8_t>> messages; // message, color
    std::mutex messages_mutex;
#ifdef _WIN32
    HANDLE hOut;
#endif

    std::uint32_t file_count = 0;
};
