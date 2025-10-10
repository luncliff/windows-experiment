#include "pch.h"

#include <CppUnitTest.h>
#include <winrt/Shared1.h> // generated file from Shared1 project

#include "../Shared2/Shared2Ifcs.h" // COM interface declarations
#include "MainWindow.g.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using winrt::UnitTestApp1::MainWindow;
using winrt::Windows::Foundation::IAsyncAction;

class MainWindowTests : public TestClass<MainWindowTests> {
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

using winrt::Shared1::BasicItem;
using winrt::Shared1::BasicViewModel;

class BasicViewModelTests : public TestClass<BasicViewModelTests> {
    BasicViewModel viewmodel{}; // test with default-consructed instance

  public:
    TEST_METHOD(TestNoLogFolder) {
        try {
            // CreateLogFolderAsync is required for this ViewModel
            std::ignore = viewmodel.GetLogFolderPath();
        } catch (const winrt::hresult_error& ex) {
            Logger::WriteMessage(ex.message().c_str());
            return; // expected failure
        }
        Assert::Fail(L"GetLogFolder must throw expected exception");
    }

    TEST_METHOD(TestNotEmpty) {
        auto items = viewmodel.Items();
        Assert::IsTrue(items.Size() > 0, L"ViewModel should have initialized items");
    }

    TEST_METHOD(TestBasicViewModelLogFolder) {
        // Initially log folder should be null
        auto folder = viewmodel.GetLogFolder();
        Assert::IsTrue(folder == nullptr, L"Log folder should initially be null");

        // Test CreateLogFolderAsync (can't easily test async in unit test, but we can verify it doesn't crash)
        try {
            IAsyncAction action = viewmodel.CreateLogFolderAsync();
            Assert::IsTrue(action != nullptr);

            action.get(); // Wait for completion
        } catch (const winrt::hresult_error& ex) {
            Assert::Fail(ex.message().c_str());
        }
    }
};

class Shared2ClassFactoryTests : public TestClass<Shared2ClassFactoryTests> {
    winrt::com_ptr<IClassFactory> factory = nullptr;

  public:
    TEST_METHOD_INITIALIZE(Initialize) {
        // from Shared2Ifcs.h
        HRESULT hr = ::CreateCustomClassFactory(factory.put());
        if (FAILED(hr))
            Assert::Fail(L"CreateCustomClassFactory failed");
    }
    TEST_METHOD_CLEANUP(Cleanup) {
        factory = nullptr; // Cleanup logic if needed
    }

    TEST_METHOD(TestCreateInstance) {
        winrt::com_ptr<ICustomService> service = nullptr;
        Assert::AreEqual(factory->CreateInstance(nullptr, __uuidof(ICustomService), service.put_void()), S_OK);
        Assert::IsTrue(service != nullptr);
    }
};
