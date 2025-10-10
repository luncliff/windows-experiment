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

// Include our custom COM interface declarations
#include "Shared2Ifcs.h"

// Include Shared1 DeviceResources
#include "../Shared1/DeviceResources.h"

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
constexpr winrt::guid IID_ICustomService{0x12345678, 0x1234, 0x5678, {0x9A, 0xBC, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}};
constexpr winrt::guid IID_IDeviceResources{0x23456789, 0x2345, 0x6789, {0xAB, 0xCD, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD}};

// Class IDs for COM registration
constexpr winrt::guid CLSID_CustomService{0xABCDEF01, 0x2345, 0x6789, {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89}};
constexpr winrt::guid CLSID_DeviceResources{0xBCDEF012, 0x3456, 0x789A, {0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A}};

// Forward declarations of implementation classes
struct CustomService;
struct CDeviceResources;

/**
 * @brief Implementation of ICustomService using winrt::implements
 * @details Demonstrates COM implementation with automatic reference counting
 */
struct CustomService : winrt::implements<CustomService, ::ICustomService> {
  private:
    std::wstring m_config;
    std::atomic<bool> m_initialized{false};
    mutable std::mutex m_mutex;

  public:
    // ICustomService implementation
    HRESULT __stdcall Initialize(LPCWSTR config) noexcept override;
    HRESULT __stdcall ProcessData(const BYTE* input, DWORD inputSize, BYTE** output,
                                  DWORD* outputSize) noexcept override;
    HRESULT __stdcall GetStatus(LPWSTR* status) noexcept override;
    HRESULT __stdcall Shutdown() noexcept override;
};

/**
 * @brief Implementation of IDeviceResources using winrt::implements
 * @details COM wrapper for DX::DeviceResources with proper error handling
 */
struct CDeviceResources : winrt::implements<CDeviceResources, ::IDeviceResources> {
  private:
    std::unique_ptr<DX::DeviceResources> m_deviceResources;
    mutable std::mutex m_mutex;

  public:
    // IDeviceResources implementation
    HRESULT __stdcall InitializeDevice(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat,
                                      UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel, UINT flags) noexcept override;
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