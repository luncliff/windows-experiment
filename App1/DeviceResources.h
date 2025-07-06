/**
 * @file DeviceResources.h
 * @brief A wrapper for the Direct3D 12 device and swapchain
 * @see https://github.com/Microsoft/DirectML/blob/master/Samples/DirectMLSuperResolution/DeviceResources.h
 * @see https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingHelloWorld/DeviceResources.h
 * @see https://github.com/Microsoft/DirectXTK12/wiki/DeviceResources

 * @copyright Copyright (c) Microsoft Corporation. All rights reserved. Licensed under the MIT License.
 * 
 * @note Changes
 *  - Run clang-format with the repository style
 *  - Include C++/WinRT headers and DirectX Agility SDK headers
 *  - Remove some member functions
 */
#pragma once
#include <winrt/windows.foundation.h>
#if __has_include(<d3dx12/d3dx12.h>) // DirectX Agility SDK from NuGet
#include <d3dx12/d3dx12.h>
#elif __has_include(<directx/d3dx12.h>) // DirectX Agility SDK from vcpkg
#include <directx/d3dx12.h>
#endif
// clang-format off
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
// clang-format on

namespace DX {

// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
interface IDeviceNotify {
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;
};

// Controls all the DirectX device resources.
class DeviceResources {
  public:
    static constexpr UINT c_AllowTearing = 0x1;
    static constexpr UINT c_RequireTearingSupport = 0x2;

    DeviceResources(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
                    D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0, UINT flags = 0) noexcept(false);
    ~DeviceResources() noexcept;

    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void SetWindow(HWND window, int width, int height) noexcept;
    bool WindowSizeChanged(int width, int height, bool minimized) noexcept;
    void HandleDeviceLost();
    void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept;

    void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT) noexcept;
    void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
    void ExecuteCommandList() noexcept;
    void WaitForGpu() noexcept;

    // Device Accessors.
    RECT GetOutputSize() const noexcept;
    bool IsWindowVisible() const noexcept;
    bool IsTearingSupported() const noexcept;

    // Direct3D Accessors.
    IDXGIAdapter1* GetAdapter() const noexcept;
    ID3D12Device* GetD3DDevice() const noexcept;
    IDXGIFactory4* GetDXGIFactory() const noexcept;
    IDXGISwapChain3* GetSwapChain() const noexcept;
    D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const noexcept;
    ID3D12Resource* GetRenderTarget() const noexcept;
    ID3D12Resource* GetDepthStencil() const noexcept;
    ID3D12CommandQueue* GetCommandQueue() const noexcept;
    ID3D12CommandAllocator* GetCommandAllocator() const noexcept;
    ID3D12GraphicsCommandList* GetCommandList() const noexcept;
    DXGI_FORMAT GetBackBufferFormat() const noexcept;
    DXGI_FORMAT GetDepthBufferFormat() const noexcept;
    D3D12_VIEWPORT GetScreenViewport() const noexcept;
    D3D12_RECT GetScissorRect() const noexcept;
    UINT GetCurrentFrameIndex() const noexcept;
    UINT GetPreviousFrameIndex() const noexcept;
    UINT GetBackBufferCount() const noexcept;
    UINT GetDeviceOptions() const noexcept;

    D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const noexcept;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const noexcept;

  private:
    void MoveToNextFrame();
    void InitializeDXGIAdapter();
    void InitializeAdapter(IDXGIAdapter1** ppAdapter);

    static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;

    // Direct3D properties with default values
    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT m_depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
    UINT m_backBufferCount = 2;
    D3D_FEATURE_LEVEL m_d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    UINT m_adapterIDoverride = UINT_MAX;
    UINT m_backBufferIndex = 0;
    winrt::com_ptr<IDXGIAdapter1> m_adapter;

    // Direct3D objects.
    winrt::com_ptr<ID3D12Device> m_d3dDevice;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
    winrt::com_ptr<ID3D12GraphicsCommandList> m_commandList;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocators[MAX_BACK_BUFFER_COUNT];

    // Swap chain objects.
    winrt::com_ptr<IDXGIFactory4> m_dxgiFactory;
    winrt::com_ptr<IDXGISwapChain3> m_swapChain;
    winrt::com_ptr<ID3D12Resource> m_renderTargets[MAX_BACK_BUFFER_COUNT];
    winrt::com_ptr<ID3D12Resource> m_depthStencil;

    // Presentation fence objects.
    winrt::com_ptr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[MAX_BACK_BUFFER_COUNT]{};
    winrt::handle m_fenceEvent;

    // Direct3D rendering objects.
    winrt::com_ptr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    winrt::com_ptr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    UINT m_rtvDescriptorSize = 0;
    D3D12_VIEWPORT m_screenViewport{};
    D3D12_RECT m_scissorRect{};

    // Cached device properties.
    HWND m_window = nullptr;
    D3D_FEATURE_LEVEL m_d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    RECT m_outputSize{0, 0, 1, 1};
    bool m_isWindowVisible = true;

    // DeviceResources options (see flags above)
    UINT m_options = 0;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    IDeviceNotify* m_deviceNotify = nullptr;
};

} // namespace DX
