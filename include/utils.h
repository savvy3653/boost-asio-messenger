#pragma once
#include <cstdint>

enum MessageType : std::uint8_t {
    M_TXT = 0,
    M_FILE = 1
};

std::string to_utf8(wchar_t wc);
std::string get_time();
