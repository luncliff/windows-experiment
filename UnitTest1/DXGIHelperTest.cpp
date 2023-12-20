#include <CppUnitTest.h>

#include <winrt/Windows.Foundation.h>

#include "dxgi_helper.hpp"
#include "swapchain_owner.hpp"
#include "texture_renderer.hpp"

using Microsoft::VisualStudio::CppUnitTestFramework::Assert;
using Microsoft::VisualStudio::CppUnitTestFramework::TestClass;

class DXGIHelperTest : public TestClass<DXGIHelperTest> {
    dxgi_helper_t dxgi{};

    TEST_METHOD(TestDXGIAdapter) {
        Assert::IsNotNull(dxgi.get_factory());
        Assert::IsNotNull(dxgi.get_adapter());

        // Don't mix IDXGIAdapter1 and IDXGIAdapter
        winrt::com_ptr<IDXGIAdapter> adapter = nullptr;
        Assert::AreEqual<HRESULT>(dxgi.get_factory()->EnumAdapters(0, adapter.put()), S_OK);
        Assert::IsFalse(dxgi.get_adapter() == adapter.get());
    }

    TEST_METHOD(TestD3D11DeviceFail) {
        // IDXGIAdapter1 will be E_INVALIDARG
        winrt::com_ptr<ID3D11Device> device = nullptr;
        winrt::com_ptr<ID3D11DeviceContext> context = nullptr;
        HRESULT hr = create_device(dxgi.get_adapter(), device.put(), context.put());
        Assert::AreEqual<HRESULT>(hr, E_INVALIDARG);
    }

    TEST_METHOD(TestD3D11Device) {
        winrt::com_ptr<ID3D11Device> device = nullptr;
        winrt::com_ptr<ID3D11DeviceContext> context = nullptr;
        HRESULT hr = create_device(nullptr, device.put(), context.put());
        Assert::AreEqual<HRESULT>(hr, S_OK);
    }
};

class SwapChainOwnerTest : public TestClass<SwapChainOwnerTest> {
    dxgi_helper_t dxgi{};
    winrt::com_ptr<ID3D11Device> device = nullptr;
    winrt::com_ptr<ID3D11DeviceContext> context = nullptr;

    TEST_METHOD(TestCreationFail) {
        Assert::AreEqual<HRESULT>(create_device(nullptr, device.put(), context.put()), S_OK);
        // E_INVALIDARG
        Assert::ExpectException<winrt::hresult_error>([factory = dxgi.get_factory(), device = device]() {
            DXGI_SWAP_CHAIN_DESC1 empty{};
            swapchain_owner_t swapchain{factory, device.get(), empty};
        });
    }

    TEST_METHOD(TestMethod1) {
        Assert::AreEqual<HRESULT>(create_device(nullptr, device.put(), context.put()), S_OK);

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Width = 300;
        desc.Height = 300;
        // the other fields will be updated in the ctor
        swapchain_owner_t swapchain{dxgi.get_factory(), device.get(), desc};
        Assert::IsNotNull(swapchain.get());

        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Width = desc.Height = 256;
        swapchain.resize(desc);

        winrt::com_ptr<ID3D11Texture2D> buffer0 = nullptr;
        swapchain.get(0, buffer0.put());
        Assert::IsNotNull(buffer0.get());

        winrt::com_ptr<ID3D11Texture2D> buffer1 = nullptr;
        swapchain.get(1, buffer1.put());
        Assert::IsNotNull(buffer1.get());
    }
};

class ShaderTest1 : public TestClass<ShaderTest1> {
    dxgi_helper_t dxgi{};
    winrt::com_ptr<ID3D11Device> device = nullptr;
    winrt::com_ptr<ID3D11DeviceContext> context = nullptr;

    TEST_METHOD(TestMethod1) {
        Assert::AreEqual<HRESULT>(create_device(nullptr, device.put(), context.put()), S_OK);

        texture_renderer_t renderer{device.get(), context.get()};
        const auto workspace = std::filesystem::current_path();
        renderer.setup(workspace.native());

        winrt::com_ptr<ID3D11Texture2D> texture0 = nullptr;
        {
            CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_B8G8R8A8_UNORM, 300, 300);
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
            desc.MipLevels = 1;
            Assert::AreEqual<HRESULT>(device->CreateTexture2D(&desc, nullptr, texture0.put()), S_OK);
        }
        winrt::com_ptr<ID3D11ShaderResourceView> input0 = nullptr;
        {
            CD3D11_SHADER_RESOURCE_VIEW_DESC desc{texture0.get(), D3D11_SRV_DIMENSION_TEXTURE2D,
                                                  DXGI_FORMAT_B8G8R8A8_UNORM};
            Assert::AreEqual<HRESULT>(device->CreateShaderResourceView(texture0.get(), &desc, input0.put()), S_OK);
        }

        winrt::com_ptr<ID3D11Texture2D> texture1 = nullptr;
        {
            CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, 300, 300);
            desc.BindFlags = D3D11_BIND_RENDER_TARGET;
            desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.MipLevels = 1;
            Assert::AreEqual<HRESULT>(device->CreateTexture2D(&desc, nullptr, texture1.put()), S_OK);
        }
        winrt::com_ptr<ID3D11RenderTargetView> output0 = nullptr;
        Assert::AreEqual<HRESULT>(device->CreateRenderTargetView(texture1.get(), nullptr, output0.put()), S_OK);

        D3D11_VIEWPORT viewport{};
        viewport.Width = viewport.Height = 300.0f;

        renderer.draw(input0.get(), output0.get(), viewport);
    }
};
