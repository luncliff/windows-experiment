// https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=vs-2022
#include <CppUnitTest.h>

#include <functional>
#include <spdlog/spdlog.h>
#define USING_DIRECTX_HEADERS
#include <directxtk12/ScreenGrab.h>

#include "DeviceResources.h"
#include "winrt_fmt_helper.hpp"

using Microsoft::VisualStudio::CppUnitTestFramework::Assert;
using Microsoft::VisualStudio::CppUnitTestFramework::TestClass;

class DeviceResourcesTest : public TestClass<DeviceResourcesTest> {
    DX::DeviceResources res{};

    TEST_METHOD_INITIALIZE(Initialize) {
        res.CreateDeviceResources();
        res.WaitForGpu();
    }
    TEST_METHOD_CLEANUP(Cleanup) {
        res.WaitForGpu();
    }

    DXGI_SWAP_CHAIN_DESC1 make_swapchain_desc() {
        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = 512;
        desc.Height = 512;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        return desc;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforcomposition
    TEST_METHOD(TestCreateSwapchain3) {
        winrt::com_ptr<IDXGIFactory4> factory = nullptr;
        Assert::AreEqual(res.GetDXGIFactory()->QueryInterface(factory.put()), S_OK);

        winrt::com_ptr<IDXGISwapChain1> swapchain = nullptr;
        DXGI_SWAP_CHAIN_DESC1 desc = make_swapchain_desc();
        if (HRESULT hr = factory->CreateSwapChainForComposition( //
                res.GetCommandQueue(), &desc, nullptr, swapchain.put());
            FAILED(hr))
            Assert::Fail(winrt::hresult_error{hr}.message().c_str());

        winrt::com_ptr<IDXGISwapChain3> swapchain3 = swapchain.try_as<IDXGISwapChain3>();
        Assert::IsTrue(swapchain3.get());
        Assert::AreEqual<UINT>(swapchain3->GetCurrentBackBufferIndex(), 0);

        DXGI_PRESENT_PARAMETERS params{};
        Assert::AreEqual<HRESULT>(swapchain3->Present1(1, 0, &params), S_OK);
        res.WaitForGpu();

        Assert::AreEqual<UINT>(swapchain3->GetCurrentBackBufferIndex(), 1);
    }

    TEST_METHOD(TestSwapchain1Buffers) {
        winrt::com_ptr<IDXGIFactory4> factory = nullptr;
        Assert::AreEqual(res.GetDXGIFactory()->QueryInterface(factory.put()), S_OK);

        winrt::com_ptr<IDXGISwapChain1> swapchain = nullptr;
        DXGI_SWAP_CHAIN_DESC1 desc = make_swapchain_desc();
        if (HRESULT hr = factory->CreateSwapChainForComposition( //
                res.GetCommandQueue(), &desc, nullptr, swapchain.put());
            FAILED(hr))
            Assert::Fail(winrt::hresult_error{hr}.message().c_str());

        winrt::com_ptr<ID3D12Resource> resource = nullptr;
        resource = winrt::capture<ID3D12Resource>(swapchain, &IDXGISwapChain1::GetBuffer, 0);
        Assert::IsTrue(resource.get());

        resource = nullptr;
        Assert::AreEqual(swapchain->GetBuffer(1, __uuidof(ID3D12Resource), resource.put_void()), S_OK);
    }

    TEST_METHOD(TestInvalidHWND) {
        HWND hwnd = nullptr;
        try {
            res.WindowSizeChanged(hwnd, 0, 0);
            Assert::Fail(L"reached to unreachable code");
        } catch (const std::exception&) {
            // ...
        }
    }

    TEST_METHOD(TestSizeDependentResources) {
        res.CreateSizeDependentResources(512, 512);

        winrt::com_ptr<IDXGISwapChain3> swapchain = {res.GetSwapChain(), winrt::take_ownership_from_abi};
        Assert::IsNotNull(swapchain.get());
        swapchain->AddRef();

        DXGI_SWAP_CHAIN_DESC1 desc{};
        swapchain->GetDesc1(&desc);
        Assert::AreEqual<UINT32>(desc.Format, res.GetBackBufferFormat());
        Assert::AreEqual<UINT>(desc.Width, 512);
        Assert::AreEqual<UINT>(desc.Width, res.GetOutputSize().cx);
        Assert::AreEqual<UINT>(desc.Height, 512);
        Assert::AreEqual<UINT>(desc.Height, res.GetOutputSize().cy);

        res.CreateSizeDependentResources(256, 512);
        swapchain->GetDesc1(&desc);
        Assert::AreEqual<UINT>(desc.Width, 256);
        Assert::AreEqual<UINT>(desc.Width, res.GetOutputSize().cx);
        Assert::AreEqual<UINT>(desc.Height, 512);
        Assert::AreEqual<UINT>(desc.Height, res.GetOutputSize().cy);
    }
};
