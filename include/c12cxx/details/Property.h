#ifndef C12CXX_DETAILS_PROPERTY_H
#define C12CXX_DETAILS_PROPERTY_H

#include <c12cxx/details/Metadata.h>
#include <c12cxx/details/ValueAccessor.h>

#include <c12cxx/details/function_traits.h>
#include <functional>
#include <type_traits>

namespace c12cxx {
/*

template<typename T>
struct setter_traits;

template<typename C, typename Ret, typename Arg>
struct setter_traits<Ret(C::*)(Arg)> {
    using return_type = Ret;
    using arg_type = Arg;
};

template<typename C, typename Ret, typename Arg>
struct setter_traits<Ret(C::*)(Arg) const> {
    using return_type = Ret;
    using arg_type = Arg;
};

template<typename C, typename Ret, typename Arg>
struct setter_traits<Ret(C::*)(Arg) noexcept> {
    using return_type = Ret;
    using arg_type = Arg;
};

template<typename C, typename Ret, typename Arg>
struct setter_traits<Ret(C::*)(Arg) const noexcept> {
    using return_type = Ret;
    using arg_type = Arg;
};

template<typename T>
struct setter_traits{
private:
   using method_type = decltype(&T::operator());
public:
  using return_type = typename setter_traits<method_type>::return_type;
  using arg_type = typename setter_traits<method_type>::arg_type;
};
*/

class Property: public Metadata {
public:
    Property() = delete;

    Property(std::u16string const& aName, std::u16string const& aAlt): Metadata(aName, aAlt) { }

    template<typename Getter>
    Property& withGetter(Getter getter)
    {
        if constexpr (std::is_same_v<std::nullptr_t, Getter>) {
            getter_ = nullptr;
            isReadable_ = false;
        } else {
            static_assert(!std::is_void_v<std::invoke_result_t<Getter>>, "Invalid getter signature, should be T().");
            getter_ = [getter](ValueAccessor varValue) -> bool {
                varValue.setValue(getter());
                return true;
            };
            isReadable_ = true;
        }

        return *this;
    }

    template<typename Class, typename MethodPtr>
    Property& withGetter(Class& obj, MethodPtr method)
    {
        return withGetter([&obj, method]() -> std::invoke_result_t<MethodPtr, Class> { return (obj.*method)(); });
    }

    template<typename Setter>
    Property& withSetter(Setter setter)
    {
        if constexpr (std::is_same_v<std::nullptr_t, Setter>) {
            setter_ = nullptr;
            isWritable_ = false;
        } else {
            static_assert(std::is_void_v<typename function_traits<Setter>::return_type> &&
                              (function_traits<Setter>::arity == 1) &&
                              !std::is_void_v<typename function_traits<Setter>::template arg_type<0>>,
                          "Invalid Setter signature, should be void(T).");
            setter_ = [setter](ValueAccessor varValue) -> bool {
                using arg_type = std::remove_const_t<std::remove_reference_t<
                    std::remove_pointer_t<typename function_traits<Setter>::template arg_type<0>>>>;
                setter(varValue.getValue<arg_type>());
                return true;
            };
            isWritable_ = true;
        }

        return *this;
    }

    template<typename Class, typename ReturnType, typename Arg>
    Property& withSetter(Class& obj, ReturnType (Class::*method)(Arg))
    {
        static_assert(std::is_same_v<ReturnType, void>, "Invalid setter signature, should by void(T).");
        return withSetter([&obj, method](Arg arg) { (obj.*method)(arg); });
    }

    bool isReadable() const noexcept { return isReadable_; }

    bool isWritable() const noexcept { return isWritable_; }

    bool callGetter(ValueAccessor valueAccessor)
    {
        if (getter_)
            return getter_(valueAccessor);

        return false;
    }

    bool callSetter(ValueAccessor valueAccessor)
    {
        if (setter_)
            return setter_(valueAccessor);

        return false;
    }

private:
    bool isReadable_{};
    bool isWritable_{};
    std::function<bool(ValueAccessor)> getter_{nullptr};
    std::function<bool(ValueAccessor)> setter_{nullptr};
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_PROPERTY_H
