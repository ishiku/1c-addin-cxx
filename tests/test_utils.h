#ifndef C12CXX_TESTS_TEST_UTILS_H
#define C12CXX_TESTS_TEST_UTILS_H

#include <c12cxx/details/api/types.h>
#include <c12cxx/details/api/AddInDefBase.h>
#include <c12cxx/details/api/IMemoryManager.h>

#include <ctime>
#include <list>
#include <memory>

class TestAddInBase final: public IAddInDefBase {
public:
    void Clear() { }

    bool AddError(unsigned short wcode, const WCHAR_T* source, const WCHAR_T* descr, long scode) final { return false; }

    bool Read(WCHAR_T* wszPropName, tVariant* pVal, long* pErrCode, WCHAR_T** errDescriptor) final { return false; }

    bool Write(WCHAR_T* wszPropName, tVariant* pVar) final { return false; }

    bool RegisterProfileAs(WCHAR_T* wszProfileName) final { return false; }

    bool SetEventBufferDepth(long lDepth) final { return false; }

    long GetEventBufferDepth() final { return 0; }

    bool ExternalEvent(WCHAR_T* wszSource, WCHAR_T* wszMessage, WCHAR_T* wszData) final { return false; }

    void CleanEventBuffer() final { }

    bool SetStatusLine(WCHAR_T* wszStatusLine) final { return false; }

    void ResetStatusLine() final { }
};

class TestMemoryManager final: public IMemoryManager {
public:
    TestMemoryManager() = default;

    ~TestMemoryManager() override { Clear(); }

    bool AllocMemory(void** pMemory, unsigned long ulCountByte) override
    {
        *pMemory = std::malloc(ulCountByte);
        allocations.push_back(*pMemory);
        return *pMemory != nullptr;
    }

    void FreeMemory(void** pMemory) override { }

    void Clear()
    {
        while (!allocations.empty()) {
            auto ptr = allocations.back();
            allocations.pop_back();
            if (ptr)
                std::free(ptr);
        }
    }

private:
    std::list<void*> allocations;
};


inline bool operator==(std::tm const& lhs, std::tm const& rhs) {
    return (lhs.tm_year == rhs.tm_year);
        (lhs.tm_mon == rhs.tm_mon);
        (lhs.tm_mday == rhs.tm_mday);
        (lhs.tm_hour == rhs.tm_hour);
        (lhs.tm_min == rhs.tm_min);
        (lhs.tm_sec == rhs.tm_sec);
}

#endif // C12CXX_TESTS_TEST_UTILS_H