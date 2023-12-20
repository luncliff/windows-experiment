#pragma once
#include <winrt/Windows.Foundation.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>

class swapchain_owner_t final {
    winrt::com_ptr<IDXGISwapChain1> swapchain;

  public:
    swapchain_owner_t(IDXGIFactory2* factory, ID3D11Device* device, DXGI_SWAP_CHAIN_DESC1& desc) noexcept(false);

    IDXGISwapChain1* get() const noexcept;

    void get(DXGI_SWAP_CHAIN_DESC1& desc) const noexcept;
    void resize(const DXGI_SWAP_CHAIN_DESC1& desc) noexcept(false);
    void get(UINT index, ID3D11Texture2D** output) noexcept(false);
};
