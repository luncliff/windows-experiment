#pragma once
// @file: Shared2 PCH - COM interface implementations using C++/WinRT
// Scope: Custom COM interfaces with IClassFactory implementations

// Windows headers first
// clang-format off
#include <windows.h>
#include <unknwn.h>
#include <objbase.h>
#include <combaseapi.h>
#include <hstring.h>
#include <restrictederrorinfo.h>
// clang-format on

// WinRT headers
#undef GetCurrentTime
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

// DirectX headers and libraries
#if __has_include(<d3dx12/d3dx12.h>) // DirectX Agility SDK from NuGet
#include <d3dx12/d3dx12.h>
#elif __has_include(<directx/d3dx12.h>) // DirectX Agility SDK from vcpkg
#include <directx/d3dx12.h>
#endif
#include <dxgidebug.h>

// Include our custom COM interface declarations
#include "Shared2Ifcs.h"

// Standard C++
#include <atomic>
#include <format>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace winrt::Shared2 {

// Interface IDs as constexpr for use with winrt::implements
constexpr winrt::guid IID_IDeviceResources{
    0x23456789, 0x2345, 0x6789, {0xAB, 0xCD, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD}};

// Class IDs for COM registration
constexpr winrt::guid CLSID_DeviceResources{
    0xBCDEF012, 0x3456, 0x789A, {0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A}};

// Forward declarations of implementation classes
struct CDeviceResources;

/**
 * @brief Implementation of IDeviceResources using winrt::implements
 * @details Self-contained DirectX 12 device and resource management
 */
struct CDeviceResources : winrt::implements<CDeviceResources, ::IDeviceResources> {
  private:
    mutable std::mutex m_mutex;

    // Direct3D properties
    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT m_depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
    UINT m_backBufferCount = 2;
    D3D_FEATURE_LEVEL m_d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL m_d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT m_options = 0;

    // Device and factory
    winrt::com_ptr<IDXGIAdapter1> m_adapter;
    winrt::com_ptr<IDXGIFactory4> m_dxgiFactory;
    winrt::com_ptr<ID3D12Device> m_d3dDevice;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
    winrt::com_ptr<ID3D12GraphicsCommandList> m_commandList;

    // Swap chain objects
    winrt::com_ptr<IDXGISwapChain3> m_swapChain;
    static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocators[MAX_BACK_BUFFER_COUNT];
    winrt::com_ptr<ID3D12Resource> m_renderTargets[MAX_BACK_BUFFER_COUNT];
    winrt::com_ptr<ID3D12Resource> m_depthStencil;

    // Descriptor heaps and state
    winrt::com_ptr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    winrt::com_ptr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
    UINT m_backBufferIndex = 0;

    // Fence objects
    winrt::com_ptr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[MAX_BACK_BUFFER_COUNT]{};
    winrt::handle m_fenceEvent;

    // Output and viewport state
    RECT m_outputSize{};
    D3D12_VIEWPORT m_screenViewport{};
    D3D12_RECT m_scissorRect{};
    bool m_isWindowVisible = true;

    // Device notification (simplified interface)
    void* m_deviceNotify = nullptr;

    // Device creation options
    static constexpr UINT c_AllowTearing = 0x1;
    static constexpr UINT c_EnableHDR = 0x2;

    // Helper methods
    void InitializeDXGIAdapter();
    void InitializeAdapter(IDXGIAdapter1** ppAdapter,
                           DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE) noexcept(false);
    void MoveToNextFrame();
    DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt);

  public:
    // IDeviceResources implementation
    HRESULT __stdcall InitializeDevice(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat,
                                       UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel,
                                       UINT flags) noexcept override;
    HRESULT __stdcall CreateDeviceResources() noexcept override;
    HRESULT __stdcall CreateWindowSizeDependentResources(UINT width, UINT height) noexcept override;
    HRESULT __stdcall HandleDeviceLost() noexcept override;
    HRESULT __stdcall GetOutputSize(UINT* pWidth, UINT* pHeight) noexcept override;
    HRESULT __stdcall IsTearingSupported(BOOL* pSupported) noexcept override;
    HRESULT __stdcall GetD3DDevice(ID3D12Device** ppDevice) noexcept override;
    HRESULT __stdcall GetDXGIFactory(IDXGIFactory4** ppFactory) noexcept override;
    HRESULT __stdcall GetSwapChain(IDXGISwapChain3** ppSwapChain) noexcept override;
    HRESULT __stdcall GetCommandQueue(ID3D12CommandQueue** ppCommandQueue) noexcept override;
    HRESULT __stdcall GetDeviceFeatureLevel(D3D_FEATURE_LEVEL* pFeatureLevel) noexcept override;
    HRESULT __stdcall GetBackBufferFormat(DXGI_FORMAT* pFormat) noexcept override;
    HRESULT __stdcall GetDepthBufferFormat(DXGI_FORMAT* pFormat) noexcept override;
    HRESULT __stdcall Prepare(D3D12_RESOURCE_STATES beforeState) noexcept override;
    HRESULT __stdcall Present(D3D12_RESOURCE_STATES beforeState) noexcept override;
    HRESULT __stdcall ExecuteCommandList() noexcept override;
    HRESULT __stdcall WaitForGpu() noexcept override;
};

/**
 * @brief Implementation of ICustomClassFactory using winrt::implements
 * @details Custom class factory that creates ICustomService instances
 */
struct CustomClassFactory : winrt::implements<CustomClassFactory, ::IClassFactory> {
  private:
    std::atomic<ULONG> m_lockCount{0};

  public:
    // IClassFactory implementation
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) noexcept override;
    HRESULT __stdcall LockServer(BOOL fLock) noexcept override;

    // Helper methods
    ULONG GetLockCount() const noexcept;
};

} // namespace winrt::Shared2