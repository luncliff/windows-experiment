#include "pch.h"
// @file: Shared2 PCH implementation - COM interface method implementations

namespace winrt::Shared2 {

// CustomService implementation
HRESULT __stdcall CustomService::Initialize(LPCWSTR config) noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_initialized.load()) {
            return E_FAIL; // Already initialized
        }

        if (config == nullptr) {
            return E_INVALIDARG;
        }

        m_config = config;
        m_initialized.store(true);

        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::ProcessData(const BYTE* input, DWORD inputSize, BYTE** output,
                                             DWORD* outputSize) noexcept {

    try {
        if (!m_initialized.load()) {
            return E_FAIL; // Not initialized
        }

        if (input == nullptr || output == nullptr || outputSize == nullptr) {
            return E_INVALIDARG;
        }

        if (inputSize == 0) {
            *output = nullptr;
            *outputSize = 0;
            return S_OK;
        }

        // Simple processing: copy input to output with size prefix
        DWORD totalSize = sizeof(DWORD) + inputSize;
        BYTE* result = static_cast<BYTE*>(CoTaskMemAlloc(totalSize));
        if (result == nullptr) {
            return E_OUTOFMEMORY;
        }

        // Write size prefix
        *reinterpret_cast<DWORD*>(result) = inputSize;
        // Copy input data
        memcpy(result + sizeof(DWORD), input, inputSize);

        *output = result;
        *outputSize = totalSize;

        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::GetStatus(LPWSTR* status) noexcept {
    try {
        if (status == nullptr) {
            return E_INVALIDARG;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        std::wstring statusText =
            std::format(L"CustomService: {} (Config: {})", m_initialized.load() ? L"Initialized" : L"Not Initialized",
                        m_config.empty() ? L"None" : m_config);

        size_t length = (statusText.length() + 1) * sizeof(wchar_t);
        LPWSTR result = static_cast<LPWSTR>(CoTaskMemAlloc(length));
        if (result == nullptr) {
            return E_OUTOFMEMORY;
        }

        wcscpy_s(result, statusText.length() + 1, statusText.c_str());
        *status = result;

        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::Shutdown() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_config.clear();
        m_initialized.store(false);

        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomClassFactory::CreateInstance(IUnknown* unknown, REFIID riid, void** ppv) noexcept {
    try {
        if (ppv == nullptr)
            return E_INVALIDARG;
        if (unknown) // No aggregation support
            return CLASS_E_NOAGGREGATION;

        if (IsEqualIID(riid, __uuidof(ICustomService))) {
            auto service = winrt::make<CustomService>();
            return service->QueryInterface(riid, ppv);
        }
        return E_NOINTERFACE;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

ULONG CustomClassFactory::GetLockCount() const noexcept {
    return m_lockCount.load();
}

HRESULT __stdcall CustomClassFactory::LockServer(BOOL fLock) noexcept {
    try {
        if (fLock) {
            m_lockCount.fetch_add(1);
        } else {
            m_lockCount.fetch_sub(1);
        }
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

} // namespace winrt::Shared2

// Standard DLL export implementations
extern "C" {

HRESULT __stdcall CreateCustomClassFactory(IClassFactory** output) noexcept {
    using namespace winrt::Shared2;
    if (output == nullptr)
        return E_INVALIDARG;
    try {
        auto factory = winrt::make<CustomClassFactory>();
        factory.copy_to(output);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    }
}

SHARED2_API HRESULT __stdcall CreateCustomService(REFIID riid, void** ppv) {
    using namespace winrt::Shared2;
    try {
        if (ppv == nullptr)
            return E_INVALIDARG;
        *ppv = nullptr;

        auto service = winrt::make<CustomService>();
        return service->QueryInterface(riid, ppv);
    } catch (...) {
        return E_UNEXPECTED;
    }
}

} // extern "C"

/// @see combaseapi.h
__control_entrypoint(DllExport) STDAPI DllCanUnloadNow(void) {
    // todo: count the active number of IClassFactory, then return S_OK if the count is zero
    return S_OK;
}
