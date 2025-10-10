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

// Class IDs for COM registration
constexpr winrt::guid CLSID_CustomService{0xABCDEF01, 0x2345, 0x6789, {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89}};

// Forward declarations of implementation classes
struct CustomService;

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