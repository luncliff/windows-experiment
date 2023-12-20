#pragma once
#include <winrt/Windows.Foundation.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>

HRESULT create_device(IDXGIAdapter* adapter, ID3D11Device** device, ID3D11DeviceContext** context, UINT flags=0);

class dxgi_helper_t final {
    winrt::com_ptr<IDXGIFactory4> factory;
    winrt::com_ptr<IDXGIAdapter1> adapter;

  public:
    dxgi_helper_t() noexcept(false);

    IDXGIFactory4* get_factory() noexcept;
    IDXGIAdapter1* get_adapter() noexcept;

  private:
    static void
    get_hardware_adapter(IDXGIFactory1* factory, IDXGIAdapter1** output,
                         DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE) noexcept(false);
};
