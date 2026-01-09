#ifndef C12CXX_DETAILS_VALUEACCESSOR_H
#define C12CXX_DETAILS_VALUEACCESSOR_H

#include <c12cxx/details/api/IMemoryManager.h>
#include <c12cxx/details/api/types.h>
#include <c12cxx/details/isocalendar.h>

#include <chrono>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace c12cxx {

using Variant = std::variant<bool,
                             int,
                             double,
                             std::tm,
                             std::chrono::system_clock::time_point,
                             std::string,
                             std::u16string,
                             std::vector<unsigned char>>;

template<typename T>
inline constexpr bool is_byte_pointer_pair_v = false;

template<typename First, typename Second>
inline constexpr bool is_byte_pointer_pair_v<std::pair<First, Second>> =
    std::is_same_v<First, Second> && std::is_pointer_v<First> && sizeof(std::remove_pointer_t<First>) == 1 &&
    (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<First>>, char> ||
     std::is_same_v<std::remove_cv_t<std::remove_pointer_t<First>>, unsigned char> ||
     std::is_same_v<std::remove_cv_t<std::remove_pointer_t<First>>, signed char> ||
     std::is_same_v<std::remove_cv_t<std::remove_pointer_t<First>>, std::byte>);

template<typename T>
struct is_vector: std::false_type { };

template<typename U>
struct is_vector<std::vector<U>>: std::true_type {
    using value_type = U;
};

template<typename T, typename = void>
struct is_byte_vector: std::false_type { };

template<typename T>
struct is_byte_vector<T,
                      std::enable_if_t<is_vector<T>::value && (sizeof(typename is_vector<T>::value_type) == 1) &&
                                       (std::is_integral_v<typename is_vector<T>::value_type> ||
                                        std::is_same_v<typename is_vector<T>::value_type, std::byte>)>>
    : std::true_type { };

template<typename T>
constexpr bool is_byte_vector_v = is_byte_vector<T>::value;

class ValueAccessor {
public:
    explicit ValueAccessor(tVariant* pVar = nullptr, IMemoryManager* memoryManager = nullptr):
        pVar_(pVar),
        memoryManager_(memoryManager)
    { }

    void setValue(bool val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_BOOL;
        pVar_->bVal = val;
    };

    void setValue(int val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_I4;
        pVar_->lVal = val;
    };

    void setValue(long val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_I4;
        pVar_->lVal = val;
    };

    void setValue(double val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_R8;
        pVar_->dblVal = val;
    };

    void setValue(std::tm const& val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_TM;
        pVar_->tmVal = val;
    };

    void setValue(std::chrono::system_clock::time_point val)
    {
        std::time_t t = std::chrono::system_clock::to_time_t(val);
        std::tm* tm_local = std::localtime(&t);
        setValue(*tm_local);
    }

    void setValue(std::u16string_view val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_EMPTY;

        const size_t size = (val.size() + 1) * sizeof(char16_t);
        if (!memoryManager_ || !memoryManager_->AllocMemory(reinterpret_cast<void**>(&pVar_->pwstrVal), size) ||
            (pVar_->pwstrVal == nullptr))
            throw std::bad_alloc();

        memcpy(pVar_->pwstrVal, val.data(), size - sizeof(char16_t));
        pVar_->pwstrVal[val.size()] = 0;
        TV_VT(pVar_) = VTYPE_PWSTR;
        pVar_->wstrLen = val.size();
    };

    //   void setValue(std::u16string const& val) {
    //        setValue(std::u16string_view{val});
    //    }

    void setValue(std::string_view val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_EMPTY;

        const size_t size = (val.size() + 1);
        if (!memoryManager_ || !memoryManager_->AllocMemory(reinterpret_cast<void**>(&pVar_->pstrVal), size) ||
            (pVar_->pstrVal == nullptr))
            throw std::bad_alloc();

        memcpy(pVar_->pstrVal, val.data(), size - 1);
        pVar_->pstrVal[val.size()] = 0;
        TV_VT(pVar_) = VTYPE_PSTR;
        pVar_->strLen = val.size();
    };

    //    void setValue(std::string const& val) {
    //        setValue(std::string_view{val});
    //    }

    template<typename ByteVector>
    std::enable_if_t<is_byte_vector_v<ByteVector>, void> setValue(ByteVector const& val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_EMPTY;

        if (!memoryManager_ || !memoryManager_->AllocMemory(reinterpret_cast<void**>(&pVar_->pstrVal), val.size()) ||
            (pVar_->pstrVal == nullptr))
            throw std::bad_alloc();

        memcpy(pVar_->pstrVal, val.data(), val.size());
        TV_VT(pVar_) = VTYPE_BLOB;
        pVar_->strLen = val.size();
    }

