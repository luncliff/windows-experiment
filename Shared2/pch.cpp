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
        else if (IsEqualIID(riid, __uuidof(IDeviceResources))) {
            auto deviceResources = winrt::make<CDeviceResources>();
            return deviceResources->QueryInterface(riid, ppv);
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

// CDeviceResources implementation
HRESULT __stdcall CDeviceResources::InitializeDevice(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat,
                                                    UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel, UINT flags) noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Create the wrapped DeviceResources instance
        m_deviceResources = std::make_unique<DX::DeviceResources>(
            backBufferFormat, depthBufferFormat, backBufferCount, minFeatureLevel, flags);
        
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::CreateDeviceResources() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->CreateDeviceResources();
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::CreateWindowSizeDependentResources(UINT width, UINT height) noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->CreateWindowSizeDependentResources(width, height);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::HandleDeviceLost() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->HandleDeviceLost();
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetOutputSize(UINT* pWidth, UINT* pHeight) noexcept {
    try {
        if (!pWidth || !pHeight)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        RECT outputSize = m_deviceResources->GetOutputSize();
        *pWidth = static_cast<UINT>(outputSize.right - outputSize.left);
        *pHeight = static_cast<UINT>(outputSize.bottom - outputSize.top);
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::IsTearingSupported(BOOL* pSupported) noexcept {
    try {
        if (!pSupported)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        *pSupported = m_deviceResources->IsTearingSupported() ? TRUE : FALSE;
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetD3DDevice(ID3D12Device** ppDevice) noexcept {
    try {
        if (!ppDevice)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        auto device = m_deviceResources->GetD3DDevice();
        if (!device)
            return E_NOT_VALID_STATE;
            
        device->AddRef();
        *ppDevice = device;
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDXGIFactory(IDXGIFactory4** ppFactory) noexcept {
    try {
        if (!ppFactory)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        auto factory = m_deviceResources->GetDXGIFactory();
        if (!factory)
            return E_NOT_VALID_STATE;
            
        factory->AddRef();
        *ppFactory = factory;
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetSwapChain(IDXGISwapChain3** ppSwapChain) noexcept {
    try {
        if (!ppSwapChain)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        auto swapChain = m_deviceResources->GetSwapChain();
        if (!swapChain)
            return E_NOT_VALID_STATE;
            
        swapChain->AddRef();
        *ppSwapChain = swapChain;
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetCommandQueue(ID3D12CommandQueue** ppCommandQueue) noexcept {
    try {
        if (!ppCommandQueue)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        auto commandQueue = m_deviceResources->GetCommandQueue();
        if (!commandQueue)
            return E_NOT_VALID_STATE;
            
        commandQueue->AddRef();
        *ppCommandQueue = commandQueue;
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDeviceFeatureLevel(D3D_FEATURE_LEVEL* pFeatureLevel) noexcept {
    try {
        if (!pFeatureLevel)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        *pFeatureLevel = m_deviceResources->GetDeviceFeatureLevel();
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetBackBufferFormat(DXGI_FORMAT* pFormat) noexcept {
    try {
        if (!pFormat)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        *pFormat = m_deviceResources->GetBackBufferFormat();
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDepthBufferFormat(DXGI_FORMAT* pFormat) noexcept {
    try {
        if (!pFormat)
            return E_INVALIDARG;
            
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        *pFormat = m_deviceResources->GetDepthBufferFormat();
        return S_OK;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState) noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->Prepare(beforeState);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::Present(D3D12_RESOURCE_STATES beforeState) noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->Present(beforeState);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::ExecuteCommandList() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->ExecuteCommandList();
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::WaitForGpu() noexcept {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_deviceResources)
            return E_NOT_VALID_STATE;
            
        m_deviceResources->WaitForGpu();
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
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

SHARED2_API HRESULT __stdcall CreateDeviceResources(REFIID riid, void** ppv) noexcept {
    using namespace winrt::Shared2;
    try {
        if (ppv == nullptr)
            return E_INVALIDARG;
        *ppv = nullptr;

        auto deviceResources = winrt::make<CDeviceResources>();
        return deviceResources->QueryInterface(riid, ppv);
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
