#include "pch.h"

#include <CppUnitTest.h>

#include "MainWindow.g.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using winrt::UnitTestApp1::MainWindow;

class CppUnitTests : public TestClass<CppUnitTests> {
    MainWindow window{nullptr};

  public:
    TEST_METHOD_INITIALIZE(Initialize) {
        window = nullptr; // todo: add assignment?
    }
    TEST_METHOD_CLEANUP(Cleanup) {
        window = nullptr; // Cleanup logic if needed
    }

    TEST_METHOD(TestWindowIsNull) {
        Assert::IsTrue(window == nullptr);
    }
};
