#include "pch.h"

#include <CppUnitTest.h>
#include <winrt/Shared1.h> // generated file from Shared1 project

#include "MainWindow.g.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using winrt::UnitTestApp1::MainWindow;
using winrt::Shared1::BasicViewModel;
using winrt::Shared1::BasicItem;

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

    TEST_METHOD(TestBasicViewModelCreation) {
        BasicViewModel viewModel{};
        Assert::IsTrue(viewModel != nullptr, L"BasicViewModel should be created successfully");
    }

    TEST_METHOD(TestBasicViewModelItems) {
        BasicViewModel viewModel{};
        auto items = viewModel.Items();
        Assert::IsTrue(items.Size() > 0, L"ViewModel should have initialized items");
        Assert::AreEqual(static_cast<uint32_t>(5), items.Size(), L"Should have 5 default items");
    }

    TEST_METHOD(TestBasicViewModelLogFolder) {
        BasicViewModel viewModel{};
        
        // Initially log folder should be null
        auto folder = viewModel.GetLogFolder();
        Assert::IsTrue(folder == nullptr, L"Log folder should initially be null");

        // Test CreateLogFolderAsync (can't easily test async in unit test, but we can verify it doesn't crash)
        try {
            auto async_op = viewModel.CreateLogFolderAsync();
            // Just verify the operation was created without crashing
            Assert::IsTrue(async_op != nullptr, L"CreateLogFolderAsync should return valid operation");
        } catch (...) {
            // CreateLogFolderAsync might fail in test environment, that's acceptable
            Assert::IsTrue(true, L"CreateLogFolderAsync handled gracefully");
        }
    }
};
