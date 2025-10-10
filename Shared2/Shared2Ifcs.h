/**
 * @file Shared2Ifcs.h - Custom Windows COM interface declarations
 * @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iclassfactory-createinstance
 */
#pragma once
#include <minwinbase.h>
#include <unknwn.h>

// DirectX headers for interface declarations
#include <d3d12.h>
#include <dxgi1_6.h>

// DLL Export macros
#ifdef SHARED2_EXPORTS
#define SHARED2_API __declspec(dllexport)
#else
#define SHARED2_API __declspec(dllimport)
#endif

// Forward declarations
struct IDeviceResources;

/**
 * @brief DirectX Device Resources COM interface
 * @details COM wrapper for DirectX 12 device and resource management
 * @note Designed for size-independent operations and basic resource creation
 */
MIDL_INTERFACE("23456789-2345-6789-ABCD-23456789ABCD")
IDeviceResources : public IUnknown {
    /**
     * @brief Initialize the Direct3D device with specified parameters
     * @param backBufferFormat Format for back buffer (e.g., DXGI_FORMAT_B8G8R8A8_UNORM)
     * @param depthBufferFormat Format for depth buffer (e.g., DXGI_FORMAT_D32_FLOAT)
     * @param backBufferCount Number of back buffers (typically 2-3)
     * @param minFeatureLevel Minimum D3D feature level required
     * @param flags Device creation flags (see DeviceResources constants)
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(InitializeDevice)(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
                                D3D_FEATURE_LEVEL minFeatureLevel, UINT flags) = 0;

    /**
     * @brief Create device-dependent resources
     * @details Sets up D3D12 device, command queue, and basic infrastructure
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(CreateDeviceResources)(IDXGIAdapter1* adapter = nullptr) = 0;

    /**
     * @brief Create size-dependent resources for specified dimensions
     * @param width Render target width in pixels
     * @param height Render target height in pixels
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(CreateWindowSizeDependentResources)(UINT width, UINT height) = 0;

    /**
     * @brief Handle device lost scenarios
     * @details Recreates all device resources after device removal/reset
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(HandleDeviceLost)() = 0;

    /**
     * @brief Get the current render target size
     * @param pWidth Pointer to receive width in pixels
     * @param pHeight Pointer to receive height in pixels
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetOutputSize)(UINT * pWidth, UINT * pHeight) = 0;

    /**
     * @brief Check if tearing (variable refresh rate) is supported
     * @param pSupported Pointer to receive boolean result
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(IsTearingSupported)(BOOL * pSupported) = 0;

    /**
     * @brief Get the D3D12 device interface
     * @param ppDevice Pointer to receive ID3D12Device interface
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetD3DDevice)(ID3D12Device * *ppDevice) = 0;

    /**
     * @brief Get the DXGI factory interface
     * @param ppFactory Pointer to receive IDXGIFactory4 interface
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetDXGIFactory)(IDXGIFactory4 * *ppFactory) = 0;

    /**
     * @brief Get the swap chain interface
     * @param ppSwapChain Pointer to receive IDXGISwapChain3 interface
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetSwapChain)(IDXGISwapChain3 * *ppSwapChain) = 0;

    /**
     * @brief Get the command queue interface
     * @param ppCommandQueue Pointer to receive ID3D12CommandQueue interface
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetCommandQueue)(ID3D12CommandQueue * *ppCommandQueue) = 0;

    /**
     * @brief Get the current feature level
     * @param pFeatureLevel Pointer to receive D3D_FEATURE_LEVEL
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetDeviceFeatureLevel)(D3D_FEATURE_LEVEL * pFeatureLevel) = 0;

    /**
     * @brief Get back buffer format
     * @param pFormat Pointer to receive DXGI_FORMAT
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetBackBufferFormat)(DXGI_FORMAT * pFormat) = 0;

    /**
     * @brief Get depth buffer format
     * @param pFormat Pointer to receive DXGI_FORMAT
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetDepthBufferFormat)(DXGI_FORMAT * pFormat) = 0;

    /**
     * @brief Prepare for rendering (setup command list and render targets)
     * @param beforeState Resource state before rendering operations
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(Prepare)(D3D12_RESOURCE_STATES beforeState) = 0;

    /**
     * @brief Present the rendered frame
     * @param beforeState Resource state before present operation
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(Present)(D3D12_RESOURCE_STATES beforeState) = 0;

    /**
     * @brief Execute the current command list
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(ExecuteCommandList)() = 0;

    /**
     * @brief Wait for GPU operations to complete
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(WaitForGpu)() = 0;

    /**
     * @brief Set the name for DirectX resources for debugging/profiling
     * @param name Wide character string name to assign to resources
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(SetName)(LPCWSTR name, UINT32 namelen) = 0;
};

extern "C" {
SHARED2_API HRESULT STDAPICALLTYPE CreateCustomClassFactory(::IClassFactory** output) noexcept;
SHARED2_API HRESULT STDAPICALLTYPE CreateDeviceResources(REFIID riid, void** ppv) noexcept;
}
