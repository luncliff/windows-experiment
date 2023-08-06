#include <CppUnitTest.h>
#include <spdlog/spdlog.h>

// WindowsApp SDK headers
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>

#include "DXGIProvider.h"
#include "DeviceProvider.h"
#include "winrt_fmt_helper.hpp"

namespace fs = std::filesystem;

using experiment::DeviceProvider;
using experiment::DXGIProvider;
using Microsoft::VisualStudio::CppUnitTestFramework::Assert;
using Microsoft::VisualStudio::CppUnitTestFramework::TestClass;
using winrt::Windows::Foundation::IAsyncAction;
using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::Foundation::Uri;
using winrt::Windows::Storage::StorageFile;
using winrt::Windows::Storage::Streams::Buffer;
using winrt::Windows::Storage::Streams::InputStreamOptions;

namespace experiment {

IAsyncAction read_cso(StorageFile file, Buffer& output) noexcept(false) {
    using winrt::Windows::Storage::Streams::InputStreamOptions;
    using winrt::Windows::Storage::Streams::IRandomAccessStream;
    IRandomAccessStream stream = co_await file.OpenReadAsync();
    auto capacity = static_cast<uint32_t>(stream.Size());
    Buffer buf(capacity);
    co_await stream.ReadAsync(buf, capacity, InputStreamOptions::None);
    output = buf;
}

IAsyncAction make_shader_from_cso(StorageFile file, ID3D11Device* device, ID3D11VertexShader** output) noexcept(false) {
    Buffer buf = nullptr;
    co_await read_cso(file, buf);
    if (auto hr = device->CreateVertexShader(buf.data(), buf.Length(), nullptr, output); FAILED(hr)) {
        spdlog::error("{}: {}", __func__, file.Path());
        winrt::throw_hresult(hr);
    }
}

IAsyncAction make_shader_from_cso(StorageFile file, ID3D11Device* device, ID3D11PixelShader** output) noexcept(false) {
    Buffer buf = nullptr;
    co_await read_cso(file, buf);
    if (auto hr = device->CreatePixelShader(buf.data(), buf.Length(), nullptr, output); FAILED(hr)) {
        spdlog::error("{}: {}", __func__, file.Path());
        winrt::throw_hresult(hr);
    }
}

} // namespace experiment

class DeviceProviderTest : public TestClass<DeviceProviderTest> {
    DXGIProvider dxgi{};
    DeviceProvider devices{dxgi};

    TEST_METHOD_INITIALIZE(Initialize) {
        Assert::IsNotNull(dxgi.get_adapter().get());
        Assert::IsNotNull(dxgi.get_factory().get());
    }

    TEST_METHOD(TestDeviceIfDX12) {
        if (devices.supports_dx12() == false)
            return;

        winrt::com_ptr<ID3D12Device> device = nullptr;
        devices.get(device.put());
        Assert::IsNotNull(device.get());

        winrt::com_ptr<ID3D12CommandQueue> command_queue = nullptr;
        devices.get(command_queue.put());
        Assert::IsNotNull(command_queue.get());
    }
};

class CompiledShaderTest : public TestClass<CompiledShaderTest> {
    DXGIProvider dxgi{};
    DeviceProvider devices{dxgi};
    fs::path folder{};

    TEST_METHOD_INITIALIZE(Initialize) {
        folder = fs::current_path();
        spdlog::info("{}", folder);
        Assert::IsFalse(folder.empty());
    }

    IAsyncAction try_shader_creation(const fs::path& cso, ID3D11VertexShader** output) {
        using experiment::make_shader_from_cso;
        winrt::com_ptr<ID3D11Device> device = nullptr;
        devices.get(device.put());
        winrt::hstring fpath{cso.native()};
        StorageFile file = co_await StorageFile::GetFileFromPathAsync(fpath);
        co_await make_shader_from_cso(file, device.get(), output);
    }

    IAsyncAction try_shader_creation(const fs::path& cso, ID3D11PixelShader** output) {
        using experiment::make_shader_from_cso;
        winrt::com_ptr<ID3D11Device> device = nullptr;
        devices.get(device.put());
        winrt::hstring fpath{cso.native()};
        StorageFile file = co_await StorageFile::GetFileFromPathAsync(fpath);
        co_await make_shader_from_cso(file, device.get(), output);
    }

    TEST_METHOD(TestVertexShaders) {
        winrt::com_ptr<ID3D11VertexShader> vs = nullptr;
        try {
            for (auto name : {"QuadVertexShader.cso"}) {
                auto cso = (folder / name).make_preferred();
                vs = nullptr;
                try_shader_creation(cso, vs.put()).get();
                Assert::IsNotNull(vs.get());
            }
        } catch (const winrt::hresult_error& ex) {
            Assert::Fail(ex.message().c_str());
        }
    }

    TEST_METHOD(TestPixelShaders) {
        winrt::com_ptr<ID3D11PixelShader> ps = nullptr;
        try {
            for (auto name : {"QuadPixelShaderNV12.cso", "QuadPixelShaderRGB.cso"}) {
                auto cso = (folder / name).make_preferred();
                ps = nullptr;
                try_shader_creation(cso, ps.put()).get();
                Assert::IsNotNull(ps.get());
            }
        } catch (const winrt::hresult_error& ex) {
            Assert::Fail(ex.message().c_str());
        }
    }
};
