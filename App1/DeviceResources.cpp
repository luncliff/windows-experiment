﻿/**
 * @file DeviceResources.cpp
 * @brief A wrapper for the Direct3D 12 device and swapchain
 * @see https://github.com/Microsoft/DirectML/blob/master/Samples/DirectMLSuperResolution/DeviceResources.cpp
 * @see https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingHelloWorld/DeviceResources.cpp
 * @see https://github.com/Microsoft/DirectXTK12/wiki/DeviceResources
 * 
 * @copyright Copyright (c) Microsoft Corporation. All rights reserved. Licensed under the MIT License.
 *
 * @note Changes
 *  - clang-format with the repository style
 *  - Recomment with Doxygen tags
 *  - Use winrt::com_ptr instead of Microsoft::WRL::winrt::com_ptr
 *  - Use winrt::handle
 *  - Update header includes
 *  - GetAdapter receives DXGI_GPU_PREFERENCE
 *  - Linker configuration for Debug build
 *  - Move member functions to DeviceResources.cpp file
 *  - Replace SetWindow to WindowSizeChanged, remove m_window
 *  - Change RECT m_outputSize to SIZE m_outputSize
 *  - Create CreateSizeDependentResources, organize common part to UpdateSwapChainResources
 */
#include "pch.h"

#include "DeviceResources.h"
#include <dxgidebug.h>

#if defined(_DEBUG)
#pragma comment(lib, "DXGI") // DXGIGetDebugInterface1
#endif

using namespace DirectX;

namespace DX {

namespace {
inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) {
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
}; // namespace

// Constructor for DeviceResources.
DeviceResources::DeviceResources(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
                                 D3D_FEATURE_LEVEL minFeatureLevel, unsigned int flags) noexcept(false)
    : m_backBufferIndex(0), m_fenceValues{}, m_rtvDescriptorSize(0), m_screenViewport{}, m_scissorRect{},
      m_backBufferFormat(backBufferFormat), m_depthBufferFormat(depthBufferFormat), m_backBufferCount(backBufferCount),
      m_d3dMinFeatureLevel(minFeatureLevel), m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0), m_dxgiFactoryFlags(0),
      m_outputSize{0, 0}, m_colorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709), m_options(flags),
      m_deviceNotify(nullptr) {
    if (backBufferCount > MAX_BACK_BUFFER_COUNT) {
        throw std::out_of_range("backBufferCount too large");
    }

    if (minFeatureLevel < D3D_FEATURE_LEVEL_11_0) {
        throw std::out_of_range("minFeatureLevel too low");
    }
}

// Destructor for DeviceResources.
DeviceResources::~DeviceResources() {
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    WaitForGpu();
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DeviceResources::CreateDeviceResources() {
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    //
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        winrt::com_ptr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.put())))) {
            debugController->EnableDebugLayer();
        } else {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }

        winrt::com_ptr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.put())))) {
            m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which
                      the swapchain's window resides. */
                ,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }
#endif

    winrt::check_hresult(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.put())));

    // Determines whether tearing support is available for fullscreen borderless windows.
    if (m_options & c_AllowTearing) {
        BOOL allowTearing = FALSE;

        winrt::com_ptr<IDXGIFactory5> factory5;
        HRESULT hr = m_dxgiFactory->QueryInterface(factory5.put());
        if (SUCCEEDED(hr)) {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing) {
            m_options &= ~c_AllowTearing;
#ifdef _DEBUG
            OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
        }
    }

    winrt::com_ptr<IDXGIAdapter1> adapter;
    GetAdapter(adapter.put());

    // Create the DX12 API device object.
    winrt::check_hresult(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, IID_PPV_ARGS(m_d3dDevice.put())));

    m_d3dDevice->SetName(L"DeviceResources");

#ifndef NDEBUG
    // Configure debug device (if active).
    winrt::com_ptr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(m_d3dDevice->QueryInterface(d3dInfoQueue.put()))) {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] = {D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                                   D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE};
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

    winrt::check_hresult(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.put())));

    m_commandQueue->SetName(L"DeviceResources");

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_backBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    winrt::check_hresult(
        m_d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(m_rtvDescriptorHeap.put())));

    m_rtvDescriptorHeap->SetName(L"DeviceResources");

    m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        winrt::check_hresult(
            m_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(m_dsvDescriptorHeap.put())));

        m_dsvDescriptorHeap->SetName(L"DeviceResources");
    }

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        winrt::check_hresult(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                                 IID_PPV_ARGS(m_commandAllocators[n].put())));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_commandAllocators[n]->SetName(name);
    }

    // Create a command list for recording graphics commands.
    winrt::check_hresult(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].get(),
                                                        nullptr, IID_PPV_ARGS(m_commandList.put())));
    winrt::check_hresult(m_commandList->Close());

    m_commandList->SetName(L"DeviceResources");

    // Create a fence for tracking GPU execution progress.
    winrt::check_hresult(
        m_d3dDevice->CreateFence(m_fenceValues[m_backBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())));
    m_fenceValues[m_backBufferIndex]++;

    m_fence->SetName(L"DeviceResources");

    m_fenceEvent.attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!m_fenceEvent) {
        throw std::exception("CreateEvent");
    }
}

