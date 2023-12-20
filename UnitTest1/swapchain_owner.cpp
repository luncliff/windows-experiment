#include "swapchain_owner.hpp"

#pragma comment(lib, "dxgi.lib")

swapchain_owner_t::swapchain_owner_t(IDXGIFactory2* factory, ID3D11Device* device,
                                     DXGI_SWAP_CHAIN_DESC1& desc) noexcept(false) {
    if (factory == nullptr || device == nullptr)
        throw winrt::hresult_invalid_argument{};
    if (desc.Format == DXGI_FORMAT_UNKNOWN)
        throw winrt::hresult_invalid_argument{};
    if (desc.Width == 0 || desc.Height == 0)
        throw winrt::hresult_invalid_argument{};
    get(desc);
    if (auto hr = factory->CreateSwapChainForComposition(device, &desc, nullptr, swapchain.put()); FAILED(hr))
        throw winrt::hresult_error{hr};
}

IDXGISwapChain1* swapchain_owner_t::get() const noexcept {
    return swapchain.get();
}

void swapchain_owner_t::get(DXGI_SWAP_CHAIN_DESC1& desc) const noexcept {
    if (swapchain) {
        swapchain->GetDesc1(&desc);
        return;
    }
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.Stereo = false;
    desc.Flags = 0;
    // For WindowsStore application
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
}

void swapchain_owner_t::resize(const DXGI_SWAP_CHAIN_DESC1& desc) noexcept(false) {
    if (auto hr = swapchain->ResizeBuffers(2, desc.Width, desc.Height, desc.Format, 0); FAILED(hr))
        throw winrt::hresult_error{hr};
}

void swapchain_owner_t::get(UINT index, ID3D11Texture2D** output) noexcept(false) {
    if (auto hr = swapchain->GetBuffer(index, IID_PPV_ARGS(output)); FAILED(hr))
        throw winrt::hresult_error{hr};
}
