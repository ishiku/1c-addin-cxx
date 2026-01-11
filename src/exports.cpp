#include <c12cxx/c12cxx.h>
#include <c12cxx/details/api/ComponentBase.h>
#include <c12cxx/details/api/types.h>

#include <string>

#ifdef _WINDOWS
#    pragma warning(disable : 4311 4302 4267)
#endif

#ifdef __linux__
#    define EXPORT __attribute__((visibility("default")))
#else
#    define EXPORT __declspec(dllexport)
#endif

EXPORT const WCHAR_T* GetClassNames()
{
    try {
        return reinterpret_cast<const WCHAR_T*>(c12cxx::FactoryRegistry::instance().listNames().data()); /*NOLINT*/
    } catch (...) {
        return nullptr;
    }
}

EXPORT long GetClassObject(const WCHAR_T* clsName, IComponentBase** pIntf)
{
    if (*pIntf == nullptr) {
        try {
            auto cls_name = std::u16string(reinterpret_cast<const char16_t*>(clsName)); /*NOLINT*/
            // NOLINT(cppcoreguidelines-owning-memory)
            *pIntf = c12cxx::FactoryRegistry::instance().create(cls_name);
            return reinterpret_cast<long>(*pIntf); /*NOLINT*/
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

EXPORT long DestroyObject(IComponentBase** pIntf)
{
    if (*pIntf == nullptr) {
        return -1;
    }

    delete *pIntf; /*NOLINT*/
    *pIntf = nullptr;
    return 0;
}

EXPORT AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    return eAppCapabilitiesLast;
}