// These resources need to be recreated every time the window size is changed.
void DeviceResources::CreateWindowSizeDependentResources(HWND window, int width, int height) {
    if (!window)
        throw std::invalid_argument("Call with a valid Win32 window handle");

    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        m_renderTargets[n] = nullptr;
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    // Determine the render target size in pixels.
    UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(width), 1u);
    UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(height), 1u);
    DXGI_FORMAT backBufferFormat = NoSRGB(m_backBufferFormat);

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain) {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(m_backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat,
                                                (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                      (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this
            // method and correctly set up the new device.
            return;
        } else {
            winrt::check_hresult(hr);
        }
    } else {
        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = m_backBufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags = (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a swap chain for the window.
        winrt::com_ptr<IDXGISwapChain1> swapChain;
        winrt::check_hresult(m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue.get(), window, &swapChainDesc,
                                                                   &fsSwapChainDesc, nullptr, swapChain.put()));

        winrt::check_hresult(swapChain->QueryInterface(m_swapChain.put()));

        // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER
        // shortcut
        winrt::check_hresult(m_dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Handle color space settings for HDR
    UpdateColorSpace();
    UpdateSwapChainResources(backBufferWidth, backBufferHeight);
}

void DeviceResources::UpdateSwapChainResources(UINT backBufferWidth, UINT backBufferHeight) {
    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        winrt::check_hresult(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].put())));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_renderTargets[n]->SetName(name);

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = m_backBufferFormat;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                                    static_cast<INT>(n), m_rtvDescriptorSize);
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
            &depthOptimizedClearValue, IID_PPV_ARGS(m_depthStencil.put())));

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
    m_scissorRect.right = static_cast<LONG>(backBufferWidth);
    m_scissorRect.bottom = static_cast<LONG>(backBufferHeight);
}

void DeviceResources::CreateSizeDependentResources(UINT width, UINT height) {
    if (width == 0 || height == 0)
        throw std::invalid_argument{__FUNCTION__};
    if ((width == m_outputSize.cx) && (height == m_outputSize.cy))
        return;

    m_outputSize.cx = width;
    m_outputSize.cy = height;

    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < m_backBufferCount; n++) {
        m_renderTargets[n] = nullptr;
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    // Determine the render target size in pixels.
    UINT backBufferWidth = std::max<UINT>(width, 1u);
    UINT backBufferHeight = std::max<UINT>(height, 1u);
    DXGI_FORMAT backBufferFormat = NoSRGB(m_backBufferFormat);

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain) {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(m_backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat,
                                                (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                      (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this
            // method and correctly set up the new device.
            return;
        } else {
            winrt::check_hresult(hr);
        }
    } else {
        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
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
        if (HRESULT hr = m_dxgiFactory->CreateSwapChainForComposition(m_commandQueue.get(), &swapChainDesc, nullptr,
                                                                      swapChain.put());
            FAILED(hr))
            winrt::check_hresult(hr);

        m_swapChain = swapChain.try_as<IDXGISwapChain3>();
    }

    // Handle color space settings for HDR
    UpdateColorSpace();
    UpdateSwapChainResources(backBufferWidth, backBufferHeight);
}

// This method is called when the Win32 window changes size.
bool DeviceResources::WindowSizeChanged(HWND window, int width, int height) {
    if (width == 0 || height == 0)
        throw std::invalid_argument{__FUNCTION__};

    if ((width == m_outputSize.cx) && (height == m_outputSize.cy)) {
        // Handle color space settings for HDR
        UpdateColorSpace();

        return false;
    }

    m_outputSize.cx = width;
    m_outputSize.cy = height;
    CreateWindowSizeDependentResources(window, width, height);
    return true;
}

// Recreate all device resources and set them back to the current state.
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

#ifdef _DEBUG
    {
        winrt::com_ptr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                         DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
#endif

    CreateDeviceResources();
    WaitForGpu();

    if (m_deviceNotify) {
        m_deviceNotify->OnDeviceRestored();
    }
}

void DeviceResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify) {
    m_deviceNotify = deviceNotify;
}

// Prepare the command list and render target for rendering.
void DeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState) {
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

// Present the contents of the swap chain to the screen.
void DeviceResources::Present(D3D12_RESOURCE_STATES beforeState) {
    if (beforeState != D3D12_RESOURCE_STATE_PRESENT) {
        // Transition the render target to the state that allows it to be presented to the display.
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backBufferIndex].get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);
    }

    // Send the command list off to the GPU for processing.
    winrt::check_hresult(m_commandList->Close());
    m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.put()));

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
        char buff[64] = {};
        sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
                  (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr);
        OutputDebugStringA(buff);
#endif
        HandleDeviceLost();
    } else {
        winrt::check_hresult(hr);

        MoveToNextFrame();

        if (!m_dxgiFactory->IsCurrent()) {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            winrt::check_hresult(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.put())));
        }
    }
}

