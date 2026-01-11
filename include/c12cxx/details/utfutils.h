#ifndef C12CXX_DETAILS_UTFUTILS_H
#define C12CXX_DETAILS_UTFUTILS_H

#include <string>

namespace c12cxx {

std::string toUtf8(std::u16string_view utf16_sv);

std::u16string toUtf16(std::string_view utf8_sv);

} // namespace c12cxx

#endif // C12CXX_DETAILS_UTFUTILS_H