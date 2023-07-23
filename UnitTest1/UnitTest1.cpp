#include <CppUnitTest.h>

using Microsoft::VisualStudio::CppUnitTestFramework::Assert;
using Microsoft::VisualStudio::CppUnitTestFramework::TestClass;

class UnitTest1 : public TestClass<UnitTest1> {
    TEST_METHOD(TestMethod1) {
        // ...
        Assert::AreEqual(0, 0);
    }
};