    template<typename BytePointerPair>
    std::enable_if_t<is_byte_pointer_pair_v<BytePointerPair>, void> setValue(BytePointerPair val)
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        tVarInit(pVar_);
        TV_VT(pVar_) = VTYPE_EMPTY;

        if (!memoryManager_ ||
            !memoryManager_->AllocMemory(reinterpret_cast<void**>(&pVar_->pstrVal), val.second - val.first) ||
            (pVar_->pstrVal == nullptr))
            throw std::bad_alloc();

        memcpy(pVar_->pstrVal, val.first, val.second - val.first);
        TV_VT(pVar_) = VTYPE_BLOB;
        pVar_->strLen = val.second - val.first;
    }

    template<typename T>
    T getValue() const
    {
        if (pVar_ == nullptr)
            throw std::runtime_error("Unspecified variable access error.");

        static_assert(!std::is_reference_v<T>, "getValue<T> cannot return references — temporary would be destroyed");

        if constexpr (std::is_same_v<T, bool>) {
            if (TV_VT(pVar_) == VTYPE_BOOL)
                return pVar_->bVal;

        } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            if (TV_VT(pVar_) == VTYPE_I2 || TV_VT(pVar_) == VTYPE_I4 || TV_VT(pVar_) == VTYPE_ERROR ||
                TV_VT(pVar_) == VTYPE_UI1)
                return pVar_->lVal;
            if (TV_VT(pVar_) == VTYPE_R4 || TV_VT(pVar_) == VTYPE_R8)
                return pVar_->dblVal;

        } else if constexpr (std::is_same_v<T, std::tm>) {
            if (TV_VT(pVar_) == VTYPE_TM)
                return pVar_->tmVal;
            if (TV_VT(pVar_) == VTYPE_DATE)
                return secondsToTm(pVar_->dblVal);

        } else if constexpr (std::is_same_v<T, std::chrono::system_clock::time_point>) {
            if (TV_VT(pVar_) == VTYPE_TM)
                return tmToTimePoint(pVar_->tmVal);
            if (TV_VT(pVar_) == VTYPE_DATE)
                return tmToTimePoint(secondsToTm(pVar_->dblVal));

        } else if constexpr (std::is_same_v<T, std::u16string> || std::is_same_v<T, std::u16string_view>) {
            if (TV_VT(pVar_) == VTYPE_PWSTR)
                return T(reinterpret_cast<const char16_t*>(pVar_->pwstrVal), pVar_->wstrLen);

        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
            if (TV_VT(pVar_) == VTYPE_PSTR)
                return T(reinterpret_cast<const char*>(pVar_->pstrVal), pVar_->strLen);

        } else if constexpr (is_byte_vector_v<T>) {
            if (TV_VT(pVar_) == VTYPE_BLOB)
                return T(reinterpret_cast<typename T::value_type*>(pVar_->pstrVal),
                         reinterpret_cast<typename T::value_type*>(pVar_->pstrVal + pVar_->strLen));

        } else if constexpr (is_byte_pointer_pair_v<T>) {
            if (TV_VT(pVar_) == VTYPE_BLOB)
                return std::make_pair(reinterpret_cast<typename T::first_type>(pVar_->pstrVal),
                                      reinterpret_cast<typename T::first_type>(pVar_->pstrVal + pVar_->strLen));
        }

        throw std::runtime_error("Type conversion error.");
    }

private:
    tVariant* pVar_{};
    IMemoryManager* memoryManager_{};

    std::tm secondsToTm(std::int64_t secondsFromEpoch) const
    {
        std::tm ret{};
        int year, month, day;
        ord_to_ymd(secondsFromEpoch / (24 * 60 * 60), &ret.tm_year, &ret.tm_mon, &ret.tm_mday);
        ret.tm_year -= 1900;
        ret.tm_mon -= 1;

        auto seconds = secondsFromEpoch % (24 * 60 * 60);
        ret.tm_hour = seconds / (60 * 60);
        ret.tm_min = (seconds % (60 * 60)) / 60;
        ret.tm_sec = (seconds % (60 * 60)) % 60;
        return ret;
    }

    std::chrono::system_clock::time_point tmToTimePoint(std::tm const& aTm) const
    {
        std::time_t timeT = mktime(const_cast<std::tm*>(&aTm));
        if (timeT == -1)
            return std::chrono::system_clock::time_point();
        return std::chrono::system_clock::from_time_t(timeT);
    }
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_VALUEACCESSOR_H