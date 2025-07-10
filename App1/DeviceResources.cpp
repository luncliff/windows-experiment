/**
 * @file DeviceResources.cpp
 * @brief A wrapper for the Direct3D 12 device and swapchain
 * @see https://github.com/Microsoft/DirectML/blob/master/Samples/DirectMLSuperResolution/DeviceResources.h
 * @see https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingHelloWorld/DeviceResources.h
 * @see https://github.com/Microsoft/DirectXTK12/wiki/DeviceResources

 * @copyright Copyright (c) Microsoft Corporation. All rights reserved. Licensed under the MIT License.
 * 
 * @note Changes
 *  - Run clang-format with the repository style
 *  - Use winrt::com_ptr instead of Microsoft::WRL::ComPtr
 *  - Use winrt::check_hresult instead of ThrowIfFailed
 *  - Exception specifications for functions
 *  - Use winrt::hresult_error based exceptions for error handling
 */
#include "pch.h"

#include "DeviceResources.h"

#include <dxgidebug.h>
#include <format>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace DX {

RECT DeviceResources::GetOutputSize() const noexcept {
    return m_outputSize;
}

bool DeviceResources::IsWindowVisible() const noexcept {
    return m_isWindowVisible;
}

bool DeviceResources::IsTearingSupported() const noexcept {
    return m_options & c_AllowTearing;
}

IDXGIAdapter1* DeviceResources::GetAdapter() const noexcept {
    return m_adapter.get();
}

ID3D12Device* DeviceResources::GetD3DDevice() const noexcept {
    return m_d3dDevice.get();
}

IDXGIFactory4* DeviceResources::GetDXGIFactory() const noexcept {
    return m_dxgiFactory.get();
}

IDXGISwapChain3* DeviceResources::GetSwapChain() const noexcept {
    return m_swapChain.get();
}

D3D_FEATURE_LEVEL DeviceResources::GetDeviceFeatureLevel() const noexcept {
    return m_d3dFeatureLevel;
}

ID3D12Resource* DeviceResources::GetRenderTarget() const noexcept {
    return m_renderTargets[m_backBufferIndex].get();
}

ID3D12Resource* DeviceResources::GetDepthStencil() const noexcept {
    return m_depthStencil.get();
}

ID3D12CommandQueue* DeviceResources::GetCommandQueue() const noexcept {
    return m_commandQueue.get();
}

ID3D12CommandAllocator* DeviceResources::GetCommandAllocator() const noexcept {
    return m_commandAllocators[m_backBufferIndex].get();
}

ID3D12GraphicsCommandList* DeviceResources::GetCommandList() const noexcept {
    return m_commandList.get();
}

DXGI_FORMAT DeviceResources::GetBackBufferFormat() const noexcept {
    return m_backBufferFormat;
}

DXGI_FORMAT DeviceResources::GetDepthBufferFormat() const noexcept {
    return m_depthBufferFormat;
}

D3D12_VIEWPORT DeviceResources::GetScreenViewport() const noexcept {
    return m_screenViewport;
}

D3D12_RECT DeviceResources::GetScissorRect() const noexcept {
    return m_scissorRect;
}

UINT DeviceResources::GetCurrentFrameIndex() const noexcept {
    return m_backBufferIndex;
}

UINT DeviceResources::GetPreviousFrameIndex() const noexcept {
    return m_backBufferIndex == 0 ? m_backBufferCount - 1 : m_backBufferIndex - 1;
}

UINT DeviceResources::GetBackBufferCount() const noexcept {
    return m_backBufferCount;
}

