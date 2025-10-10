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
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace winrt::Shared2 {

// Forward declarations of implementation classes
struct CDeviceResources;

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