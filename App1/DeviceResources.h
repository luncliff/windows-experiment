/**
 * @file DeviceResources.h
 * @brief A wrapper for the Direct3D 12 device and swapchain
 * @see https://github.com/Microsoft/DirectXTK12/wiki/DeviceResources
 * 
 * @copyright Copyright (c) Microsoft Corporation. All rights reserved. Licensed under the MIT License.
 * 
 * @note Changes
 *  - clang-format with the repository style
 *  - Recomment with Doxygen tags
 *  - Use winrt::com_ptr instead of winrt::com_ptr
 *  - Use winrt::handle
 *  - Update header includes 
 *  - GetAdapter receives DXGI_GPU_PREFERENCE
 *  - Move member functions to DeviceResources.cpp file
 */
#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <winrt/base.h>

#define USING_DIRECTX_HEADERS
#include <directxtk12/DirectXHelpers.h> // DirectXTK12
#define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
#include <directx/d3dx12.h> // DirectX Headers

namespace DX {
// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
interface IDeviceNotify {
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

  protected:
    ~IDeviceNotify() = default;
};

// Controls all the DirectX device resources.
class DeviceResources final {
  public:
    static const uint32_t c_AllowTearing = 0x1;
    static const uint32_t c_EnableHDR = 0x2;

    DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
                    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT, UINT backBufferCount = 2,
                    D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0, uint32_t flags = 0) noexcept(false);
    ~DeviceResources();

    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void SetWindow(HWND window, int width, int height);
    bool WindowSizeChanged(int width, int height);
    void HandleDeviceLost();
    void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
    void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT);
    void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
    void WaitForGpu() noexcept;

    // Device Accessors.
    RECT GetOutputSize() const;

    // Direct3D Accessors.
    ID3D12Device* GetD3DDevice() const;
    IDXGISwapChain3* GetSwapChain() const;
    IDXGIFactory4* GetDXGIFactory() const;
    D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const;
    ID3D12Resource* GetRenderTarget() const;
    ID3D12Resource* GetDepthStencil() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    ID3D12CommandAllocator* GetCommandAllocator() const;
    ID3D12GraphicsCommandList* GetCommandList() const;
    DXGI_FORMAT GetBackBufferFormat() const;
    DXGI_FORMAT GetDepthBufferFormat() const;
    D3D12_VIEWPORT GetScreenViewport() const;
    D3D12_RECT GetScissorRect() const;
    UINT GetCurrentFrameIndex() const;
    UINT GetBackBufferCount() const;
    DXGI_COLOR_SPACE_TYPE GetColorSpace() const;

    uint32_t GetDeviceOptions() const;

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

  private:
    // Prepare to render the next frame.
    void MoveToNextFrame();
    // This method acquires the first available hardware adapter that supports Direct3D 12.
    // If no such adapter can be found, try WARP. Otherwise throw an exception.
    void GetAdapter(IDXGIAdapter1** ppAdapter, DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);
    void UpdateColorSpace();

    static const size_t MAX_BACK_BUFFER_COUNT = 3;

    UINT m_backBufferIndex;

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
    UINT64 m_fenceValues[MAX_BACK_BUFFER_COUNT];
    winrt::handle m_fenceEvent;

    // Direct3D rendering objects.
    winrt::com_ptr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    winrt::com_ptr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    UINT m_rtvDescriptorSize;
    D3D12_VIEWPORT m_screenViewport;
    D3D12_RECT m_scissorRect;

    // Direct3D properties.
    DXGI_FORMAT m_backBufferFormat;
    DXGI_FORMAT m_depthBufferFormat;
    UINT m_backBufferCount;
    D3D_FEATURE_LEVEL m_d3dMinFeatureLevel;

    // Cached device properties.
    HWND m_window;
    D3D_FEATURE_LEVEL m_d3dFeatureLevel;
    DWORD m_dxgiFactoryFlags;
    RECT m_outputSize;

    // HDR Support
    DXGI_COLOR_SPACE_TYPE m_colorSpace;

    // DeviceResources options (see flags above)
    uint32_t m_options;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    IDeviceNotify* m_deviceNotify;
};
} // namespace DX
