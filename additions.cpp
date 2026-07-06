#include <string>
#include <codecvt>
#include <locale>

#include "additions.h"

std::string to_utf8(wchar_t wc) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wc);
}