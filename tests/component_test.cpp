#include <c12cxx/c12cxx.h>
#include <c12cxx/details/ValueAccessor.h>
#include <c12cxx/details/api/ComponentBase.h>
#include <c12cxx/details/api/IMemoryManager.h>
#include <c12cxx/details/api/types.h>

#include "test_utils.h"
#include <ctime>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

bool& boolData()
{
    static bool data = false;
    return data;
}

bool getBoolData()
{
    return boolData();
}

void setBoolData(bool value)
{
    auto& data = boolData();
    data = value;
}

constexpr char16_t kComponentName[] = u"TestComponent";

class TestComponent final: public c12cxx::Component {
public:
    std::u16string componentName() final { return kComponentName; };

    void procedure(bool& bVal,
                   long& lVal,
                   double& dblVal,
                   std::tm& tmVal,
                   std::u16string& wstrVal,
                   std::string& strVal,
                   std::vector<unsigned char>& binary)
    {
        bVal = !bVal;
        lVal += 1;
        dblVal += 1;
        for (auto& ch: wstrVal)
            ch = u'1';
        for (auto& ch: strVal)
            ch = '1';
        for (auto& ch: binary)
            ch = 1;
    }

    int getIntData() const noexcept { return intData; }

    void setIntData(int value) noexcept { intData = value; }

private:
    int intData{};
};

} // namespace

class TestComponentFixture: public ::testing::Test {
protected:
    TestAddInBase base;
    TestMemoryManager mem;
    std::unique_ptr<IComponentBase> ext;
    TestComponent& component() { return *(dynamic_cast<TestComponent*>(ext.get())); }

    void SetUp() override
    {
        ext.reset(new TestComponent);
        ext->Init(&base);
        ext->setMemManager(&mem);
    }

    void TearDown() override
    {
        ext.release();
        base.Clear();
        mem.Clear();
    }
};

TEST_F(TestComponentFixture, SetLocale)
{
    auto prevLocale = std::locale::global(std::locale());

    ext->SetLocale(reinterpret_cast<const WCHAR_T*>(u"ru_RU.UTF-8"));
    auto curLocale = std::locale::global(prevLocale);

    EXPECT_EQ(curLocale, std::locale("ru_RU.UTF-8"));
}

TEST_F(TestComponentFixture, RegisterExtensionAs)
{
    WCHAR_T* ptr = nullptr;
    ext->RegisterExtensionAs(&ptr);

    std::u16string extName(reinterpret_cast<const char16_t*>(ptr));
    EXPECT_EQ(extName, std::u16string(reinterpret_cast<const char16_t*>(kComponentName)));
}

TEST_F(TestComponentFixture, GetNProps)
{
    const auto initial_n_props = ext->GetNProps();
    EXPECT_EQ(initial_n_props, component().properties().size());

    component().addProperty(u"test1", u"test1");
    component().addProperty(u"test2", u"test2");
    component().addProperty(u"test3", u"test3");

    EXPECT_EQ(initial_n_props + 3, component().properties().size());
    EXPECT_EQ(ext->GetNProps(), component().properties().size());
}

TEST_F(TestComponentFixture, FindProp_withUnknownName)
{
    EXPECT_EQ(ext->FindProp(reinterpret_cast<const WCHAR_T*>(u"unknown property")), -1);
    EXPECT_EQ(ext->FindProp(reinterpret_cast<const WCHAR_T*>(u"unknown property")), -1);
}

TEST_F(TestComponentFixture, FindProp_withKnownName)
{
    std::u16string prop_name{u"TestPropertyName"};
    std::u16string prop_alt{u"TestPropertyAlt"};

    const auto prop_no = component().properties().size();
    component().addProperty(prop_name, prop_alt);

    EXPECT_EQ(ext->FindProp(reinterpret_cast<const WCHAR_T*>(prop_name.data())), prop_no);
    EXPECT_EQ(ext->FindProp(reinterpret_cast<const WCHAR_T*>(prop_alt.data())), prop_no);
}

TEST_F(TestComponentFixture, GetPropName_withUnrealNum)
{
    const auto unreal_num = std::numeric_limits<long>::max();
    EXPECT_EQ(ext->GetPropName(unreal_num, 0), nullptr);
    EXPECT_EQ(ext->GetPropName(unreal_num, unreal_num), nullptr);
}