// Wait for pending GPU work to complete.
void DeviceResources::WaitForGpu() noexcept {
    if (m_commandQueue && m_fence && m_fenceEvent) {
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

// Device Accessors.
SIZE DeviceResources::GetOutputSize() const {
    return m_outputSize;
}

// Direct3D Accessors.
ID3D12Device* DeviceResources::GetD3DDevice() const {
    return m_d3dDevice.get();
}
IDXGISwapChain3* DeviceResources::GetSwapChain() const {
    return m_swapChain.get();
}
IDXGIFactory4* DeviceResources::GetDXGIFactory() const {
    return m_dxgiFactory.get();
}
D3D_FEATURE_LEVEL DeviceResources::GetDeviceFeatureLevel() const {
    return m_d3dFeatureLevel;
}
ID3D12Resource* DeviceResources::GetRenderTarget() const {
    return m_renderTargets[m_backBufferIndex].get();
}
ID3D12Resource* DeviceResources::GetDepthStencil() const {
    return m_depthStencil.get();
}
ID3D12CommandQueue* DeviceResources::GetCommandQueue() const {
    return m_commandQueue.get();
}
ID3D12CommandAllocator* DeviceResources::GetCommandAllocator() const {
    return m_commandAllocators[m_backBufferIndex].get();
}
ID3D12GraphicsCommandList* DeviceResources::GetCommandList() const {
    return m_commandList.get();
}
DXGI_FORMAT DeviceResources::GetBackBufferFormat() const {
    return m_backBufferFormat;
}
DXGI_FORMAT DeviceResources::GetDepthBufferFormat() const {
    return m_depthBufferFormat;
}
D3D12_VIEWPORT DeviceResources::GetScreenViewport() const {
    return m_screenViewport;
}
D3D12_RECT DeviceResources::GetScissorRect() const {
    return m_scissorRect;
}
UINT DeviceResources::GetCurrentFrameIndex() const {
    return m_backBufferIndex;
}
UINT DeviceResources::GetBackBufferCount() const {
    return m_backBufferCount;
}
DXGI_COLOR_SPACE_TYPE DeviceResources::GetColorSpace() const {
    return m_colorSpace;
}

uint32_t DeviceResources::GetDeviceOptions() const {
    return m_options;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DeviceResources::GetRenderTargetView() const {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                         static_cast<INT>(m_backBufferIndex), m_rtvDescriptorSize);
}
CD3DX12_CPU_DESCRIPTOR_HANDLE DeviceResources::GetDepthStencilView() const {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
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

void DeviceResources::GetAdapter(IDXGIAdapter1** ppAdapter, DXGI_GPU_PREFERENCE preference) {
    *ppAdapter = nullptr;

    winrt::com_ptr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
    winrt::com_ptr<IDXGIFactory6> factory6;
    HRESULT hr = m_dxgiFactory->QueryInterface(factory6.put());
    if (SUCCEEDED(hr)) {
        for (UINT adapterIndex = 0;
             SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, preference, IID_PPV_ARGS(adapter.put())));
             adapterIndex++) {
            DXGI_ADAPTER_DESC1 desc;
            winrt::check_hresult(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr))) {
#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId,
                           desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif
                break;
            }
        }
    }
#endif
    if (!adapter) {
        for (UINT adapterIndex = 0; SUCCEEDED(m_dxgiFactory->EnumAdapters1(adapterIndex, adapter.put()));
             ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            winrt::check_hresult(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.get(), m_d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr))) {
#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId,
                           desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif
                break;
            }
        }
    }

#if !defined(NDEBUG)
    if (!adapter) {
        // Try WARP12 instead
        if (FAILED(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.put())))) {
            throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }

        OutputDebugStringA("Direct3D Adapter - WARP12\n");
    }
#endif

    if (!adapter) {
        throw std::exception("No Direct3D 12 device found");
    }

    winrt::check_hresult(adapter->QueryInterface(ppAdapter));
}

// Sets the color space for the swap chain in order to handle HDR output.
void DeviceResources::UpdateColorSpace() {
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
    if (m_swapChain) {
        winrt::com_ptr<IDXGIOutput> output;
        if (SUCCEEDED(m_swapChain->GetContainingOutput(output.put()))) {
            winrt::com_ptr<IDXGIOutput6> output6;
            if (SUCCEEDED(output->QueryInterface(output6.put()))) {
                DXGI_OUTPUT_DESC1 desc;
                winrt::check_hresult(output6->GetDesc1(&desc));

                if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
                    // Display output is HDR10.
                    isDisplayHDR10 = true;
                }
            }
        }
    }
#endif

    if ((m_options & c_EnableHDR) && isDisplayHDR10) {
        switch (m_backBufferFormat) {
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            // The application creates the HDR10 signal.
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            break;

        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            // The system creates the HDR10 signal; application uses linear values.
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            break;

        default:
            break;
        }
    }

    m_colorSpace = colorSpace;

    UINT colorSpaceSupport = 0;
    if (SUCCEEDED(m_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
        (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {
        winrt::check_hresult(m_swapChain->SetColorSpace1(colorSpace));
    }
}

} // namespace DX
