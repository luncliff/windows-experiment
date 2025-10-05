#pragma once
// @file: Shared1 PCH - Core includes for shared static library
// Scope: Minimal WinRT headers + ILoggingChannel implementations for shared logic

// clang-format off
#include <windows.h>
#include <unknwn.h>
#include <hstring.h>
#include <restrictederrorinfo.h>
// clang-format on

#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Diagnostics.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.ApplicationModel.h>

#if __has_include(<d3dx12/d3dx12.h>) // DirectX Agility SDK from NuGet
#include <d3dx12/d3dx12.h>
#elif __has_include(<directx/d3dx12.h>) // DirectX Agility SDK from vcpkg
#include <directx/d3dx12.h>
#endif
// clang-format off
#include <d3d12.h>
#include <dxgi1_6.h>
// clang-format on

// Standard C++
#include <filesystem>
#include <string_view>
#include <system_error>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <format>
#include <chrono>

namespace winrt::App1 {

// Forward declarations
struct NullLoggingChannel;
struct StreamLoggingChannel;

/**
 * @brief Null object implementation of ILoggingChannel
 * @details No-op implementation for when logging is disabled
 */
struct NullLoggingChannel : winrt::implements<NullLoggingChannel, Windows::Foundation::Diagnostics::ILoggingChannel> {
    void LogMessage(winrt::hstring const&) const noexcept {}
    void LogMessage(winrt::hstring const&, Windows::Foundation::Diagnostics::LoggingLevel) const noexcept {}
    void LogMessage(winrt::hstring const&, Windows::Foundation::Diagnostics::LoggingLevel, Windows::Foundation::Diagnostics::LoggingOptions const&) const noexcept {}
    void LogValuePair(winrt::hstring const&, int32_t) const noexcept {}
    void LogValuePair(winrt::hstring const&, int32_t, Windows::Foundation::Diagnostics::LoggingLevel) const noexcept {}
    void StartActivity(winrt::hstring const&) const noexcept {}
    void StopActivity(winrt::hstring const&) const noexcept {}
    
    // ILoggingChannel required properties (return defaults)
    winrt::hstring Name() const noexcept { return L""; }
    bool Enabled() const noexcept { return false; }
    Windows::Foundation::Diagnostics::LoggingLevel Level() const noexcept { 
        return Windows::Foundation::Diagnostics::LoggingLevel::Verbose; 
    }
    
    // Events (no-op)
    winrt::event_token LoggingEnabled(Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::ILoggingChannel, winrt::Windows::Foundation::IInspectable> const&) const noexcept { return {}; }
    void LoggingEnabled(winrt::event_token const&) const noexcept {}
};

/**
 * @brief Stream-based implementation of ILoggingChannel  
 * @details Redirects log messages to std::wostream for testing/diagnostics
 */
struct StreamLoggingChannel : winrt::implements<StreamLoggingChannel, Windows::Foundation::Diagnostics::ILoggingChannel> {
private:
    std::wostream* m_stream;
    winrt::hstring m_name;

public:
    explicit StreamLoggingChannel(std::wostream& stream, winrt::hstring const& name = L"StreamChannel") noexcept 
        : m_stream(&stream), m_name(name) {}
        
    void LogMessage(winrt::hstring const& message) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << message.c_str() << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    void LogMessage(winrt::hstring const& message, Windows::Foundation::Diagnostics::LoggingLevel level) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << L"[" << static_cast<int>(level) << L"] " << message.c_str() << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    void LogMessage(winrt::hstring const& message, Windows::Foundation::Diagnostics::LoggingLevel level, Windows::Foundation::Diagnostics::LoggingOptions const&) const noexcept {
        LogMessage(message, level);
    }
    
    void LogValuePair(winrt::hstring const& key, int32_t value) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << key.c_str() << L"=" << value << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    void LogValuePair(winrt::hstring const& key, int32_t value, Windows::Foundation::Diagnostics::LoggingLevel level) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << L"[" << static_cast<int>(level) << L"] " << key.c_str() << L"=" << value << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    void StartActivity(winrt::hstring const& activityName) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << L"START: " << activityName.c_str() << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    void StopActivity(winrt::hstring const& activityName) const noexcept {
        if (m_stream) {
            try {
                (*m_stream) << L"STOP: " << activityName.c_str() << L'\n';
                m_stream->flush();
            } catch (...) {
                // Ignore stream errors in logging
            }
        }
    }
    
    // ILoggingChannel required properties
    winrt::hstring Name() const noexcept { return m_name; }
    bool Enabled() const noexcept { return m_stream != nullptr; }
    Windows::Foundation::Diagnostics::LoggingLevel Level() const noexcept { 
        return Windows::Foundation::Diagnostics::LoggingLevel::Verbose; 
    }
    
    // Events (no-op for stream implementation)
    winrt::event_token LoggingEnabled(Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::ILoggingChannel, winrt::Windows::Foundation::IInspectable> const&) const noexcept { return {}; }
    void LoggingEnabled(winrt::event_token const&) const noexcept {}
};

/**
 * @brief Factory functions for ILoggingChannel implementations
 */
inline Windows::Foundation::Diagnostics::ILoggingChannel make_null_logging_channel() {
    return winrt::make<NullLoggingChannel>();
}

inline Windows::Foundation::Diagnostics::ILoggingChannel make_stream_logging_channel(std::wostream& stream, winrt::hstring const& name = L"StreamChannel") {
    return winrt::make<StreamLoggingChannel>(stream, name);
}

} // namespace winrt::App1