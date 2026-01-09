#ifndef C12CXX_DETAILS_METHOD_H
#define C12CXX_DETAILS_METHOD_H

#include <c12cxx/details/Metadata.h>
#include <c12cxx/details/MethodWrapper.h>
#include <c12cxx/details/ValueAccessor.h>
#include <c12cxx/details/api/types.h>

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

namespace c12cxx {

class Method: public Metadata {
public:
    Method() = delete;

    Method(std::u16string const& aName, std::u16string const& aAlt): Metadata(aName, aAlt) { }

    template<typename Handler>
    Method& withHandler(Handler handler)
    {
        MethodWrapper wrapper(handler);
        isFunction_ = wrapper.isFunction();
        numberOfParams_ = wrapper.numberOfParams();
        handler_ = std::move(wrapper);

        return *this;
    }

    template<typename T, typename Ret, typename... Args>
    Method& withHandler(T& obj, Ret (T::*method)(Args...))
    {
        return withHandler([&obj, method](Args... args) -> Ret { return (obj.*method)(std::forward<Args>(args)...); });
    }

    Method& withDefaults(std::unordered_map<long, Variant> const& values)
    {
        defaultValues_ = values;
        return *this;
    }

    bool getParamDefValue(long lParamNum, ValueAccessor valueAccessor) const
    {
        auto it = defaultValues_.find(lParamNum);
        if (it == defaultValues_.end())
            return false;

        std::visit([&valueAccessor](auto const& value) { valueAccessor.setValue(value); }, it->second);

        return true;
    }

    size_t numberOfParams() const noexcept { return numberOfParams_; }

    bool isFunction() const noexcept { return isFunction_; }

    bool doCall(ValueAccessor varRetValue, std::vector<ValueAccessor>& params) const
    {
        if (handler_)
            return handler_(varRetValue, params);

        return false;
    }

private:
    size_t numberOfParams_{};
    bool isFunction_{};
    std::function<bool(ValueAccessor varRetValue, std::vector<ValueAccessor> const& params)> handler_;
    std::unordered_map<long, Variant> defaultValues_;
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_METHOD_H
