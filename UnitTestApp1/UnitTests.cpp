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

    TEST_METHOD(TestRejectIUnknownIID) {
        Assert::AreEqual(factory->CreateInstance(nullptr, __uuidof(IUnknown), nullptr), E_INVALIDARG);

        winrt::com_ptr<IUnknown> service = nullptr;
        Assert::AreEqual(factory->CreateInstance(nullptr, __uuidof(IUnknown), service.put_void()), E_NOINTERFACE);
        Assert::IsNull(service.get());
    }
};

class DeviceResourcesTests : public TestClass<DeviceResourcesTests> {
    winrt::com_ptr<IClassFactory> factory = nullptr;
    winrt::com_ptr<IDeviceResources> resources = nullptr;

  public:
    TEST_METHOD_INITIALIZE(Initialize) {
        HRESULT hr = ::CreateCustomClassFactory(factory.put());
        if (FAILED(hr))
            Assert::Fail(L"CreateCustomClassFactory failed");
        hr = factory->CreateInstance(nullptr, __uuidof(IDeviceResources), resources.put_void());
        Assert::IsTrue(SUCCEEDED(hr));
        Assert::IsNotNull(resources.get());
    }

    TEST_METHOD_CLEANUP(Cleanup) {
        if (resources) {
            resources->WaitForGpu();
            resources = nullptr;
        }
        factory = nullptr; // Cleanup logic if needed
    }

    TEST_METHOD(TestInitializeDevice) {
        // Test basic device initialization with common parameters
        HRESULT hr = resources->InitializeDevice(DXGI_FORMAT_B8G8R8A8_UNORM, // backBufferFormat
                                                 DXGI_FORMAT_D32_FLOAT,      // depthBufferFormat
                                                 2,                          // backBufferCount
                                                 D3D_FEATURE_LEVEL_11_0,     // minFeatureLevel
                                                 0                           // flags
        );
        Assert::AreEqual(hr, S_OK, L"InitializeDevice should succeed with valid parameters");
    }

    TEST_METHOD(TestCreateDeviceResources) {
        // Initialize first
        HRESULT hr = resources->InitializeDevice(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT, 2,
                                                 D3D_FEATURE_LEVEL_11_0, 0);
        Assert::AreEqual(hr, S_OK);

        // Then create device resources
        hr = resources->CreateDeviceResources();
        Assert::AreEqual(hr, S_OK, L"CreateDeviceResources should succeed after initialization");
    }

    TEST_METHOD(TestGetOutputSizeWithoutInit) {
        // Test that getting output size fails when not initialized
        UINT width = 0, height = 0;
        HRESULT hr = resources->GetOutputSize(&width, &height);
        Assert::AreEqual(hr, E_NOT_VALID_STATE, L"GetOutputSize should fail when not initialized");
    }

    TEST_METHOD(TestParameterValidation) {
        // Test null parameter validation
        HRESULT hr = resources->GetOutputSize(nullptr, nullptr);
        Assert::AreEqual(hr, E_INVALIDARG, L"Should reject null parameters");

        BOOL supported;
        hr = resources->IsTearingSupported(nullptr);
        Assert::AreEqual(hr, E_INVALIDARG, L"Should reject null parameters");

        hr = resources->GetD3DDevice(nullptr);
        Assert::AreEqual(hr, E_INVALIDARG, L"Should reject null parameters");

        // we need CreateDeviceResources to create the internal device handle
        winrt::com_ptr<ID3D12Device> device = nullptr;
        hr = resources->GetD3DDevice(device.put());
        Assert::AreEqual(hr, E_NOT_VALID_STATE);
    }

    TEST_METHOD(TestBasicGettersAfterInit) {
        // Initialize and create device resources
        HRESULT hr = resources->InitializeDevice(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT, 2,
                                                 D3D_FEATURE_LEVEL_11_0, 0);
        Assert::AreEqual(hr, S_OK);

        hr = resources->CreateDeviceResources();
        Assert::AreEqual(hr, S_OK);

        // Test format getters
        DXGI_FORMAT backFormat, depthFormat;
        hr = resources->GetBackBufferFormat(&backFormat);
        Assert::AreEqual(hr, S_OK);
        Assert::AreEqual(static_cast<int>(backFormat), static_cast<int>(DXGI_FORMAT_B8G8R8A8_UNORM));

        hr = resources->GetDepthBufferFormat(&depthFormat);
        Assert::AreEqual(hr, S_OK);
        Assert::AreEqual(static_cast<int>(depthFormat), static_cast<int>(DXGI_FORMAT_D32_FLOAT));

        // Test feature level getter
        D3D_FEATURE_LEVEL featureLevel;
        hr = resources->GetDeviceFeatureLevel(&featureLevel);
        Assert::AreEqual(hr, S_OK);
        Assert::IsTrue(featureLevel >= D3D_FEATURE_LEVEL_11_0);

        // Test tearing support getter
        BOOL tearingSupported;
        hr = resources->IsTearingSupported(&tearingSupported);
        Assert::AreEqual(hr, S_OK);
        // Note: actual value depends on hardware/driver support
    }
};