TEST_F(TestComponentFixture, GetPropName_withCorrectNum)
{
    std::u16string prop_name{u"TestPropertyName"};
    std::u16string prop_alt{u"TestPropertyAlt"};

    const auto prop_no = component().properties().size();
    component().addProperty(prop_name, prop_alt);

    EXPECT_EQ(prop_name, std::u16string(reinterpret_cast<const char16_t*>(ext->GetPropName(prop_no, 0))));
    EXPECT_EQ(prop_alt, std::u16string(reinterpret_cast<const char16_t*>(ext->GetPropName(prop_no, 1))));
}

TEST_F(TestComponentFixture, GetPropVal_withUnrealNum)
{
    const auto unreal_num = std::numeric_limits<long>::max();

    tVariant var;
    EXPECT_FALSE(component().hasError());
    EXPECT_FALSE(ext->GetPropVal(unreal_num, &var));
    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_withoutGetter)
{
    tVariant var;
    const auto prop_no = component().properties().size();

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"test", u"test");
    EXPECT_FALSE(ext->GetPropVal(prop_no, &var));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_bool)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    setBoolData(true);
    EXPECT_TRUE(getBoolData());

    component().addProperty(u"Test", u"Тест").withGetter(getBoolData);
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_BOOL);
    EXPECT_TRUE(var.bVal);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_int)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    component().setIntData(777);
    EXPECT_EQ(component().getIntData(), 777);

    component().addProperty(u"Test", u"Тест").withGetter(component(), &TestComponent::getIntData);
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_I4);
    EXPECT_EQ(var.lVal, 777);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_double)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withGetter([]() { return 100.0; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_R8);
    EXPECT_EQ(var.dblVal, 100.0);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_tm)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    component().addProperty(u"Test", u"Тестовое").withGetter([ret = *local_time]() { return ret; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_TM);
    EXPECT_EQ(var.tmVal, *local_time);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_timePoint)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    auto tp_now = std::chrono::system_clock::from_time_t(now);
    component().addProperty(u"Test", u"Тестовое").withGetter([ret = tp_now]() { return ret; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_TM);
    EXPECT_EQ(var.tmVal, *local_time);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_u16string)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::u16string test{u"Test"};

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_PWSTR);
    EXPECT_NE(var.pwstrVal, nullptr);
    EXPECT_NE(var.pwstrVal, reinterpret_cast<WCHAR_T*>(test.data()));
    EXPECT_EQ(var.wstrLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pwstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_u16string_view)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::u16string_view test{u"Test"};

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_PWSTR);
    EXPECT_NE(var.pwstrVal, nullptr);
    EXPECT_NE(var.pwstrVal, reinterpret_cast<WCHAR_T*>(const_cast<char16_t*>(test.begin())));
    EXPECT_EQ(var.wstrLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pwstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_string)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::string test{"Test"};

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_PSTR);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, reinterpret_cast<char*>(test.data()));
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_string_view)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::string_view test{"Test"};

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_PSTR);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, test.begin());
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_vector)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::vector<unsigned char> test{1, 2, 3};

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_BLOB);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, reinterpret_cast<char*>(test.data()));
    EXPECT_EQ(var.strLen, test.size());
    EXPECT_TRUE(std::equal(test.begin(), test.end(), var.pstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_pair)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    std::vector<unsigned char> vec{1, 2, 3};
    auto test = std::make_pair(vec.data(), vec.data() + vec.size());

    component().addProperty(u"Test", u"Тест").withGetter([test]() { return test; });
    EXPECT_TRUE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_BLOB);
    EXPECT_NE(var.pstrVal, nullptr);
    EXPECT_NE(var.pstrVal, reinterpret_cast<char*>(test.first));
    EXPECT_EQ(var.strLen, test.second - test.first);
    EXPECT_TRUE(std::equal(test.first, test.second, var.pstrVal));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetPropVal_withError)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withGetter([]() {
        throw std::runtime_error("test");
        return false;
    });

    EXPECT_FALSE(ext->GetPropVal(component().properties().size() - 1, &var));
    EXPECT_TRUE(component().hasError());
    EXPECT_EQ(component().errorMessage(), u"test");
}

