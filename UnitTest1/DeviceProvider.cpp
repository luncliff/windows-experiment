#include "DeviceProvider.h"
#include "DXGIProvider.h"

#include <spdlog/spdlog.h>

#pragma comment(lib, "D3D12")

namespace experiment {

void DeviceProvider::setup_dx11(DWORD flags) noexcept(false) {
    // No. use DX11
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL levels[]{D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    if (auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, 1, D3D11_SDK_VERSION,
                                    d11_device.put(), &level, d11_context.put());
        FAILED(hr))
        winrt::throw_hresult(hr);
}
void DeviceProvider::setup_dx12(DWORD flags) noexcept(false) {
    // DX12 command queue
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        if (auto hr = d12_device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), d12_command_queue.put_void());
            FAILED(hr))
            winrt::throw_hresult(hr);
        d12_command_queue->SetName(L"d12_command_queue");
    }
    // DX11 device on the DX12 device, command queue
    {
        IUnknown* cq = static_cast<IUnknown*>(d12_command_queue.get());
        if (auto hr = D3D11On12CreateDevice(d12_device.get(), flags, nullptr, 0, &cq, 1, 0, d11_device.put(),
                                            d11_context.put(), nullptr);
            FAILED(hr))
            winrt::throw_hresult(hr);
        // query the 11on12 device
        if (auto hr = d11_device->QueryInterface(d11ond12_device.put()); FAILED(hr))
            winrt::throw_hresult(hr);
    }
#if defined(_DEBUG)
    if (auto hr = d12_device->QueryInterface(d11ond12_info.put()); SUCCEEDED(hr)) {
        // Suppress whole categories of messages.
        D3D12_MESSAGE_CATEGORY categories[]{
            D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION,
            D3D12_MESSAGE_CATEGORY_SHADER,
        };
        // Suppress messages based on their severity level.
        D3D12_MESSAGE_SEVERITY severities[]{
            D3D12_MESSAGE_SEVERITY_INFO,
        };
        // Suppress individual messages by their ID.
        D3D12_MESSAGE_ID denyIds[]{
            // This occurs when there are uninitialized descriptors
            // in a descriptor table, even when a shader does not access
            // the missing descriptors.
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumCategories = _countof(categories);
        filter.DenyList.pCategoryList = categories;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        winrt::check_hresult(d11ond12_info->PushStorageFilter(&filter));
    }
#endif
}

DeviceProvider::DeviceProvider(DXGIProvider& dxgi) noexcept(false) {
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if defined(_DEBUG)
    winrt::com_ptr<ID3D12Debug> control = nullptr;
    if (auto hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug), control.put_void()); SUCCEEDED(hr)) {
        control->EnableDebugLayer();
        flags |= D3D11_CREATE_DEVICE_DEBUG;
        spdlog::info("{}: {}", "DeviceProvider", "enable ID3D12Debug");
    }
#endif
    // can we use DX12 device?
    if (FAILED(dxgi.create_device(d12_device.put(), true))) {
        // No. use DX11
        setup_dx11(flags);
        return;
    }
    setup_dx12(flags);
}

bool DeviceProvider::supports_dx12() const noexcept {
    return d12_device != nullptr;
}

void DeviceProvider::get(ID3D12Device** output) noexcept {
    d12_device->QueryInterface(output);
}
void DeviceProvider::get(ID3D12CommandQueue** output) noexcept {
    d12_command_queue->QueryInterface(output);
}
void DeviceProvider::get(ID3D11Device** output) noexcept {
    d11_device->QueryInterface(output);
}
void DeviceProvider::get(ID3D11DeviceContext** output) noexcept {
    d11_context->QueryInterface(output);
}

} // namespace experiment
