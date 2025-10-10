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
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::ProcessData(
    const BYTE* input, 
    DWORD inputSize, 
    BYTE** output, 
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
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::GetStatus(LPWSTR* status) noexcept {
    try {
        if (status == nullptr) {
            return E_INVALIDARG;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::wstring statusText = std::format(L"CustomService: {} (Config: {})", 
            m_initialized.load() ? L"Initialized" : L"Not Initialized",
            m_config.empty() ? L"None" : m_config);
        
        size_t length = (statusText.length() + 1) * sizeof(wchar_t);
        LPWSTR result = static_cast<LPWSTR>(CoTaskMemAlloc(length));
        if (result == nullptr) {
            return E_OUTOFMEMORY;
        }
        
        wcscpy_s(result, statusText.length() + 1, statusText.c_str());
        *status = result;
        
        return S_OK;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomService::Shutdown() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_config.clear();
        m_initialized.store(false);
        
        return S_OK;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

// CustomClassFactory implementation
HRESULT __stdcall CustomClassFactory::CreateInstance(
    IUnknown* pUnkOuter, 
    REFIID riid, 
    void** ppvObject) noexcept {
    
    try {
        if (ppvObject == nullptr) {
            return E_INVALIDARG;
        }
        
        *ppvObject = nullptr;
        
        // No aggregation support
        if (pUnkOuter != nullptr) {
            return CLASS_E_NOAGGREGATION;
        }
        
        // Create and return the service
        if (riid == __uuidof(ICustomService) || riid == IID_IUnknown) {
            auto service = make_custom_service();
            return service->QueryInterface(riid, ppvObject);
        }
        
        return E_NOINTERFACE;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomClassFactory::LockServer(BOOL fLock) noexcept {
    try {
        if (fLock) {
            m_lockCount.fetch_add(1);
        } else {
            m_lockCount.fetch_sub(1);
        }
        return S_OK;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomClassFactory::CreateConfiguredInstance(
    LPCWSTR config, 
    REFIID riid, 
    void** ppvObject) noexcept {
    
    try {
        if (ppvObject == nullptr) {
            return E_INVALIDARG;
        }
        
        *ppvObject = nullptr;
        
        if (riid == __uuidof(ICustomService) || riid == IID_IUnknown) {
            auto service = make_custom_service();
            
            // Initialize with configuration if provided
            if (config != nullptr) {
                HRESULT hr = service->Initialize(config);
                if (FAILED(hr)) {
                    return hr;
                }
            }
            
            return service->QueryInterface(riid, ppvObject);
        }
        
        return E_NOINTERFACE;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CustomClassFactory::GetFactoryInfo(LPWSTR* info) noexcept {
    try {
        if (info == nullptr) {
            return E_INVALIDARG;
        }
        
        std::wstring factoryInfo = std::format(L"CustomClassFactory: LockCount={}", m_lockCount.load());
        
        size_t length = (factoryInfo.length() + 1) * sizeof(wchar_t);
        LPWSTR result = static_cast<LPWSTR>(CoTaskMemAlloc(length));
        if (result == nullptr) {
            return E_OUTOFMEMORY;
        }
        
        wcscpy_s(result, factoryInfo.length() + 1, factoryInfo.c_str());
        *info = result;
        
        return S_OK;
    }
    catch (...) {
        return E_UNEXPECTED;
    }
}

} // namespace winrt::Shared2