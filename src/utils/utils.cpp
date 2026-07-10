#include <string>
#include <codecvt>
#include <locale>
#include <Windows.h>

#include "../../include/utils.h"

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