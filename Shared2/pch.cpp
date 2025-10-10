#include "pch.h"
// @file: Shared2 PCH implementation - Self-contained DirectX device resources

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

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

// Helper function for DXGI format conversion
DXGI_FORMAT CDeviceResources::NoSRGB(DXGI_FORMAT fmt) {
    switch (fmt) {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_UNORM;
    default:
        return fmt;
    }
}

void CDeviceResources::InitializeDXGIAdapter(IDXGIFactory4* factory) {
    // Use provided factory if available, otherwise create new one
    if (factory != nullptr) {
        m_dxgiFactory.copy_from(factory);
    } else {
        winrt::check_hresult(CreateDXGIFactory2(0, __uuidof(IDXGIFactory4), m_dxgiFactory.put_void()));
    }

    // Determine whether tearing support is available for fullscreen borderless windows.
    if (m_options & c_AllowTearing) {
        BOOL allowTearing = FALSE;
        winrt::com_ptr<IDXGIFactory5> factory5;

        if (m_dxgiFactory.try_as(factory5)) {
            HRESULT hr =
                factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            if (FAILED(hr) || !allowTearing) {
                m_options &= ~c_AllowTearing;
            }
        } else {
            m_options &= ~c_AllowTearing;
        }
    }

    InitializeAdapter(m_adapter.put());
}

void CDeviceResources::InitializeAdapter(IDXGIAdapter1** ppAdapter, DXGI_GPU_PREFERENCE preference) {
    *ppAdapter = nullptr;

    winrt::com_ptr<IDXGIAdapter1> adapter;
    winrt::com_ptr<IDXGIFactory6> factory6;

    if (m_dxgiFactory.try_as(factory6)) {
        for (UINT adapterIndex = 0;
             DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, preference,
                                                                          __uuidof(IDXGIAdapter1), adapter.put_void());
             ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            winrt::check_hresult(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
                break;
            }
            adapter = nullptr;
        }
    }

    if (!adapter) {
        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, adapter.put());
             ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            winrt::check_hresult(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
                break;
            }
            adapter = nullptr;
        }
    }

    *ppAdapter = adapter.detach();
}

void CDeviceResources::MoveToNextFrame() {
    const UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
    winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), currentFenceValue));

    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex]) {
        winrt::check_hresult(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.get()));
        WaitForSingleObjectEx(m_fenceEvent.get(), INFINITE, FALSE);
    }

    m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;
}

