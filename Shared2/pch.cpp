#include "pch.h"
#include "CDeviceResources.h"
// @file: Shared2 PCH implementation - CustomClassFactory and exports

namespace winrt::Shared2 {

HRESULT __stdcall CustomClassFactory::CreateInstance(IUnknown* unknown, REFIID riid, void** ppv) noexcept {
    try {
        if (ppv == nullptr)
            return E_INVALIDARG;
        if (unknown) // No aggregation support
            return CLASS_E_NOAGGREGATION;

        // note: use short-circuiting to make ease of debugging with multiple branches
        if (IsEqualIID(riid, __uuidof(IDeviceResources)))
            return CreateDeviceResources(riid, ppv);

        // ... put more classes below ...
        // ...

        return E_NOINTERFACE;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

ULONG CustomClassFactory::GetLockCount() const noexcept {
    return m_lockCount.load();
}

HRESULT __stdcall CustomClassFactory::LockServer(BOOL fLock) noexcept {
    if (fLock) {
        m_lockCount.fetch_add(1);
    } else {
        m_lockCount.fetch_sub(1);
    }
    return S_OK;
}

} // namespace winrt::Shared2

extern "C" {

HRESULT __stdcall CreateDeviceResources(REFIID riid, void** ppv) noexcept {
    using namespace winrt::Shared2;
    try {
        if (!ppv)
            return E_INVALIDARG;
        auto res = winrt::make<CDeviceResources>();
        return res->QueryInterface(riid, ppv);
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CreateCustomClassFactory(IClassFactory** output) noexcept {
    using namespace winrt::Shared2;
    try {
        if (!output)
            return E_INVALIDARG;
        auto factory = winrt::make<CustomClassFactory>();
        factory.copy_to(output);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    }
}

} // extern "C"

/// @see combaseapi.h
__control_entrypoint(DllExport) STDAPI DllCanUnloadNow(void) {
    return S_OK;
}