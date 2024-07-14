#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include <source_location>
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

using namespace std::placeholders;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using Microsoft::UI::Xaml::ApplicationTheme;

App::App() {
    InitializeComponent();
    set_log_stream("App");

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    //auto handler = std::bind(&App::OnUnhandledException, this, _1, _2);
    UnhandledException({this, &App::OnUnhandledException});
#endif
    RequestedTheme(ApplicationTheme::Dark);
}

void App::OnUnhandledException(IInspectable const&, UnhandledExceptionEventArgs const& e) {
    if (IsDebuggerPresent() == false)
        // ...
        return;
    winrt::hstring txt = e.Message();
    spdlog::critical(L"App: {:s}", static_cast<std::wstring_view>(txt));
    __debugbreak();
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
    spdlog::info("App: {:s}", std::source_location::current().function_name());
    window = make<MainWindow>();
    window.Activate();
}

} // namespace winrt::App1::implementation
