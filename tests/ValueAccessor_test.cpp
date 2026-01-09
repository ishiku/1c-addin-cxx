#include <c12cxx/details/ValueAccessor.h>

#include "test_utils.h"
#include <c12cxx/details/api/types.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

class ValueAccessorFixture: public ::testing::Test {
protected:
    void TearDown() override { mem.Clear(); }

    TestMemoryManager mem;

    template<typename... Types>
    void expectThrowOnGetValue(tVariant* pvar = nullptr)
    {
        auto visitor = [this, pvar](auto typeTag) {
            c12cxx::ValueAccessor v(pvar);
            EXPECT_THROW((void)v.getValue<decltype(typeTag)>(), std::runtime_error);
        };
        (visitor(Types{}), ...);
    }

    template<typename... Types>
    void expectThrowOnSetValue(tVariant* pvar = nullptr)
    {
        auto visitor = [this, pvar](auto value) {
            c12cxx::ValueAccessor v(pvar);
            EXPECT_THROW((void)v.setValue(value), std::runtime_error);
        };
        (visitor(Types{}), ...);
    }
};

TEST(ValueAccessor, shouldNoThrowInit)
{
    tVariant var;
    TestMemoryManager mem;

    EXPECT_NO_THROW({
        c12cxx::ValueAccessor v1;
        c12cxx::ValueAccessor v2(&var);
        c12cxx::ValueAccessor v3(&var, &mem);
    });
}

TEST_F(ValueAccessorFixture, shouldThrowWhenReadWithoutVar)
{
    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::tm,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(nullptr);
}

TEST_F(ValueAccessorFixture, shouldThrowWhenWriteWithoutVar)
{
    expectThrowOnSetValue<bool,
                          int,
                          double,
                          std::tm,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(nullptr);
}

TEST_F(ValueAccessorFixture, read_BOOL)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    TV_VT(&var) = VTYPE_BOOL;

    var.bVal = true;
    EXPECT_TRUE(v.getValue<bool>());

    var.bVal = false;
    EXPECT_FALSE(v.getValue<bool>());

    expectThrowOnGetValue<int,
                          double,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_I2_I4_ERROR_UI1)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    TV_VT(&var) = VTYPE_I2;
    var.lVal = 777;
    EXPECT_EQ(v.getValue<int>(), var.lVal);
    EXPECT_EQ(v.getValue<double>(), var.lVal);

    TV_VT(&var) = VTYPE_I4;
    EXPECT_EQ(v.getValue<int>(), var.lVal);
    EXPECT_EQ(v.getValue<double>(), var.lVal);

    TV_VT(&var) = VTYPE_ERROR;
    EXPECT_EQ(v.getValue<int>(), var.lVal);
    EXPECT_EQ(v.getValue<double>(), var.lVal);

    TV_VT(&var) = VTYPE_UI1;
    EXPECT_EQ(v.getValue<int>(), var.lVal);
    EXPECT_EQ(v.getValue<double>(), var.lVal);

    expectThrowOnGetValue<bool,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_R4_R8)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    TV_VT(&var) = VTYPE_R8;
    var.dblVal = 777.0;
    EXPECT_EQ(v.getValue<double>(), var.dblVal);
    EXPECT_EQ(v.getValue<int>(), var.dblVal);

    TV_VT(&var) = VTYPE_R4;
    var.dblVal = 777.0;
    EXPECT_EQ(v.getValue<double>(), var.dblVal);
    EXPECT_EQ(v.getValue<int>(), var.dblVal);

    expectThrowOnGetValue<bool,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_TM)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    TV_VT(&var) = VTYPE_TM;
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    var.tmVal = *tm_now;

    EXPECT_EQ(v.getValue<std::tm>(), var.tmVal);
    EXPECT_EQ(v.getValue<std::chrono::system_clock::time_point>(), std::chrono::system_clock::from_time_t(now));

    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_DATE)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    TV_VT(&var) = VTYPE_DATE;
    var.dblVal = 62636712356;
    auto tmVal = v.getValue<std::tm>();
    EXPECT_EQ(tmVal.tm_year, 1985 - 1900);
    EXPECT_EQ(tmVal.tm_mon, 11 - 1);
    EXPECT_EQ(tmVal.tm_mday, 17);
    EXPECT_EQ(tmVal.tm_hour, 22);
    EXPECT_EQ(tmVal.tm_min, 45);
    EXPECT_EQ(tmVal.tm_sec, 56);

    std::time_t timeT = mktime(const_cast<std::tm*>(&tmVal));
    auto tpVal = std::chrono::system_clock::from_time_t(timeT);
    EXPECT_EQ(v.getValue<std::chrono::system_clock::time_point>(), tpVal);

    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_WSTR)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    std::u16string test{u"Test"};
    TV_VT(&var) = VTYPE_PWSTR;
    var.pwstrVal = reinterpret_cast<WCHAR_T*>(test.data());
    var.wstrLen = test.size();
    EXPECT_EQ(v.getValue<std::u16string>(), test);
    EXPECT_EQ(v.getValue<std::u16string_view>(), test);

    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::string,
                          std::string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_STR)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    std::string test{"Test"};
    TV_VT(&var) = VTYPE_PSTR;
    var.pstrVal = reinterpret_cast<char*>(test.data());
    var.strLen = test.size();
    EXPECT_EQ(v.getValue<std::string>(), test);
    EXPECT_EQ(v.getValue<std::string_view>(), test);

    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::u16string,
                          std::u16string_view,
                          std::vector<char>,
                          std::pair<char*, char*>>(&var);
}

