#pragma once
// @file: Shared2Ifcs.h - Custom Windows COM interface declarations
// Purpose: Declares COM interfaces that will be implemented using winrt::implements
// Reference: https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iclassfactory-createinstance

#include <activation.h>
#include <objidl.h>
#include <unknwn.h>

// DLL Export macros
#ifdef SHARED2_EXPORTS
#define SHARED2_API __declspec(dllexport)
#else
#define SHARED2_API __declspec(dllimport)
#endif

// Forward declarations
struct ICustomService;

/**
 * @brief Custom COM service interface
 * @details Example interface for demonstrating COM implementation with C++/WinRT
 */
MIDL_INTERFACE("12345678-1234-5678-9ABC-123456789ABC")
ICustomService : public IUnknown {
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
    STDMETHOD(ProcessData)(const BYTE* input, DWORD inputSize, BYTE** output, DWORD* outputSize) = 0;

    /**
     * @brief Get service status information
     * @param status Pointer to receive status string (allocated by callee)
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(GetStatus)(LPWSTR * status) = 0;

    /**
     * @brief Cleanup service resources
     * @return S_OK on success, error HRESULT on failure
     */
    STDMETHOD(Shutdown)() = 0;
};

extern "C" {
SHARED2_API HRESULT STDAPICALLTYPE CreateCustomClassFactory(IClassFactory** output) noexcept;
}