UINT DeviceResources::GetDeviceOptions() const noexcept {
    return m_options;
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceResources::GetRenderTargetView() const noexcept {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex,
                                         m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DeviceResources::GetDepthStencilView() const noexcept {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void DeviceResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept {
    m_deviceNotify = deviceNotify;

    // On RS4 and higher, applications that handle device removal
    // should declare themselves as being able to do so
    __if_exists(DXGIDeclareAdapterRemovalSupport) {
        if (deviceNotify) {
            if (FAILED(DXGIDeclareAdapterRemovalSupport())) {
                OutputDebugStringW(L"Warning: application failed to declare adapter removal support.\n");
            }
        }
    }
}

DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) {
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

// Constructor for DeviceResources.
DeviceResources::DeviceResources(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
                                 D3D_FEATURE_LEVEL minFeatureLevel, UINT flags) noexcept(false)
    : m_backBufferFormat(backBufferFormat), m_depthBufferFormat(depthBufferFormat), m_backBufferCount(backBufferCount),
      m_d3dMinFeatureLevel(minFeatureLevel), m_options(flags) {
    if (backBufferCount > MAX_BACK_BUFFER_COUNT)
        throw winrt::hresult_out_of_bounds{};
    if (minFeatureLevel < D3D_FEATURE_LEVEL_11_0)
        throw winrt::hresult_out_of_bounds{};

    if (m_options & c_RequireTearingSupport) {
        m_options |= c_AllowTearing;
    }

    InitializeDXGIAdapter();
}

// Destructor for DeviceResources.
DeviceResources::~DeviceResources() {
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    WaitForGpu();
}

// Configures DXGI Factory and retrieve an adapter.
void DeviceResources::InitializeDXGIAdapter() {
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        winrt::com_ptr<ID3D12Debug> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), debug.put_void()))) {
            debug->EnableDebugLayer();
        }

        winrt::com_ptr<ID3D12Debug1> debug1;
        if (debug.try_as(debug1)) {
            debug1->SetEnableGPUBasedValidation(true);
            debug1->SetEnableSynchronizedCommandQueueValidation(true);
        }

        // Use classic COM for DXGI debug interfaces since C++/WinRT doesn't directly support them
        winrt::com_ptr<IDXGIInfoQueue> dxgiInfoQueue = nullptr;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), dxgiInfoQueue.put_void()))) {

            winrt::check_hresult(
                CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory4), m_dxgiFactory.put_void()));

            // Using error and corruption severity constants directly
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        }
    }
#endif
    if (m_dxgiFactory == nullptr) {
        winrt::check_hresult(CreateDXGIFactory1(__uuidof(IDXGIFactory4), m_dxgiFactory.put_void()));
    }

    // Determines whether tearing support is available for fullscreen borderless windows.
    if (m_options & (c_AllowTearing | c_RequireTearingSupport)) {
        BOOL allowTearing = FALSE;

        winrt::com_ptr<IDXGIFactory5> factory5;
        HRESULT hr = m_dxgiFactory.try_as(factory5);
        if (SUCCEEDED(hr)) {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing) {
            OutputDebugStringW(L"WARNING: Variable refresh rate displays are not supported.\n");
            if (m_options & c_RequireTearingSupport) {
                throw winrt::hresult_error(E_FAIL, L"Error: Sample must be run on an OS with tearing support.\n");
            }
            m_options &= ~c_AllowTearing;
        }
    }

    // Initialize the adapter
    InitializeAdapter(m_adapter.put());
}

void DeviceResources::CreateDeviceResources() noexcept(false) {
    // Create the DX12 API device object.
    winrt::check_hresult(
        D3D12CreateDevice(m_adapter.get(), m_d3dMinFeatureLevel, __uuidof(ID3D12Device), m_d3dDevice.put_void()));

#ifndef NDEBUG
    // Configure debug device (if active).
    winrt::com_ptr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(m_d3dDevice.try_as(d3dInfoQueue))) {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] = {D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE};
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    // Determine maximum supported feature level for this device
    static const D3D_FEATURE_LEVEL s_featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = {_countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0};

    HRESULT hr = m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
    if (SUCCEEDED(hr)) {
        m_d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
    } else {
        m_d3dFeatureLevel = m_d3dMinFeatureLevel;
    }

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    winrt::check_hresult(
        m_d3dDevice->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), m_commandQueue.put_void()));

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_backBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap),
                                                           m_rtvDescriptorHeap.put_void()));

    m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap),
                                                               m_dsvDescriptorHeap.put_void()));
    }

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        winrt::check_hresult(m_d3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), m_commandAllocators[n].put_void()));
    }

    // Create a command list for recording graphics commands.
    winrt::check_hresult(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].get(),
                                                        nullptr, __uuidof(ID3D12GraphicsCommandList),
                                                        m_commandList.put_void()));
    winrt::check_hresult(m_commandList->Close());

    // Create a fence for tracking GPU execution progress.
    winrt::check_hresult(m_d3dDevice->CreateFence(m_fenceValues[m_backBufferIndex], D3D12_FENCE_FLAG_NONE,
                                                  __uuidof(ID3D12Fence), m_fence.put_void()));
    m_fenceValues[m_backBufferIndex]++;

    m_fenceEvent.attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (m_fenceEvent.get() == nullptr)
        winrt::throw_last_error();
}

