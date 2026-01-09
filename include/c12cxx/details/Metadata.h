#ifndef C12CXX_DETAILS_METADATA_H
#define C12CXX_DETAILS_METADATA_H

#include <string>

namespace c12cxx {

class Metadata {
public:
    Metadata() = delete;

    Metadata(std::u16string const& aName, std::u16string const& aAlt): name_(aName), alt_(aAlt) { }

    bool nameIs(std::u16string const& test) const noexcept { return (name_ == test || alt_ == test); }

    const std::u16string& getName() const noexcept { return name_; }

    const std::u16string& getAlt() const noexcept { return alt_; }

private:
    std::u16string name_;
    std::u16string alt_;
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_METADATA_H
