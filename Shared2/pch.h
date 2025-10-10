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
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

// Include our custom COM interface declarations
#include "Shared2Ifcs.h"

// Standard C++
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>
#include <atomic>
#include <format>
#include <stdexcept>

namespace winrt::Shared2 {

// Forward declarations of implementation classes
struct CustomService;
struct CustomClassFactory;

/**
 * @brief Implementation of ICustomService using winrt::implements
 * @details Demonstrates COM implementation with automatic reference counting
 */
struct CustomService : winrt::implements<CustomService, ::ICustomService>
{
private:
    std::wstring m_config;
    std::atomic<bool> m_initialized{ false };
    mutable std::mutex m_mutex;

public:
    // ICustomService implementation
    HRESULT __stdcall Initialize(LPCWSTR config) noexcept override;
    HRESULT __stdcall ProcessData(const BYTE* input, DWORD inputSize, BYTE** output, DWORD* outputSize) noexcept override;
    HRESULT __stdcall GetStatus(LPWSTR* status) noexcept override;
    HRESULT __stdcall Shutdown() noexcept override;
};

/**
 * @brief Implementation of ICustomClassFactory using winrt::implements
 * @details Custom class factory that creates ICustomService instances
 */
struct CustomClassFactory : winrt::implements<CustomClassFactory, ::ICustomClassFactory, ::IClassFactory>
{
private:
    std::atomic<ULONG> m_lockCount{ 0 };

public:
    // IClassFactory implementation
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) noexcept override;
    HRESULT __stdcall LockServer(BOOL fLock) noexcept override;
    
    // ICustomClassFactory implementation
    HRESULT __stdcall CreateConfiguredInstance(LPCWSTR config, REFIID riid, void** ppvObject) noexcept override;
    HRESULT __stdcall GetFactoryInfo(LPWSTR* info) noexcept override;
    
    // Helper methods
    ULONG GetLockCount() const noexcept { return m_lockCount.load(); }
};

/**
 * @brief Factory functions for creating COM objects
 */
inline winrt::com_ptr<ICustomService> make_custom_service() {
    return winrt::make<CustomService>();
}

inline winrt::com_ptr<ICustomClassFactory> make_custom_class_factory() {
    return winrt::make<CustomClassFactory>();
}

} // namespace winrt::Shared2