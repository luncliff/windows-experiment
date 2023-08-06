#pragma once
#include <d3d11on12.h>
#include <d3d12.h>

#include <winrt/Windows.Foundation.h>

#include "DXGIProvider.h"

namespace experiment {

using winrt::Windows::Foundation::IAsyncAction;

class DeviceProvider final {
    winrt::com_ptr<ID3D12InfoQueue> d11ond12_info = nullptr;
    winrt::com_ptr<ID3D11On12Device> d11ond12_device = nullptr;
    winrt::com_ptr<ID3D12Device> d12_device = nullptr;
    winrt::com_ptr<ID3D12CommandQueue> d12_command_queue = nullptr;
    winrt::com_ptr<ID3D11Device> d11_device = nullptr;
    winrt::com_ptr<ID3D11DeviceContext> d11_context = nullptr;

  public:
    explicit DeviceProvider(DXGIProvider& dxgi) noexcept(false);

    bool supports_dx12() const noexcept;
    void get(ID3D12Device**) noexcept;
    void get(ID3D12CommandQueue**) noexcept;
    void get(ID3D11Device**) noexcept;
    void get(ID3D11DeviceContext**) noexcept;

  private:
    void setup_dx11(DWORD flags) noexcept(false);
    void setup_dx12(DWORD flags) noexcept(false);
};

} // namespace experiment
