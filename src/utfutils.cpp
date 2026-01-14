#include <c12cxx/details/utfutils.h>

#include <iterator>
#include <string>

#include "utf8.h"

namespace c12cxx {
std::string toUtf8(std::u16string_view utf16_sv)
{
    std::string utf8_str;
    utf8::utf16to8(utf16_sv.begin(), utf16_sv.end(), std::back_inserter(utf8_str));
    return utf8_str;
}

std::u16string toUtf16(std::string_view utf8_sv)
{
    std::u16string utf16_str;
    utf8::utf8to16(utf8_sv.begin(), utf8_sv.end(), std::back_inserter(utf16_str));
    return utf16_str;
}


bool isValidUtf8(std::string_view utf8_sv) {
    return utf8::is_valid(utf8_sv.begin(), utf8_sv.end());
}

} // namespace c12cxx
