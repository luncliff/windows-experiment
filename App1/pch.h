#pragma once
// clang-format off
#include <windows.h>
#include <unknwn.h>
#include <hstring.h>
#include <restrictederrorinfo.h>
// clang-format on

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>
#include <winrt/Microsoft.Windows.ApplicationModel.Resources.h>
#include <winrt/Microsoft.Windows.ApplicationModel.WindowsAppRuntime.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Notifications.h>

// Standard C++
#include <filesystem>
#include <string_view>
#include <system_error>

namespace winrt::App1 {

void set_log_stream(const char* name) noexcept(false);

/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew
 * @see GetModuleFileNameW
 */
DWORD get_module_path(WCHAR* path, UINT capacity) noexcept(false);

std::filesystem::path get_module_path() noexcept(false);

using Microsoft::UI::Dispatching::DispatcherQueue;

struct resume_on_ui final : public std::suspend_always {
    Microsoft::UI::Dispatching::DispatcherQueue queue;

  public:
    resume_on_ui(Microsoft::UI::Dispatching::DispatcherQueue queue) noexcept : queue{queue} {
    }
    void await_suspend(std::coroutine_handle<void> handle) const noexcept {
        queue.TryEnqueue(handle);
    }
};

} // namespace winrt::App1