TEST_F(TestComponentFixture, SetPropVal_withUnrealNum)
{
    const auto unrealNum = std::numeric_limits<long>::max();
    tVariant var;

    EXPECT_FALSE(component().hasError());
    EXPECT_FALSE(ext->SetPropVal(unrealNum, &var));
    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_withoutSetter)
{
    tVariant var;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"test", u"test");
    EXPECT_FALSE(ext->SetPropVal(component().properties().size() - 1, &var));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_bool)
{
    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter(setBoolData);

    setBoolData(false);
    EXPECT_FALSE(getBoolData());

    tVariant var;
    TV_VT(&var) = VTYPE_BOOL;
    var.bVal = true;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));

    EXPECT_TRUE(getBoolData);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_int)
{
    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter(component(), &TestComponent::setIntData);

    component().setIntData(0);
    EXPECT_EQ(component().getIntData(), 0);

    tVariant var;
    TV_VT(&var) = VTYPE_I4;
    var.lVal = 777;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(component().getIntData(), 777);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_double)
{
    using test_type = double;
    test_type value = 0.0;
    test_type test_value = 777;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_R8;
    var.dblVal = test_value;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_tm)
{
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);

    using test_type = std::tm;
    test_type value{};
    test_type test_value = *local_time;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_TM;
    var.tmVal = test_value;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_timePoint)
{
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    auto tp_now = std::chrono::system_clock::from_time_t(now);

    using test_type = std::chrono::system_clock::time_point;
    test_type value{};
    test_type test_value = tp_now;

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_TM;
    var.tmVal = *local_time;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_u16string)
{
    using test_type = std::u16string;
    test_type value{};
    test_type test_value{u"Test"};

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_PWSTR;
    var.pwstrVal = reinterpret_cast<WCHAR_T*>(test_value.data());
    var.wstrLen = test_value.size();
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_u16string_view)
{
    std::u16string test{u"Test"};
    using test_type = std::u16string_view;
    test_type value{};
    test_type test_value{test};

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_PWSTR;
    var.pwstrVal = reinterpret_cast<WCHAR_T*>(test.data());
    var.wstrLen = test.size();
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_string)
{
    using test_type = std::string;
    test_type value{};
    test_type test_value{"Test"};

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_PSTR;
    var.pstrVal = test_value.data();
    var.strLen = test_value.size();
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_string_view)
{
    std::string test{"Test"};
    using test_type = std::string_view;
    test_type value{};
    test_type test_value{test};

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_PSTR;
    var.pstrVal = test.data();
    var.strLen = test.size();
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_EQ(value, test_value);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_vector)
{
    using test_type = std::vector<unsigned char>;
    test_type value{};
    test_type test_value{1, 2, 3};

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_BLOB;
    var.pstrVal = reinterpret_cast<char*>(test_value.data());
    var.strLen = test_value.size();
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_TRUE(std::equal(test_value.begin(), test_value.end(), value.begin()));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_pair)
{
    std::vector<unsigned char> test{1, 2, 3};
    using test_type = std::pair<unsigned char*, unsigned char*>;
    test_type value{};
    test_type test_value = std::make_pair(test.data(), test.data() + test.size());

    EXPECT_FALSE(component().hasError());

    component().addProperty(u"Test", u"Тест").withSetter([&value](test_type val) { value = val; });

    tVariant var;
    TV_VT(&var) = VTYPE_BLOB;
    var.pstrVal = reinterpret_cast<char*>(test_value.first);
    var.strLen = test_value.second - test_value.first;
    EXPECT_TRUE(ext->SetPropVal(component().properties().size() - 1, &var));
    EXPECT_TRUE(std::equal(test_value.first, test_value.second, value.first));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, SetPropVal_error)
{
    EXPECT_FALSE(component().hasError());
    component().addProperty(u"Test", u"Тест").withSetter([](bool val) { throw std::runtime_error("test"); });

    tVariant var;
    EXPECT_FALSE(ext->SetPropVal(component().properties().size() - 1, &var));

    EXPECT_TRUE(component().hasError());
}

TEST_F(TestComponentFixture, GetNMethods)
{
    const auto initial_n_methods = ext->GetNMethods();
    EXPECT_EQ(initial_n_methods, component().methods().size());

    component().addMethod(u"test1", u"test1");
    component().addMethod(u"test2", u"test2");
    component().addMethod(u"test3", u"test3");

    EXPECT_EQ(initial_n_methods + 3, component().methods().size());
    EXPECT_EQ(ext->GetNMethods(), component().methods().size());
}

TEST_F(TestComponentFixture, FindMethod_withUnknownName)
{
    EXPECT_EQ(ext->FindMethod(reinterpret_cast<const WCHAR_T*>(u"unknown property")), -1);
    EXPECT_EQ(ext->FindMethod(reinterpret_cast<const WCHAR_T*>(u"unknown property")), -1);
}

TEST_F(TestComponentFixture, FindMethod_withKnownName)
{
    std::u16string method_name{u"TestName"};
    std::u16string method_alt{u"TestAlt"};

    component().addMethod(method_name, method_alt);

    EXPECT_EQ(ext->FindMethod(reinterpret_cast<const WCHAR_T*>(method_name.data())), component().methods().size() - 1);
    EXPECT_EQ(ext->FindMethod(reinterpret_cast<const WCHAR_T*>(method_alt.data())), component().methods().size() - 1);
}

