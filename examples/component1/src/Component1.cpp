#include <c12cxx/c12cxx.h>
#include <string>

class Component1: public c12cxx::Component {
public:
    static constexpr char16_t kComponentName[] = u"Component1";
    std::u16string componentName() final { return kComponentName; };

    Component1() { addMethod(u"Ping", u"Пинг").withHandler(*this, &Component1::Ping); }

private:
    std::u16string str_;
    std::u16string Ping(std::u16string const& str)
    {
        str_ = str;
        return str;
    }
};

REGISTER_COMPONENT(Component1)