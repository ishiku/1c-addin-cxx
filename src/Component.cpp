#include <c12cxx/details/Component.h>

#include <cstring>
#include <locale>
#include <string>

#include <c12cxx/details/api/AddInDefBase.h>
#include <c12cxx/details/api/ComponentBase.h>
#include <c12cxx/details/api/IMemoryManager.h>
#include <c12cxx/details/api/types.h>

#include <c12cxx/details/ValueAccessor.h>
#include <c12cxx/details/utfutils.h>

namespace c12cxx {

Component::Component()
{
    addProperty(u"HasError", u"ЕстьОшибка").withGetter(*this, &Component::hasError);
    addProperty(u"ErrorMessage", u"ОписаниеОшибки").withGetter(*this, &Component::errorMessage);
    addMethod(u"ClearError", u"ОчиститьОшибку").withHandler(*this, &Component::clearError);
}

bool Component::Init(void* connection)
{
    connection_ = static_cast<IAddInDefBase*>(connection);

    try {
        onInit();
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("Init: unexpected error.");
    }

    return static_cast<bool>(connection_);
}

bool Component::setMemManager(void* memoryManager)
{
    memoryManager_ = static_cast<IMemoryManager*>(memoryManager);
    return static_cast<bool>(memoryManager_);
}

long Component::GetInfo()
{
    return 2100;
};

void Component::Done()
{
    try {
        onDone();
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("Done: unexpected error.");
    }
}

void Component::SetLocale(const WCHAR_T* locale)
{
    try {
        std::u16string local_name{reinterpret_cast<const char16_t*>(locale)}; /*NOLINT*/
        std::locale::global(std::locale{toUtf8(local_name)});
    } catch (std::runtime_error const& e) {
        std::locale::global(std::locale{""});
        setError(e.what());
    }
}

bool Component::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
    if (wsExtensionName == nullptr)
        return false;

    const size_t size = (componentName().size() + 1) * sizeof(char16_t);
    if ((memoryManager_ == nullptr) || !memoryManager_->AllocMemory(reinterpret_cast<void**>(wsExtensionName), size) || /*NOLINT*/
        *wsExtensionName == nullptr)
        return false;

    std::memcpy(*wsExtensionName, componentName().c_str(), size);

    return true;
}

long Component::GetNProps()
{
    return static_cast<long>(properties_.size());
}

long Component::FindProp(const WCHAR_T* wsPropName)
{
    std::u16string lookup_name{reinterpret_cast<const char16_t*>(wsPropName)}; /*NOLINT*/
    for (size_t i = 0; i < properties_.size(); ++i)
        if (properties_[i].nameIs(lookup_name))
            return static_cast<long>(i);

    return -1;
}

const WCHAR_T* Component::GetPropName(long lPropNum, long lPropAlias)
{
    if (lPropNum >= properties_.size())
        return nullptr;

    auto const& property = properties_[lPropNum];
    std::u16string name = property.getName();
    if (lPropAlias != 0)
        name = property.getAlt();

    WCHAR_T* ptr = nullptr;
    const size_t size = (name.size() + 1) * sizeof(char16_t);
    if ((memoryManager_ == nullptr) || !memoryManager_->AllocMemory(reinterpret_cast<void**>(&ptr), size) || ptr == nullptr) /*NOLINT*/
        return nullptr;

    std::memcpy(ptr, name.c_str(), size);

    return ptr;
}

bool Component::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    if (lPropNum >= properties_.size())
        return false;

    try {
        return properties_[lPropNum].callGetter(ValueAccessor(pvarPropVal, memoryManager_));
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("GetPropVal: unexpected error.");
    }

    return false;
}

bool Component::SetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    if (lPropNum >= properties_.size())
        return false;

    try {
        return properties_[lPropNum].callSetter(ValueAccessor(pvarPropVal));

    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("SetPropVal: unexpected error.");
    }

    return false;
}

bool Component::IsPropReadable(const long lPropNum)
{
    if (lPropNum >= properties_.size())
        return false;

    return properties_[lPropNum].isReadable();
}

bool Component::IsPropWritable(const long lPropNum)
{
    if (lPropNum >= properties_.size())
        return false;

    return properties_[lPropNum].isWritable();
}

long Component::GetNMethods()
{
    return static_cast<long>(methods_.size());
}

long Component::FindMethod(const WCHAR_T* wsMethodName)
{
    std::u16string lookup_name{reinterpret_cast<const char16_t*>(wsMethodName)}; /*NOLINT*/
    for (size_t i = 0; i < methods_.size(); ++i)
        if (methods_[i].nameIs(lookup_name))
            return static_cast<long>(i);

    return -1;
}

const WCHAR_T* Component::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
    if (lMethodNum >= methods_.size())
        return nullptr;

    auto const& method = methods_[lMethodNum];
    std::u16string name = method.getName();
    if (lMethodAlias != 0)
        name = method.getAlt();

    WCHAR_T* ptr = nullptr;
    const size_t size = (name.size() + 1) * sizeof(char16_t);
    if ((memoryManager_ == nullptr) || !memoryManager_->AllocMemory(reinterpret_cast<void**>(&ptr), size) || ptr == nullptr) /*NOLINT*/
        return nullptr;

    std::memcpy(ptr, name.c_str(), size);

    return ptr;
}

long Component::GetNParams(const long lMethodNum)
{
    if (lMethodNum >= methods_.size())
        return 0;

    return static_cast<long>(methods_[lMethodNum].numberOfParams());
}

bool Component::GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue)
{
    if (lMethodNum >= methods_.size())
        return false;

    try {
        return methods_[lMethodNum].getParamDefValue(lParamNum, ValueAccessor(pvarParamDefValue, memoryManager_));
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("GetParamDefValue: unexpected error.");
    }

    return false;
}

bool Component::HasRetVal(const long lMethodNum)
{
    if (lMethodNum >= methods_.size())
        return false;

    return methods_[lMethodNum].isFunction();
}

bool Component::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray)
{
    if (lMethodNum >= methods_.size() || (lSizeArray > 0 && paParams == nullptr))
        return false;

    auto const& method = methods_[lMethodNum];
    if (method.isFunction())
        return false;

    std::vector<ValueAccessor> params;
    params.reserve(lSizeArray);
    for (size_t i = 0; i < lSizeArray; ++i)
        params.emplace_back(&(paParams[i]), memoryManager_);

    try {
        return method.doCall(ValueAccessor(), params);
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("CallAsProp: unexpected error.");
    }

    return false;
}

bool Component::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    if (lMethodNum >= methods_.size() || (lSizeArray > 0 && paParams == nullptr))
        return false;

    auto const& method = methods_[lMethodNum];
    if (!method.isFunction())
        return false;

    std::vector<ValueAccessor> params;
    params.reserve(lSizeArray);
    for (size_t i = 0; i < lSizeArray; ++i)
        params.emplace_back(&(paParams[i]), memoryManager_);

    try {
        return method.doCall(ValueAccessor(pvarRetValue, memoryManager_), params);
    } catch (std::exception const& e) {
        setError(e.what());
    } catch (...) {
        setError("CallAsFunc: unexpected error.");
    }

    return false;
}

void Component::setError(std::string const& msg)
{
    setError(toUtf16(msg));
}

} // namespace c12cxx