TEST_F(TestComponentFixture, GetMethodName_withUnrealNum)
{
    const auto unreal_num = std::numeric_limits<long>::max();
    EXPECT_EQ(ext->GetMethodName(unreal_num, 0), nullptr);
    EXPECT_EQ(ext->GetMethodName(unreal_num, unreal_num), nullptr);
}

TEST_F(TestComponentFixture, GetMethodName_withCorrectNum)
{
    std::u16string method_name{u"TestMethodName"};
    std::u16string method_alt{u"TestMethodAlt"};

    component().addMethod(method_name, method_alt);

    EXPECT_EQ(
        method_name,
        std::u16string(reinterpret_cast<const char16_t*>(ext->GetMethodName(component().methods().size() - 1, 0))));
    EXPECT_EQ(
        method_alt,
        std::u16string(reinterpret_cast<const char16_t*>(ext->GetMethodName(component().methods().size() - 1, 1))));
}

TEST_F(TestComponentFixture, GetNParams_withUnrealNum)
{
    EXPECT_EQ(ext->GetNParams(std::numeric_limits<long>::max()), 0);
}

TEST_F(TestComponentFixture, GetNParams)
{
    component().addMethod(u"TestMethod1", u"ТестовыйМетод1").withHandler([]() { });
    EXPECT_EQ(ext->GetNParams(component().methods().size() - 1), 0);

    component().addMethod(u"TestMethod2", u"ТестовыйМетод2").withHandler([](int a) { });
    EXPECT_EQ(ext->GetNParams(component().methods().size() - 1), 1);

    component().addMethod(u"TestMethod3", u"ТестовыйМетод3").withHandler([](int a, int b) { });
    EXPECT_EQ(ext->GetNParams(component().methods().size() - 1), 2);

    component().addMethod(u"TestMethod4", u"ТестовыйМетод4").withHandler([](int a, int b, int c) { });
    EXPECT_EQ(ext->GetNParams(component().methods().size() - 1), 3);
}

