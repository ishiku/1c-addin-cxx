#include <c12cxx/details/MethodWrapper.h>
#include <c12cxx/details/ValueAccessor.h>

#include "test_utils.h"
#include <c12cxx/details/api/types.h>

#include <vector>

#include <gtest/gtest.h>

class MethodWrapperFixture: public ::testing::Test {
protected:
    void TearDown() override { mem.Clear(); }
    TestMemoryManager mem;
};

namespace {
bool inspectMethodTypeFunction()
{
    return true;
}
void inspectMethodTypeProceudre(bool a, int b, double c, char d) { }
} // namespace

TEST_F(MethodWrapperFixture, inspectMethodType)
{
    EXPECT_TRUE(c12cxx::MethodWrapper([]() -> bool { return true; }).isFunction());
    EXPECT_FALSE(c12cxx::MethodWrapper([]() { }).isFunction());
    EXPECT_TRUE(c12cxx::MethodWrapper(inspectMethodTypeFunction).isFunction());
    EXPECT_FALSE(c12cxx::MethodWrapper(inspectMethodTypeProceudre).isFunction());
}

TEST_F(MethodWrapperFixture, inspectMethodParamCount)
{
    EXPECT_EQ(c12cxx::MethodWrapper([]() { }).numberOfParams(), 0);
    EXPECT_EQ(c12cxx::MethodWrapper([](int a) { }).numberOfParams(), 1);
    EXPECT_EQ(c12cxx::MethodWrapper([](bool a, int b) { }).numberOfParams(), 2);
    EXPECT_EQ(c12cxx::MethodWrapper([](bool a, int b, double c) { }).numberOfParams(), 3);
    EXPECT_EQ(c12cxx::MethodWrapper(inspectMethodTypeFunction).numberOfParams(), 0);
    EXPECT_EQ(c12cxx::MethodWrapper(inspectMethodTypeProceudre).numberOfParams(), 4);
}

TEST_F(MethodWrapperFixture, doCall)
{
    constexpr size_t param_count = 7;
    tVariant vars[param_count];
    tVariant ret;

    tVarInit(&vars[0]);
    TV_VT(&vars[0]) = VTYPE_BOOL;
    vars[0].bVal = true;

    tVarInit(&vars[1]);
    TV_VT(&vars[1]) = VTYPE_I4;
    vars[1].lVal = 123;

    tVarInit(&vars[2]);
    TV_VT(&vars[2]) = VTYPE_R8;
    vars[2].lVal = 456.0;

    tVarInit(&vars[3]);
    TV_VT(&vars[3]) = VTYPE_TM;
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    vars[3].tmVal = *tm_now;

    tVarInit(&vars[4]);
    TV_VT(&vars[4]) = VTYPE_PWSTR;
    std::u16string u16TestString{u"Test"};
    vars[4].pwstrVal = reinterpret_cast<WCHAR_T*>(u16TestString.data());
    vars[4].wstrLen = u16TestString.size();

    tVarInit(&vars[5]);
    TV_VT(&vars[5]) = VTYPE_PSTR;
    std::string testString{"Test"};
    vars[5].pstrVal = reinterpret_cast<char*>(testString.data());
    vars[5].strLen = testString.size();

    tVarInit(&vars[6]);
    TV_VT(&vars[6]) = VTYPE_BLOB;
    std::vector<unsigned char> testBlob{1, 2, 3};
    vars[6].pstrVal = reinterpret_cast<char*>(testBlob.data());
    vars[6].strLen = testBlob.size();

    c12cxx::ValueAccessor vRet{&ret, &mem};
    std::vector<c12cxx::ValueAccessor> vParams;
    for (size_t i = 0; i < param_count; ++i)
        vParams.emplace_back(&vars[i], &mem);

    c12cxx::MethodWrapper wrapper{[](bool& aBool,
                                     int& aInt,
                                     double& aDouble,
                                     std::tm& aTm,
                                     std::u16string& aU16String,
                                     std::string& aString,
                                     std::vector<unsigned char>& aVec) -> bool {
        aBool = false;
        aInt = 999;
        aDouble = 888.0;
        aTm.tm_year += 1;
        for (auto& ch: aU16String)
            ch = u'1';
        for (auto& ch: aString)
            ch = '2';
        for (auto& b: aVec)
            b = 3;

        return true;
    }};
    EXPECT_TRUE(wrapper(vRet, vParams));

    EXPECT_EQ(TV_VT(&vars[0]), VTYPE_BOOL);
    EXPECT_FALSE(vars[0].bVal);

    EXPECT_EQ(TV_VT(&vars[1]), VTYPE_I4);
    EXPECT_EQ(vars[1].lVal, 999);

    EXPECT_EQ(TV_VT(&vars[2]), VTYPE_R8);
    EXPECT_EQ(vars[2].dblVal, 888.0);

    EXPECT_EQ(TV_VT(&vars[3]), VTYPE_TM);
    vars[3].tmVal.tm_year -= 1;
    EXPECT_EQ(vars[3].tmVal, *tm_now);

    EXPECT_EQ(TV_VT(&vars[4]), VTYPE_PWSTR);
    EXPECT_NE(vars[4].pwstrVal, nullptr);
    EXPECT_NE(vars[4].pwstrVal, reinterpret_cast<WCHAR_T*>(u16TestString.data()));
    EXPECT_EQ(vars[4].wstrLen, u16TestString.size());
    for (size_t i = 0; i < vars[4].wstrLen; ++i)
        EXPECT_EQ(vars[4].pwstrVal[i], u'1');

    EXPECT_EQ(TV_VT(&vars[5]), VTYPE_PSTR);
    EXPECT_NE(vars[5].pstrVal, nullptr);
    EXPECT_NE(vars[5].pstrVal, reinterpret_cast<char*>(testString.data()));
    EXPECT_EQ(vars[5].strLen, testString.size());
    for (size_t i = 0; i < vars[5].strLen; ++i)
        EXPECT_EQ(vars[5].pstrVal[i], '2');

    EXPECT_EQ(TV_VT(&vars[6]), VTYPE_BLOB);
    EXPECT_NE(vars[6].pstrVal, nullptr);
    EXPECT_NE(vars[6].pstrVal, reinterpret_cast<char*>(testBlob.data()));
    EXPECT_EQ(vars[6].strLen, testBlob.size());
    for (size_t i = 0; i < vars[6].strLen; ++i)
        EXPECT_EQ(vars[6].pstrVal[i], 3);
}