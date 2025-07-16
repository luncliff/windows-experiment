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
    RequestedTheme(ApplicationTheme::Light);
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    //auto handler = std::bind(&App::OnUnhandledException, this, _1, _2);
    UnhandledException({this, &App::OnUnhandledException});
#endif
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

/// @see DISABLE_XAML_GENERATED_MAIN
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    // setup the logger and print the current executable's path
    winrt::App1::set_log_stream("App");
    auto exe = winrt::App1::get_module_path();
    spdlog::debug(L"{}", exe.native());

    winrt::Microsoft::UI::Xaml::Application::Start([](auto&&) {
        // ... start the application lifecycle ...
        winrt::make<winrt::App1::implementation::App>();
    });
    return S_OK;
}