TEST_F(TestComponentFixture, GetParamDefValue_withUnrealNum)
{
    const auto unrealNum = std::numeric_limits<long>::max();

    EXPECT_FALSE(component().hasError());

    EXPECT_FALSE(ext->GetParamDefValue(unrealNum, unrealNum, nullptr));

    component().addMethod(u"TestMethod", u"ТестовыйМетод");

    EXPECT_FALSE(ext->GetParamDefValue(component().properties().size() - 1, unrealNum, nullptr));

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, GetParamDefValue)
{
    EXPECT_FALSE(component().hasError());

    component().addMethod(u"TestMethod", u"ТестовыйМетод").withDefaults({{0, true}, {1, 1}, {2, 2.0}});

    tVariant var;

    EXPECT_TRUE(ext->GetParamDefValue(component().methods().size() - 1, 0, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_BOOL);
    EXPECT_TRUE(var.bVal);

    EXPECT_TRUE(ext->GetParamDefValue(component().methods().size() - 1, 1, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_I4);
    EXPECT_EQ(var.lVal, 1);

    EXPECT_TRUE(ext->GetParamDefValue(component().methods().size() - 1, 2, &var));
    EXPECT_EQ(TV_VT(&var), VTYPE_R8);
    EXPECT_EQ(var.dblVal, 2.0);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, HasRetValue_withUnrealNum)
{
    EXPECT_FALSE(ext->HasRetVal(std::numeric_limits<long>::max()));
}

TEST_F(TestComponentFixture, HasRetValue)
{
    component().addMethod(u"TestMethod1", u"ТестовыйМетод1").withHandler([]() { });
    EXPECT_FALSE(ext->HasRetVal(component().methods().size() - 1));

    component().addMethod(u"TestMethod2", u"ТестовыйМетод2").withHandler([]() { return true; });
    EXPECT_TRUE(ext->HasRetVal(component().methods().size() - 1));
}

TEST_F(TestComponentFixture, CallAsProc_withUnrealNum)
{
    EXPECT_FALSE(ext->CallAsProc(std::numeric_limits<long>::max(), nullptr, 0));
}

TEST_F(TestComponentFixture, CallAsProc_withInvalidParamsCount)
{
    constexpr size_t params_count = 3;
    tVariant params[params_count];

    component().addMethod(u"TestMethod", u"ТестовыйМетод").withHandler([](int a, double b) { });
    EXPECT_FALSE(component().hasError());
    EXPECT_FALSE(ext->CallAsProc(component().methods().size() - 1, params, params_count));
    EXPECT_TRUE(component().hasError());
}

TEST_F(TestComponentFixture, CallAsProc)
{
    EXPECT_FALSE(component().hasError());

    component().addMethod(u"TestMethod", u"ТестовыйМетод").withHandler(component(), &TestComponent::procedure);

    constexpr size_t params_count = 7;
    tVariant params[params_count];

    TV_VT(&params[0]) = VTYPE_BOOL;
    params[0].bVal = true;

    TV_VT(&params[1]) = VTYPE_I4;
    params[1].lVal = 2;

    TV_VT(&params[2]) = VTYPE_R8;
    params[2].dblVal = 3.0;

    TV_VT(&params[3]) = VTYPE_TM;

    TV_VT(&params[4]) = VTYPE_PWSTR;
    std::u16string u16Test{u"Test"};
    params[4].pwstrVal = reinterpret_cast<WCHAR_T*>(u16Test.data());
    params[4].wstrLen = u16Test.size();

    TV_VT(&params[5]) = VTYPE_PSTR;
    std::string test{"Test"};
    params[5].pstrVal = test.data();
    params[5].strLen = test.size();

    TV_VT(&params[6]) = VTYPE_BLOB;
    std::vector<unsigned char> vec{1, 2, 3};
    params[6].pstrVal = reinterpret_cast<char*>(vec.data());
    params[6].strLen = vec.size();

    EXPECT_TRUE(ext->CallAsProc(component().methods().size() - 1, params, params_count));

    EXPECT_EQ(TV_VT(&params[0]), VTYPE_BOOL);
    EXPECT_FALSE(params[0].bVal);

    EXPECT_EQ(TV_VT(&params[1]), VTYPE_I4);
    EXPECT_EQ(params[1].lVal, 3);

    EXPECT_EQ(TV_VT(&params[2]), VTYPE_R8);
    EXPECT_EQ(params[2].dblVal, 4.0);

    EXPECT_EQ(TV_VT(&params[4]), VTYPE_PWSTR);
    EXPECT_EQ(params[4].wstrLen, u16Test.size());
    for (auto it = params[4].pwstrVal; it != params[4].pwstrVal + params[4].wstrLen; ++it)
        EXPECT_EQ(*it, u'1');

    EXPECT_EQ(TV_VT(&params[5]), VTYPE_PSTR);
    EXPECT_EQ(params[5].strLen, test.size());
    for (auto it = params[5].pstrVal; it != params[5].pstrVal + params[5].strLen; ++it)
        EXPECT_EQ(*it, '1');

    EXPECT_EQ(TV_VT(&params[6]), VTYPE_BLOB);
    EXPECT_EQ(params[6].strLen, vec.size());
    for (auto it = params[6].pstrVal; it != params[6].pstrVal + params[6].strLen; ++it)
        EXPECT_EQ(*it, 1);

    EXPECT_FALSE(component().hasError());
}

TEST_F(TestComponentFixture, CallAsFunc_withUnrealNum)
{
    EXPECT_FALSE(ext->CallAsFunc(std::numeric_limits<long>::max(), nullptr, nullptr, 0));
}

TEST_F(TestComponentFixture, CallAsFunc_withInvalidParamsCount)
{
    constexpr size_t params_count = 3;
    tVariant params[params_count];
    tVariant ret;

    component().addMethod(u"TestMethod", u"ТестовыйМетод").withHandler([](int a, double b) { return false; });
    EXPECT_FALSE(component().hasError());
    EXPECT_FALSE(ext->CallAsFunc(component().methods().size() - 1, &ret, params, params_count));
    EXPECT_TRUE(component().hasError());
}

TEST_F(TestComponentFixture, CallAsFunc)
{
    EXPECT_FALSE(component().hasError());

    component().addMethod(u"TestMethod", u"ТестовыйМетод").withHandler([](std::u16string const& str) -> std::u16string {
        return str;
    });

    std::u16string test{u"Test"};

    tVariant param;
    TV_VT(&param) = VTYPE_PWSTR;
    param.pwstrVal = reinterpret_cast<WCHAR_T*>(test.data());
    param.wstrLen = test.size();

    tVariant result;

    EXPECT_TRUE(ext->CallAsFunc(component().methods().size() - 1, &result, &param, 1));

    EXPECT_EQ(param.vt, result.vt);
    EXPECT_NE(result.pwstrVal, nullptr);
    EXPECT_EQ(param.wstrLen, result.wstrLen);
    EXPECT_TRUE(std::equal(test.begin(), test.end(), result.pwstrVal));

    EXPECT_FALSE(component().hasError());
}
