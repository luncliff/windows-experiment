#pragma once
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <evr.h>
#include <mfapi.h>

#include <winrt/Windows.Foundation.h>

namespace experiment {

using winrt::Windows::Foundation::IAsyncAction;

class DXGIDeviceManager final {
    winrt::com_ptr<IMFDXGIDeviceManager> manager = nullptr;
    UINT token = 0;
    HANDLE handle = nullptr;
    winrt::com_ptr<ID3D11VideoDevice> video_device = nullptr;

  public:
    DXGIDeviceManager() noexcept(false);
    ~DXGIDeviceManager() noexcept;

    HRESULT reset(ID3D11Device* device) noexcept;
};

class DXGIProvider final {
    winrt::com_ptr<IDXGIFactory4> factory;
    winrt::com_ptr<IDXGIAdapter1> adapter;

  public:
    DXGIProvider() noexcept(false);

    winrt::com_ptr<IDXGIFactory4> get_factory() noexcept;
    winrt::com_ptr<IDXGIAdapter1> get_adapter() noexcept;

    HRESULT create_device(ID3D12Device** output, bool hardware = true) noexcept(false);
    HRESULT create_swapchain(const DXGI_SWAP_CHAIN_DESC1& desc, ID3D11Device* device,
                             IDXGISwapChain1** outpur) noexcept(false);
    HRESULT create_swapchain(const DXGI_SWAP_CHAIN_DESC1& desc, ID3D11Device* device,
                             IDXGISwapChain3** outpur) noexcept(false);

  private:
    /**
   * @brief Helper function for acquiring the first available hardware adapter
   * @param factory
   * @param output  If no adapter found, it will be set to `nullptr`
   * @param preference
   */
    static void
    GetHardwareAdapter(IDXGIFactory1* factory, IDXGIAdapter1** output,
                       DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE) noexcept(false);
};

} // namespace experiment