void DeviceResources::CreateWindowSizeDependentResources(UINT width, UINT height) noexcept(false) {
    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        m_renderTargets[n] = nullptr;
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    m_outputSize.left = m_outputSize.top = 0;
    m_outputSize.right = width;
    m_outputSize.bottom = height;

    // Determine the render target size in pixels.
    UINT backBufferWidth = max(m_outputSize.right - m_outputSize.left, 1);
    UINT backBufferHeight = max(m_outputSize.bottom - m_outputSize.top, 1);
    DXGI_FORMAT backBufferFormat = NoSRGB(m_backBufferFormat);

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain) {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(m_backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat,
                                                (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
#ifdef _DEBUG
            wchar_t buff[64] = {};
            swprintf_s(buff, L"Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                       (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr);
            OutputDebugStringW(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        } else {
            winrt::check_hresult(hr);
        }
    } else {
        DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = backBufferWidth,
            .Height = backBufferHeight,
            .Format = backBufferFormat,
            .Stereo = false,
            .SampleDesc = {1, 0}, // count, quality
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = m_backBufferCount,
        };
        // All Windows Store apps must use this SwapEffect.
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        winrt::com_ptr<IDXGISwapChain1> swapChain;
        if (auto hr =
                m_dxgiFactory->CreateSwapChainForComposition(m_commandQueue.get(), &desc, nullptr, swapChain.put());
            FAILED(hr))
            winrt::throw_hresult(hr);
        winrt::check_hresult(swapChain.try_as(m_swapChain));
#if 0
        // With tearing support enabled we will handle ALT+Enter key presses in the
        // window message loop rather than let DXGI handle it by calling SetFullscreenState.
        if (IsTearingSupported()) {
            m_dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER);
        }
#endif
    }

    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        winrt::check_hresult(m_swapChain->GetBuffer(n, __uuidof(ID3D12Resource), m_renderTargets[n].put_void()));

        std::wstring name = std::format(L"Render Target {}", n);
        m_renderTargets[n]->SetName(name.c_str());

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = m_backBufferFormat;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), n,
                                                    m_rtvDescriptorSize);
        m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].get(), &rtvDesc, rtvDescriptor);
    }

    // Reset the index to the current back buffer.
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC depthStencilDesc =
            CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight,
                                         1, // This depth stencil view has only one texture.
                                         1  // Use a single mipmap level.
            );
        depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

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
    m_screenViewport.Width = static_cast<float>(backBufferWidth);
    m_screenViewport.Height = static_cast<float>(backBufferHeight);
    m_screenViewport.MinDepth = D3D12_MIN_DEPTH;
    m_screenViewport.MaxDepth = D3D12_MAX_DEPTH;

    m_scissorRect.left = m_scissorRect.top = 0;
    m_scissorRect.right = backBufferWidth;
    m_scissorRect.bottom = backBufferHeight;
}

void DeviceResources::HandleDeviceLost() {
    if (m_deviceNotify) {
        m_deviceNotify->OnDeviceLost();
    }

    for (UINT n = 0; n < m_backBufferCount; n++) {
        m_commandAllocators[n] = nullptr;
        m_renderTargets[n] = nullptr;
    }

    m_depthStencil = nullptr;
    m_commandQueue = nullptr;
    m_commandList = nullptr;
    m_fence = nullptr;
    m_rtvDescriptorHeap = nullptr;
    m_dsvDescriptorHeap = nullptr;
    m_swapChain = nullptr;
    m_d3dDevice = nullptr;
    m_dxgiFactory = nullptr;
    m_adapter = nullptr;

#ifdef _DEBUG
    {
        // Use classic COM for DXGI debug interfaces
        winrt::com_ptr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), dxgiDebug.put_void()))) {
            DXGI_DEBUG_RLO_FLAGS flags =
                (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, flags);
        }
    }
#endif
    InitializeDXGIAdapter();
    CreateDeviceResources();
    CreateWindowSizeDependentResources(m_outputSize.right, m_outputSize.bottom);

    if (m_deviceNotify) {
        m_deviceNotify->OnDeviceRestored();
    }
}

void DeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState) noexcept {
    // Reset command list and allocator.
    winrt::check_hresult(m_commandAllocators[m_backBufferIndex]->Reset());
    winrt::check_hresult(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].get(), nullptr));

    if (beforeState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        // Transition the render target into the correct state to allow for drawing into it.
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backBufferIndex].get(), beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);
    }
}

