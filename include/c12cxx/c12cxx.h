#ifndef C12CXX_C12CXX_H
#define C12CXX_C12CXX_H

#include <c12cxx/details/Component.h>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace c12cxx {

class FactoryRegistry {
public:
    using FactoryFn = std::function<Component*()>;

    static FactoryRegistry& instance()
    {
        static FactoryRegistry registry;
        return registry;
    }

    void registerFactory(std::u16string const& name, FactoryFn fn)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_[name] = std::move(fn);
    }

    Component* create(const std::u16string& name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            return (it->second)();
        }
        return nullptr;
    }

    const std::u16string& listNames()
    {
        static std::u16string names;
        std::lock_guard<std::mutex> lock(mutex_);

        if (names.empty()) {
            std::basic_ostringstream<char16_t> ss;
            bool first = true;
            for (auto const& el: factories_) {
                if (!first)
                    ss << u";";
                ss << el.first;
                if (first)
                    first = false;
            }
            names = ss.str();
        }
        return names;
    }

private:
    FactoryRegistry() = default;
    std::unordered_map<std::u16string, FactoryFn> factories_;
    std::mutex mutex_;
};

#define REGISTER_COMPONENT(TYPE)                                                                           \
    namespace {                                                                                            \
    struct TYPE##Registrar {                                                                               \
        TYPE##Registrar()                                                                                  \
        {                                                                                                  \
            c12cxx::FactoryRegistry::instance().registerFactory(TYPE::kComponentName,                      \
                                                                []() { return new TYPE(); }); /* NOLINT */ \
        }                                                                                                  \
    };                                                                                                     \
    static const TYPE##Registrar global_##TYPE##_registrar;                                                \
    }

} // namespace c12cxx

#endif // C12CXX_C12CXX_H