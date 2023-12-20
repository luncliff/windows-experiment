#include "dxgi_helper.hpp"

#include <d3d11_4.h>
#include <d3d11on12.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")

dxgi_helper_t::dxgi_helper_t() noexcept(false) {
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    if (auto hr = CreateDXGIFactory2(flags, __uuidof(IDXGIFactory4), factory.put_void()); FAILED(hr))
        throw winrt::hresult_error{hr};
    get_hardware_adapter(factory.get(), adapter.put());
}

IDXGIFactory4* dxgi_helper_t ::get_factory() noexcept {
    return factory.get();
}

IDXGIAdapter1* dxgi_helper_t ::get_adapter() noexcept {
    return adapter.get();
}

void dxgi_helper_t ::get_hardware_adapter(IDXGIFactory1* factory, IDXGIAdapter1** output,
                                          DXGI_GPU_PREFERENCE preference) noexcept(false) {
    if (factory == nullptr || output == nullptr)
        throw winrt::hresult_invalid_argument{};

    winrt::com_ptr<IDXGIFactory6> factory6 = nullptr;
    if (auto hr = factory->QueryInterface(factory6.put()); FAILED(hr))
        throw winrt::hresult_error{hr};

    winrt::com_ptr<IDXGIAdapter1> adapter = nullptr;
    for (UINT index = 0; SUCCEEDED(
             factory6->EnumAdapterByGpuPreference(index, preference, __uuidof(IDXGIAdapter1), adapter.put_void()));
         ++index) {
        DXGI_ADAPTER_DESC1 desc{};
        if (auto hr = adapter->GetDesc1(&desc); FAILED(hr))
            continue;
        // Basic Render Driver adapter will be skipped
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        // Dry run of DirectX 11 support
        if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            break;
        adapter = nullptr;
    }

    if (adapter != nullptr)
        adapter->QueryInterface(output);
}

HRESULT create_device(IDXGIAdapter* adapter, ID3D11Device** device, ID3D11DeviceContext** context, UINT flags) {
    if (flags == 0)
        flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL level{};
    D3D_FEATURE_LEVEL levels[]{D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    // use D3D11_CREATE_DEVICE_SINGLETHREADED ?
    return D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, 2, //
                             D3D11_SDK_VERSION, device, &level, context);
}