void DeviceResources::Present(D3D12_RESOURCE_STATES beforeState) noexcept(false) {
    if (beforeState != D3D12_RESOURCE_STATE_PRESENT) {
        // Transition the render target to the state that allows it to be presented to the display.
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backBufferIndex].get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);
    }

    ExecuteCommandList();

    HRESULT hr;
    if (m_options & c_AllowTearing) {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        // Note this will fail if in true 'fullscreen' mode.
        hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    } else {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        hr = m_swapChain->Present(1, 0);
    }

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
#ifdef _DEBUG
        wchar_t buff[64] = {};
        swprintf_s(buff, L"Device Lost on Present: Reason code 0x%08X\n",
                   (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr);
        OutputDebugStringW(buff);
#endif
        HandleDeviceLost();
    } else {
        winrt::check_hresult(hr);

        MoveToNextFrame();
    }
}

void DeviceResources::ExecuteCommandList() noexcept {
    winrt::check_hresult(m_commandList->Close());
    ID3D12CommandList* commandLists[] = {m_commandList.get()};
    m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
}

// Wait for pending GPU work to complete.
void DeviceResources::WaitForGpu() noexcept {
    if (m_commandQueue && m_fence && m_fenceEvent.get() != nullptr) {
        // Schedule a Signal command in the GPU queue.
        UINT64 fenceValue = m_fenceValues[m_backBufferIndex];
        if (SUCCEEDED(m_commandQueue->Signal(m_fence.get(), fenceValue))) {
            // Wait until the Signal has been processed.
            if (SUCCEEDED(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.get()))) {
                WaitForSingleObjectEx(m_fenceEvent.get(), INFINITE, FALSE);

                // Increment the fence value for the current frame.
                m_fenceValues[m_backBufferIndex]++;
            }
        }
    }
}

void DeviceResources::MoveToNextFrame() {
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
    winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), currentFenceValue));

    // Update the back buffer index.
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex]) {
        winrt::check_hresult(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.get()));
        WaitForSingleObjectEx(m_fenceEvent.get(), INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;
}

// This method acquires the first high-performance hardware adapter that supports Direct3D 12.
// If no such adapter can be found, try WARP. Otherwise throw an exception.
void DeviceResources::InitializeAdapter(IDXGIAdapter1** ppAdapter, DXGI_GPU_PREFERENCE preference) noexcept(false) {
    if (ppAdapter == nullptr)
        throw winrt::hresult_invalid_argument{};

    winrt::com_ptr<IDXGIAdapter1> adapter;

    winrt::com_ptr<IDXGIFactory6> factory6;
    if (HRESULT hr = m_dxgiFactory.try_as(factory6); FAILED(hr))
        winrt::throw_hresult(hr);

    for (UINT adapterID = 0;
         DXGI_ERROR_NOT_FOUND !=
         factory6->EnumAdapterByGpuPreference(adapterID, preference, __uuidof(IDXGIAdapter1), adapter.put_void());
         ++adapterID) {
        if (m_adapterIDoverride != UINT_MAX && adapterID != m_adapterIDoverride) {
            continue;
        }

        DXGI_ADAPTER_DESC1 desc;
        winrt::check_hresult(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
#ifdef _DEBUG
            wchar_t buff[256] = {};
            swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterID, desc.VendorId,
                       desc.DeviceId, desc.Description);
            OutputDebugStringW(buff);
#endif
            // We found a suitable adapter, store it in m_adapter
            m_adapter = adapter;
            // Also return it through the parameter
            adapter.copy_to(ppAdapter);
            return;
        }
    }

#if !defined(NDEBUG)
    if (m_adapterIDoverride == UINT_MAX) {
        // Try WARP12 instead
        if (HRESULT hr = m_dxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), adapter.put_void()); FAILED(hr))
            winrt::throw_hresult(hr);

        OutputDebugStringW(L"Direct3D Adapter - WARP12\n");

        // Store the WARP adapter
        m_adapter = adapter;
        adapter.copy_to(ppAdapter);
        return;
    }
#endif

    // If we get here, we failed to find a suitable adapter
    if (m_adapterIDoverride != UINT_MAX) {
        throw std::runtime_error("Unavailable adapter requested.");
    } else {
        throw std::runtime_error("Unavailable adapter.");
    }
}
} // namespace DX