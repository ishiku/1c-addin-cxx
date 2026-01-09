#ifndef C12CXX_DETAILS_METHODWRAPPER_H
#define C12CXX_DETAILS_METHODWRAPPER_H

#include <c12cxx/details/ValueAccessor.h>
#include <c12cxx/details/function_traits.h>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace c12cxx {

template<typename Handler>
class MethodWrapper {
private:
    Handler handler_;

    template<typename T>
    struct remove_cvrefptr {
        using type = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
    };

    template<typename T>
    using remove_cvrefptr_t = typename remove_cvrefptr<T>::type;

    template<typename Tuple>
    struct remove_cvrefptr_tuple;

    template<typename... Ts>
    struct remove_cvrefptr_tuple<std::tuple<Ts...>> {
        using type = std::tuple<remove_cvrefptr_t<Ts>...>;
    };

    template<typename Tuple>
    using remove_cvrefptr_tuple_t = typename remove_cvrefptr_tuple<Tuple>::type;

public:
    MethodWrapper() = default;

    MethodWrapper(Handler handler): handler_(handler) { }

    bool isFunction() { return !std::is_void_v<typename function_traits<Handler>::return_type>; }

    size_t numberOfParams() { return function_traits<Handler>::arity; }

    bool operator()(ValueAccessor varRetValue, std::vector<ValueAccessor> const& params)
    {
        if (params.size() != numberOfParams())
            throw std::invalid_argument("Invalid number of params.");

        auto args = paramsToArgs(params);

        if constexpr (std::is_void_v<typename function_traits<Handler>::return_type>) {
            std::apply(handler_, args);
        } else {
            auto ret = std::apply(handler_, args);
            varRetValue.setValue(ret);
        }

        updateOutputParams(params, args);
        return true;
    }

private:
    auto paramsToArgs(std::vector<ValueAccessor> const& params)
    {
        using src_args_types = typename function_traits<Handler>::args_tuple;
        using dst_args_types = remove_cvrefptr_tuple_t<src_args_types>;

        return fillTupleFromParams<dst_args_types>(params);
    }

    template<typename Tuple>
    Tuple fillTupleFromParams(std::vector<ValueAccessor> const& params)
    {
        constexpr std::size_t N = std::tuple_size_v<Tuple>;

        if (params.size() < N) {
            throw std::invalid_argument("Vector size too small for tuple");
        }

        return fillTupleFromParamsImpl<Tuple>(params, std::make_index_sequence<N>{});
    }

    template<typename Tuple, std::size_t... Is>
    Tuple fillTupleFromParamsImpl(std::vector<ValueAccessor> const& params, std::index_sequence<Is...>)
    {
        return Tuple{params[Is].getValue<std::tuple_element_t<Is, Tuple>>()...};
    }

    template<typename T>
    void updateOutputParam(ValueAccessor param, T const& value, bool isOutput)
    {
        if (isOutput) {
            param.setValue(value);
        }
    }

    template<typename Tuple, std::size_t... Is>
    void updateOutputParamsImpl(std::vector<ValueAccessor> const& params,
                                Tuple const& tuple,
                                std::index_sequence<Is...>)
    {
        (updateOutputParam(params[Is],
                           std::get<Is>(tuple),
                           (std::is_reference_v<typename function_traits<Handler>::template arg_type<Is>> &&
                            !std::is_const_v<typename function_traits<Handler>::template arg_type<Is>>)),
         ...);
    }

    template<typename Tuple, std::size_t TupSize = std::tuple_size<std::decay_t<Tuple>>::value>
    void updateOutputParams(std::vector<ValueAccessor> const& params, Tuple const& tuple)
    {
        if (params.size() < TupSize)
            throw std::invalid_argument("params list too small");

        updateOutputParamsImpl(params, tuple, std::make_index_sequence<TupSize>{});
    }
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_METHODWRAPPER_H