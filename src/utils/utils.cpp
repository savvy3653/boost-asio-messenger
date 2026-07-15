#include <string>
#include <codecvt>
#include <locale>
#include <Windows.h>

#include "../../include/utils.h"

#include <memory>

std::string to_utf8(wchar_t wc) {
    if (wc == 0) return "";
    std::wstring wstr(1, wc);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::string get_time() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    std::string hour   = std::to_string(st.wHour);
    std::string minute = std::to_string(st.wMinute);
    std::string second = std::to_string(st.wSecond);
    std::string timestamp = "[" + hour + ":" +
                            (st.wMinute < 10 ? "0" : "") + minute + ":" +
                            (st.wSecond < 10 ? "0" : "") + second + "] ";
    return timestamp;
}

std::unique_ptr<char[]> convert_to_big_endian(std::uint32_t num) {
    auto arr = std::make_unique<char[]>(sizeof(num));
    arr[0] = (num >> 24) & 0xFF;
    arr[1] = (num >> 16) & 0xFF;
    arr[2] = (num >> 8)  & 0xFF;
    arr[3] = num & 0xFF;

    return arr;
}