TEST_F(ValueAccessorFixture, read_BLOB)
{
    tVariant var;
    c12cxx::ValueAccessor v(&var);

    std::vector<unsigned char> test{1, 2, 3};
    TV_VT(&var) = VTYPE_BLOB;
    var.pstrVal = reinterpret_cast<char*>(test.data());
    var.strLen = test.size();

    auto val = v.getValue<std::vector<unsigned char>>();
    EXPECT_TRUE(std::equal(test.begin(), test.end(), val.begin()));

    auto [begin, end] = v.getValue<std::pair<const char*, const char*>>();
    EXPECT_TRUE(std::equal(begin, end, test.begin()));

    expectThrowOnGetValue<bool,
                          int,
                          double,
                          std::tm,
                          std::chrono::system_clock::time_point,
                          std::u16string,
                          std::u16string_view,
                          std::string,
                          std::string_view>(&var);
}

TEST_F(ValueAccessorFixture, writeBool)
{
    tVariant var;
    tVarInit(&var);

    c12cxx::ValueAccessor v(&var);
    v.setValue(true);
    EXPECT_EQ(TV_VT(&var), VTYPE_BOOL);
    EXPECT_TRUE(var.bVal);
}

TEST_F(ValueAccessorFixture, writeInt)
{
    tVariant var;
    tVarInit(&var);

    c12cxx::ValueAccessor v(&var);
    v.setValue(777);
    EXPECT_EQ(TV_VT(&var), VTYPE_I4);
    EXPECT_EQ(var.lVal, 777);
}

TEST_F(ValueAccessorFixture, writeDouble)
{
    tVariant var;
    tVarInit(&var);

    c12cxx::ValueAccessor v(&var);
    v.setValue(777.0);
    EXPECT_EQ(TV_VT(&var), VTYPE_R8);
    EXPECT_EQ(var.dblVal, 777.0);
}

TEST_F(ValueAccessorFixture, writeTm)
{
    tVariant var;
    tVarInit(&var);

    c12cxx::ValueAccessor v(&var);

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    v.setValue(*tm_now);
    EXPECT_EQ(TV_VT(&var), VTYPE_TM);
    EXPECT_EQ(var.tmVal, *tm_now);
}

TEST_F(ValueAccessorFixture, writeTimePoint)
{
    tVariant var;
    tVarInit(&var);

    c12cxx::ValueAccessor v(&var);

    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&now);
    v.setValue(std::chrono::system_clock::from_time_t(now));
    EXPECT_EQ(TV_VT(&var), VTYPE_TM);
    EXPECT_EQ(var.tmVal, *tm_now);
}

TEST_F(ValueAccessorFixture, writeU16String)
{
    tVariant var;
    tVarInit(&var);

    std::u16string test{u"Test"};

    EXPECT_THROW(
        {
            c12cxx::ValueAccessor v(&var);
            v.setValue(test);
        },
        std::bad_alloc);
    ;

    c12cxx::ValueAccessor v(&var, &mem);
    tVarInit(&var);
    v.setValue(std::u16string_view{test});

    EXPECT_EQ(TV_VT(&var), VTYPE_PWSTR);
    EXPECT_NE(var.pwstrVal, nullptr);
    EXPECT_NE(var.pwstrVal, reinterpret_cast<WCHAR_T*>(test.data()));
    EXPECT_EQ(var.wstrLen, test.size());
    EXPECT_EQ(std::u16string(reinterpret_cast<const char16_t*>(var.pwstrVal), var.wstrLen), test);

    tVarInit(&var);
    v.setValue(std::u16string_view{test});

    EXPECT_EQ(TV_VT(&var), VTYPE_PWSTR);
    EXPECT_NE(var.pwstrVal, nullptr);
    EXPECT_NE(var.pwstrVal, reinterpret_cast<WCHAR_T*>(test.data()));
    EXPECT_EQ(var.wstrLen, test.size());
    EXPECT_EQ(std::u16string(reinterpret_cast<const char16_t*>(var.pwstrVal), var.wstrLen), test);
}

TEST_F(ValueAccessorFixture, writeString)
{
    tVariant var;
    tVarInit(&var);

    std::string test{"Test"};

    EXPECT_THROW(
        {
            c12cxx::ValueAccessor v(&var);
            v.setValue(test);
        },
        std::bad_alloc);
    ;

    c12cxx::ValueAccessor v(&var, &mem);
    v.setValue(test);

    EXPECT_EQ(TV_VT(&var), VTYPE_PSTR);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, test.data());
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(var.pstrVal), var.strLen), test);

    tVarInit(&var);
    v.setValue(std::string_view{test});

    EXPECT_EQ(TV_VT(&var), VTYPE_PSTR);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, test.data());
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(var.pstrVal), var.strLen), test);
}

TEST_F(ValueAccessorFixture, writeVector)
{
    tVariant var;
    tVarInit(&var);

    std::vector<unsigned char> test{1, 2, 3};

    EXPECT_THROW(
        {
            c12cxx::ValueAccessor v(&var);
            v.setValue(test);
        },
        std::bad_alloc);
    ;

    c12cxx::ValueAccessor v(&var, &mem);
    v.setValue(test);

    EXPECT_EQ(TV_VT(&var), VTYPE_BLOB);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, reinterpret_cast<char*>(test.data()));
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pstrVal));

    tVarInit(&var);
    v.setValue(std::make_pair(test.data(), test.data() + test.size()));

    EXPECT_EQ(TV_VT(&var), VTYPE_BLOB);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, reinterpret_cast<char*>(test.data()));
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pstrVal));
}
