#pragma once
#include <cstdint>
#include <memory>

#define BLACK_BACKGROUND 0
#define RED_COLOR        12
#define GREEN_COLOR      10
#define GREY_COLOR       7

#define HEADER_SIZE 32  // type(0), bytes for file length(4), bytes for file extension(27)

enum MessageType : char {
    M_TXT =      0,
    M_FILE =     1,
    M_USR_DATA = 3
};

// struct __attribute__((packed)) Settings {
//     bool receiveTXT;
//     bool receiveFILE;
// };

std::string to_utf8(wchar_t wc);
std::string get_time();
std::unique_ptr<char[]> convert_to_big_endian(std::uint32_t num);
#ifdef __linux__
void LBeep(int Hz, int msec);
#endif
