#ifndef C12CXX_DETAILS_COMPONENT_H
#define C12CXX_DETAILS_COMPONENT_H

#include <c12cxx/details/api/AddInDefBase.h>
#include <c12cxx/details/api/ComponentBase.h>
#include <c12cxx/details/api/IMemoryManager.h>
#include <c12cxx/details/api/types.h>

#include <c12cxx/details/Method.h>
#include <c12cxx/details/Property.h>
#include <c12cxx/details/ValueAccessor.h>

#include <memory>
#include <string>
#include <vector>

namespace c12cxx {

class Component: public IComponentBase {
public:
    Component();

public:
    bool ADDIN_API Init(void* connection) final;

    bool ADDIN_API setMemManager(void* memoryManager) final;

    long ADDIN_API GetInfo() final;

    void ADDIN_API Done() final;

    void ADDIN_API SetLocale(const WCHAR_T* locale) final;

    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) final;

    long ADDIN_API GetNProps() final;

    long ADDIN_API FindProp(const WCHAR_T* wsPropName) final;

    const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) final;

    bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal) final;

    bool ADDIN_API SetPropVal(const long lPropNum, tVariant* pvarPropVal) final;

    bool ADDIN_API IsPropReadable(const long lPropNum) final;

    bool ADDIN_API IsPropWritable(const long lPropNum) final;

    long ADDIN_API GetNMethods() final;

    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) final;

    const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum, const long lMethodAlias) final;

    long ADDIN_API GetNParams(const long lMethodNum) final;

    bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue) final;

    bool ADDIN_API HasRetVal(const long lMethodNum) final;

    bool ADDIN_API CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray) final;

    bool ADDIN_API CallAsFunc(const long lMethodNum,
                              tVariant* pvarRetValue,
                              tVariant* paParams,
                              const long lSizeArray) final;

public:
    Property& addProperty(std::u16string const& name, std::u16string const& alt)
    {
        properties_.emplace_back(name, alt);
        return properties_.back();
    }

    Method& addMethod(std::u16string const& name, std::u16string const& alt)
    {
        methods_.emplace_back(name, alt);
        return methods_.back();
    }

    bool hasError() const noexcept { return !errorMessage_.empty(); }

    std::u16string errorMessage() const { return errorMessage_; }

    const std::vector<Property>& properties() const noexcept { return properties_; }

    const std::vector<Method>& methods() const noexcept { return methods_; }

protected:
    void setError(std::u16string const& msg) { errorMessage_ = msg; }
    void clearError() { errorMessage_.clear(); }

public:
    virtual std::u16string componentName() = 0;

protected:
    virtual void onInit() { }
    virtual void onDone() { }

private:
    std::u16string errorMessage_{};
    void setError(std::string const& msg);

private:
    IAddInDefBase* connection_{};
    IMemoryManager* memoryManager_{};

    std::vector<Property> properties_;
    std::vector<Method> methods_;
};

} // namespace c12cxx

#endif // C12CXX_DETAILS_COMPONENT_H