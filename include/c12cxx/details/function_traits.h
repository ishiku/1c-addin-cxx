#ifndef C12CXX_DETAILS_FUNCTIONTRAITS_H
#define C12CXX_DETAILS_FUNCTIONTRAITS_H

#include <functional>
#include <tuple>
#include <type_traits>

namespace c12cxx {

template<typename T>
struct function_traits;

template<typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename C, typename Ret, typename... Args>
struct function_traits<Ret (C::*)(Args...)> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename C, typename Ret, typename... Args>
struct function_traits<Ret (C::*)(Args...) const> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename C, typename Ret, typename... Args>
struct function_traits<Ret (C::*)(Args...) noexcept> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename C, typename Ret, typename... Args>
struct function_traits<Ret (C::*)(Args...) const noexcept> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    template<std::size_t I>
    using arg_type = std::tuple_element_t<I, args_tuple>;
};

template<typename T>
struct function_traits {
private:
    using method_type = decltype(&T::operator());

public:
    using return_type = typename function_traits<method_type>::return_type;
    using args_tuple = typename function_traits<method_type>::args_tuple;
    static constexpr size_t arity = function_traits<method_type>::arity;
    template<std::size_t I>
    using arg_type = typename std::tuple_element_t<I, args_tuple>;
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_FUNCTIONTRAITS_H
