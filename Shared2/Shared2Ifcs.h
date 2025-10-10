#pragma once
// @file: Shared2Ifcs.h - Custom Windows COM interface declarations
// Purpose: Declares COM interfaces that will be implemented using winrt::implements
// Reference: https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iclassfactory-createinstance

#include <unknwn.h>
#include <objidl.h>
#include <activation.h>

// DLL Export macros
#ifdef SHARED2_EXPORTS
#define SHARED2_API __declspec(dllexport)
#else
#define SHARED2_API __declspec(dllimport)
#endif

// Forward declarations
struct ICustomService;
struct ICustomClassFactory;

/**
 * @brief Custom COM service interface
 * @details Example interface for demonstrating COM implementation with C++/WinRT
 */
MIDL_INTERFACE("12345678-1234-5678-9ABC-123456789ABC")
ICustomService : public IUnknown
{
    /**
     * @brief Initialize the service with configuration parameters
     * @param config Configuration string
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(Initialize)(LPCWSTR config) = 0;
    
    /**
     * @brief Process data using the service
     * @param input Input data buffer
     * @param inputSize Size of input buffer in bytes
     * @param output Output data buffer (allocated by callee)
     * @param outputSize Size of output buffer in bytes
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(ProcessData)(
        const BYTE* input,
        DWORD inputSize,
        BYTE** output,
        DWORD* outputSize) = 0;
    
    /**
     * @brief Get service status information
     * @param status Pointer to receive status string (allocated by callee)
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetStatus)(LPWSTR* status) = 0;
    
    /**
     * @brief Cleanup service resources
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(Shutdown)() = 0;
};

/**
 * @brief Custom class factory interface for creating ICustomService instances
 * @details Extends IClassFactory with custom creation methods
 */
MIDL_INTERFACE("87654321-4321-8765-CBA9-987654321CBA")
ICustomClassFactory : public IClassFactory
{
    /**
     * @brief Create service instance with specific configuration
     * @param config Configuration parameters
     * @param riid Interface ID to query for
     * @param ppvObject Pointer to receive the created object
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(CreateConfiguredInstance)(
        LPCWSTR config,
        REFIID riid,
        void** ppvObject) = 0;
    
    /**
     * @brief Get factory information
     * @param info Pointer to receive factory info string (allocated by callee)
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetFactoryInfo)(LPWSTR* info) = 0;
};

// Interface IDs as constexpr for use with winrt::implements
constexpr winrt::guid IID_ICustomService{ 0x12345678, 0x1234, 0x5678, { 0x9A, 0xBC, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC } };
constexpr winrt::guid IID_ICustomClassFactory{ 0x87654321, 0x4321, 0x8765, { 0xCB, 0xA9, 0x98, 0x76, 0x54, 0x32, 0x1C, 0xBA } };

// Class IDs for COM registration
constexpr winrt::guid CLSID_CustomService{ 0xABCDEF01, 0x2345, 0x6789, { 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89 } };
constexpr winrt::guid CLSID_CustomClassFactory{ 0xFEDCBA98, 0x7654, 0x3210, { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 } };

// Standard DLL exports for COM registration
extern "C" {
    /**
     * @brief Standard COM DLL export - determines if DLL can be unloaded
     * @return S_OK if DLL can be unloaded, S_FALSE otherwise
     */
    SHARED2_API HRESULT STDAPICALLTYPE Shared2_DllCanUnloadNow();
    
    /**
     * @brief Factory function to create ICustomService instances
     * @param riid Interface ID to query for
     * @param ppvObject Pointer to receive the created object
     * @return S_OK on success, error HRESULT on failure
     */
    SHARED2_API HRESULT STDAPICALLTYPE CreateCustomService(REFIID riid, void** ppvObject);
    
    /**
     * @brief Factory function to create ICustomClassFactory instances
     * @param riid Interface ID to query for
     * @param ppvObject Pointer to receive the created object
     * @return S_OK on success, error HRESULT on failure
     */
    SHARED2_API HRESULT STDAPICALLTYPE CreateCustomClassFactory(REFIID riid, void** ppvObject);
}