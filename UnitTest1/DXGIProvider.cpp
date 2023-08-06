#include "DeviceProvider.h"

//#include <microsoft.ui.xaml.media.dxinterop.h> // NOT <windows.ui.xaml.media.dxinterop.h>

#include "winrt_fmt_helper.hpp"
#include <spdlog/spdlog.h>

namespace experiment {

DXGIDeviceManager::DXGIDeviceManager() noexcept(false) {
    if (auto hr = MFCreateDXGIDeviceManager(&token, manager.put()); FAILED(hr)) {
        spdlog::error("{}: {}", "DXGIDeviceManager", winrt::hresult_error{hr});
        winrt::throw_hresult(hr);
    }
    // IID_ID3D11VideoDevice
    const GUID service_id = {0x10EC4D5B, 0x975A, 0x4689, {0xB9, 0xE4, 0xD0, 0xAA, 0xC3, 0x0F, 0xE3, 0x33}};
    if (auto hr = manager->GetVideoService(handle, service_id, video_device.put_void()); FAILED(hr)) {
        switch (hr) {
        case MF_E_DXGI_NEW_VIDEO_DEVICE:
            spdlog::debug("{}: {}", "DXGIDeviceManager", "MF_E_DXGI_NEW_VIDEO_DEVICE");
            manager->CloseDeviceHandle(handle);
            [[fallthrough]];
        case MF_E_DXGI_DEVICE_NOT_INITIALIZED:
        case E_HANDLE:
            spdlog::debug("{}: {}", "DXGIDeviceManager", winrt::hresult_error{hr});
            break;
        default:
            winrt::throw_hresult(hr);
        }
    }
}

DXGIDeviceManager::~DXGIDeviceManager() noexcept {
    if (handle) {
        manager->CloseDeviceHandle(handle);
        handle = 0;
    }
    manager = nullptr;
    token = 0;
    video_device = nullptr;
}

HRESULT DXGIDeviceManager::reset(ID3D11Device* device) noexcept {
    if (handle) {
        spdlog::debug("{}: {}", "DXGIDeviceManager", "CloseDeviceHandle");
        manager->CloseDeviceHandle(handle);
        handle = 0;
    }
    spdlog::debug("{}: {}", "DXGIDeviceManager", "ResetDevice");
    if (auto hr = manager->ResetDevice(device, token); FAILED(hr))
        return hr;
    spdlog::debug("{}: {}", "DXGIDeviceManager", "OpenDeviceHandle");
    return manager->OpenDeviceHandle(&handle);
}

void DXGIProvider::GetHardwareAdapter(IDXGIFactory1* factory, IDXGIAdapter1** output,
                                      DXGI_GPU_PREFERENCE preference) noexcept(false) {
    *output = nullptr;
    winrt::com_ptr<IDXGIFactory6> factory6;
    if (auto hr = factory->QueryInterface(factory6.put()); FAILED(hr))
        winrt::throw_hresult(hr);
    // ...
    winrt::com_ptr<IDXGIAdapter1> adapter = nullptr;
    for (UINT index = 0; SUCCEEDED(
             factory6->EnumAdapterByGpuPreference(index, preference, __uuidof(IDXGIAdapter1), adapter.put_void()));
         ++index) {
        DXGI_ADAPTER_DESC1 desc{};
        if (auto hr = adapter->GetDesc1(&desc); FAILED(hr))
            FAILED(hr);
        // Don't select the Basic Render Driver adapter.
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        // Support DirectX 11?
        if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            break;
        adapter = nullptr;
    }
    if (adapter)
        adapter->QueryInterface(output);
}

HRESULT DXGIProvider::create_device(ID3D12Device** device, bool hardware) noexcept(false) {
    try {
        if (hardware)
            GetHardwareAdapter(factory.get(), adapter.put());
        else if (auto hr = factory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), adapter.put_void()); FAILED(hr)) {
            spdlog::warn("{}: {}", "EnumWarpAdapter", hr);
            winrt::throw_hresult(hr);
        }
        // create device with the adpater
        if (auto hr = D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device)); FAILED(hr))
            winrt::throw_hresult(hr);
    } catch (const winrt::hresult_error& ex) {
        spdlog::error("{}: {}", __func__, winrt::to_string(ex.message()));
        return ex.code();
    }
    return S_OK;
}

HRESULT
DXGIProvider::create_swapchain(const DXGI_SWAP_CHAIN_DESC1& desc, ID3D11Device* device,
                               IDXGISwapChain1** output) noexcept(false) {
    if (auto hr = factory->CreateSwapChainForComposition(device, &desc, nullptr, output); FAILED(hr))
        winrt::throw_hresult(hr);
    spdlog::info("{}: {}", "DXGIProvider", "created swapchain");
    return S_OK;
}

HRESULT
DXGIProvider::create_swapchain(const DXGI_SWAP_CHAIN_DESC1& desc, ID3D11Device* device,
                               IDXGISwapChain3** output) noexcept(false) {
    winrt::com_ptr<IDXGISwapChain1> swapchain1 = nullptr;
    if (auto hr = create_swapchain(desc, device, swapchain1.put()); FAILED(hr))
        return hr;
    return swapchain1->QueryInterface(output);
}

winrt::com_ptr<IDXGIFactory4> DXGIProvider::get_factory() noexcept {
    return factory;
}

winrt::com_ptr<IDXGIAdapter1> DXGIProvider::get_adapter() noexcept {
    return adapter;
}

DXGIProvider::DXGIProvider() noexcept(false) {
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    if (auto hr = CreateDXGIFactory2(flags, __uuidof(IDXGIFactory4), factory.put_void()); FAILED(hr))
        winrt::throw_hresult(hr);
}

} // namespace experiment