// CDeviceResources COM implementation
HRESULT __stdcall CDeviceResources::InitializeDevice(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat,
                                                     UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel,
                                                     UINT flags) noexcept {
    try {
        if (backBufferCount > MAX_BACK_BUFFER_COUNT)
            return E_INVALIDARG;
        if (minFeatureLevel < D3D_FEATURE_LEVEL_11_0)
            return E_INVALIDARG;

        m_backBufferFormat = backBufferFormat;
        m_depthBufferFormat = depthBufferFormat;
        m_backBufferCount = backBufferCount;
        m_d3dMinFeatureLevel = minFeatureLevel;
        m_options = flags;

        if (m_options & 0x4) { // c_RequireTearingSupport equivalent
            m_options |= c_AllowTearing;
        }

        InitializeDXGIAdapter();
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::CreateDeviceResources(IDXGIAdapter1* adapter) noexcept {
    try {
        // Use provided adapter if available, otherwise use member adapter
        IDXGIAdapter1* adapterToUse = adapter ? adapter : m_adapter.get();
        if (!adapterToUse)
            return E_NOT_VALID_STATE;

        // Create the Direct3D 12 API device object
        winrt::check_hresult(
            D3D12CreateDevice(adapterToUse, m_d3dMinFeatureLevel, __uuidof(ID3D12Device), m_d3dDevice.put_void()));

        m_d3dDevice->SetName(L"DeviceResources");

#ifndef NDEBUG
        winrt::com_ptr<ID3D12InfoQueue> d3dInfoQueue;
        if (m_d3dDevice.try_as(d3dInfoQueue)) {
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        }
#endif

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        winrt::check_hresult(
            m_d3dDevice->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), m_commandQueue.put_void()));
        m_commandQueue->SetName(L"DeviceResources");

        // Create descriptor heaps for render target views and depth stencil views.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = m_backBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap),
                                                               m_rtvDescriptorHeap.put_void()));
        winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(ID3D12DescriptorHeap),
                                                               m_dsvDescriptorHeap.put_void()));

        m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // Create command allocators for each back buffer.
        for (UINT n = 0; n < m_backBufferCount; n++) {
            winrt::check_hresult(m_d3dDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), m_commandAllocators[n].put_void()));

            wchar_t name[25] = {};
            swprintf_s(name, L"Render target %u", n);
            m_commandAllocators[n]->SetName(name);
        }

        // Create a command list for recording graphics commands.
        winrt::check_hresult(
            m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].get(), nullptr,
                                           __uuidof(ID3D12GraphicsCommandList), m_commandList.put_void()));
        winrt::check_hresult(m_commandList->Close());
        m_commandList->SetName(L"DeviceResources");

        // Create a fence for tracking GPU execution progress.
        winrt::check_hresult(m_d3dDevice->CreateFence(m_fenceValues[m_backBufferIndex], D3D12_FENCE_FLAG_NONE,
                                                      __uuidof(ID3D12Fence), m_fence.put_void()));
        m_fenceValues[m_backBufferIndex]++;

        m_fence->SetName(L"DeviceResources");

        m_fenceEvent.attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!m_fenceEvent) {
            winrt::throw_last_error();
        }

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

        if (!m_d3dDevice)
            return E_NOT_VALID_STATE;

        // Wait until all previous GPU work is complete.
        for (UINT n = 0; n < m_backBufferCount; n++) {
            m_renderTargets[n] = nullptr;
            m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
        }

        // Determine the render target size in pixels.
        m_outputSize.left = m_outputSize.top = 0;
        m_outputSize.right = static_cast<LONG>(width);
        m_outputSize.bottom = static_cast<LONG>(height);

        DXGI_FORMAT backBufferFormat = NoSRGB(m_backBufferFormat);

        if (m_swapChain) {
            HRESULT hr =
                m_swapChain->ResizeBuffers(m_backBufferCount, width, height, backBufferFormat,
                                           (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
                HandleDeviceLost();
                return S_OK;
            } else {
                winrt::check_hresult(hr);
            }
        } else {
            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = m_backBufferCount;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

            winrt::com_ptr<IDXGISwapChain1> swapChain;
            winrt::check_hresult(m_dxgiFactory->CreateSwapChainForComposition(m_commandQueue.get(), &swapChainDesc,
                                                                              nullptr, swapChain.put()));

            winrt::check_hresult(swapChain.try_as(m_swapChain));
        }

        // Create render target views of the swap chain back buffers.
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT n = 0; n < m_backBufferCount; n++) {
            winrt::check_hresult(m_swapChain->GetBuffer(n, __uuidof(ID3D12Resource), m_renderTargets[n].put_void()));

            m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].get(), nullptr, rtvDescriptor);

            wchar_t name[25] = {};
            swprintf_s(name, L"Render target %u", n);
            m_renderTargets[n]->SetName(name);

            rtvDescriptor.ptr += m_rtvDescriptorSize;
        }

        m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
            D3D12_HEAP_PROPERTIES depthHeapProperties = {};
            depthHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
            depthHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            depthHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            depthHeapProperties.CreationNodeMask = 1;
            depthHeapProperties.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC depthStencilDesc = {};
            depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            depthStencilDesc.Alignment = 0;
            depthStencilDesc.Width = width;
            depthStencilDesc.Height = height;
            depthStencilDesc.DepthOrArraySize = 1;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.Format = m_depthBufferFormat;
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
            depthOptimizedClearValue.Format = m_depthBufferFormat;
            depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
            depthOptimizedClearValue.DepthStencil.Stencil = 0;

            winrt::check_hresult(m_d3dDevice->CreateCommittedResource(
                &depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &depthOptimizedClearValue, __uuidof(ID3D12Resource), m_depthStencil.put_void()));

            m_depthStencil->SetName(L"Depth stencil");

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = m_depthBufferFormat;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            m_d3dDevice->CreateDepthStencilView(m_depthStencil.get(), &dsvDesc,
                                                m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

        // Set the 3D rendering viewport and scissor rectangle to target the entire window.
        m_screenViewport.TopLeftX = m_screenViewport.TopLeftY = 0.f;
        m_screenViewport.Width = static_cast<float>(width);
        m_screenViewport.Height = static_cast<float>(height);
        m_screenViewport.MinDepth = D3D12_MIN_DEPTH;
        m_screenViewport.MaxDepth = D3D12_MAX_DEPTH;

        m_scissorRect.left = m_scissorRect.top = 0;
        m_scissorRect.right = static_cast<LONG>(width);
        m_scissorRect.bottom = static_cast<LONG>(height);

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

        for (UINT n = 0; n < m_backBufferCount; n++) {
            m_commandAllocators[n] = nullptr;
            m_renderTargets[n] = nullptr;
        }

        m_depthStencil = nullptr;
        m_commandList = nullptr;
        m_commandQueue = nullptr;
        m_fence = nullptr;
        m_rtvDescriptorHeap = nullptr;
        m_dsvDescriptorHeap = nullptr;
        m_swapChain = nullptr;
        m_d3dDevice = nullptr;
        m_dxgiFactory = nullptr;
        m_adapter = nullptr;

        m_fenceEvent.close();

        CreateDeviceResources();
        CreateWindowSizeDependentResources(m_outputSize.right - m_outputSize.left,
                                           m_outputSize.bottom - m_outputSize.top);

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

        if (m_swapChain == nullptr)
            return E_NOT_VALID_STATE;

        *pWidth = static_cast<UINT>(m_outputSize.right - m_outputSize.left);
        *pHeight = static_cast<UINT>(m_outputSize.bottom - m_outputSize.top);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::IsTearingSupported(BOOL* pSupported) noexcept {
    try {

        if (!pSupported)
            return E_INVALIDARG;

        *pSupported = (m_options & c_AllowTearing) ? TRUE : FALSE;
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetD3DDevice(ID3D12Device** ppDevice) noexcept {
    try {

        if (!ppDevice)
            return E_INVALIDARG;

        if (!m_d3dDevice)
            return E_NOT_VALID_STATE;

        m_d3dDevice.copy_to(ppDevice);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDXGIFactory(IDXGIFactory4** ppFactory) noexcept {
    try {

        if (!ppFactory)
            return E_INVALIDARG;

        if (!m_dxgiFactory)
            return E_NOT_VALID_STATE;

        m_dxgiFactory.copy_to(ppFactory);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetSwapChain(IDXGISwapChain3** ppSwapChain) noexcept {
    try {

        if (!ppSwapChain)
            return E_INVALIDARG;

        if (!m_swapChain)
            return E_NOT_VALID_STATE;

        m_swapChain.copy_to(ppSwapChain);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetCommandQueue(ID3D12CommandQueue** ppCommandQueue) noexcept {
    try {

        if (!ppCommandQueue)
            return E_INVALIDARG;

        if (!m_commandQueue)
            return E_NOT_VALID_STATE;

        m_commandQueue.copy_to(ppCommandQueue);
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDeviceFeatureLevel(D3D_FEATURE_LEVEL* pFeatureLevel) noexcept {
    try {

        if (!pFeatureLevel)
            return E_INVALIDARG;

        *pFeatureLevel = m_d3dFeatureLevel;
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetBackBufferFormat(DXGI_FORMAT* pFormat) noexcept {
    try {

        if (!pFormat)
            return E_INVALIDARG;

        *pFormat = m_backBufferFormat;
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::GetDepthBufferFormat(DXGI_FORMAT* pFormat) noexcept {
    try {

        if (!pFormat)
            return E_INVALIDARG;

        *pFormat = m_depthBufferFormat;
        return S_OK;
    } catch (const winrt::hresult_error& ex) {
        return ex.code();
    } catch (const std::exception&) {
        return E_FAIL;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT __stdcall CDeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState) noexcept {
    try {

        if (!m_commandList)
            return E_NOT_VALID_STATE;

        winrt::check_hresult(m_commandAllocators[m_backBufferIndex]->Reset());
        winrt::check_hresult(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].get(), nullptr));

        if (beforeState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = m_renderTargets[m_backBufferIndex].get();
            barrier.Transition.StateBefore = beforeState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            m_commandList->ResourceBarrier(1, &barrier);
        }

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

        if (!m_swapChain)
            return E_NOT_VALID_STATE;

        if (beforeState != D3D12_RESOURCE_STATE_PRESENT) {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = m_renderTargets[m_backBufferIndex].get();
            barrier.Transition.StateBefore = beforeState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            m_commandList->ResourceBarrier(1, &barrier);
        }

        winrt::check_hresult(m_commandList->Close());

        std::initializer_list<ID3D12CommandList*> commands{m_commandList.get()};
        m_commandQueue->ExecuteCommandLists(commands.size(), commands.begin());

        HRESULT hr;
        if (m_options & c_AllowTearing) {
            hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        } else {
            hr = m_swapChain->Present(1, 0);
        }

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            HandleDeviceLost();
        } else {
            winrt::check_hresult(hr);
            MoveToNextFrame();
        }

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

        if (!m_commandList || !m_commandQueue)
            return E_NOT_VALID_STATE;

        winrt::check_hresult(m_commandList->Close());

        std::initializer_list<ID3D12CommandList*> commands{m_commandList.get()};
        m_commandQueue->ExecuteCommandLists(commands.size(), commands.begin());

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

        if (!m_commandQueue || !m_fence)
            return E_NOT_VALID_STATE;

        winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), m_fenceValues[m_backBufferIndex]));

        winrt::check_hresult(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.get()));
        WaitForSingleObjectEx(m_fenceEvent.get(), INFINITE, FALSE);

        m_fenceValues[m_backBufferIndex]++;

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
