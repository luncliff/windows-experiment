#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include "SettingsViewModel.h"

#include <source_location>
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

using namespace std::placeholders;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;

App::App() noexcept(false) {
    InitializeComponent();
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException({this, &App::OnUnhandledException});
#endif
    provider.Settings(winrt::make<implementation::SettingsViewModel>());
}

App::~App() noexcept {
    clear_settings_event();
}

void App::OnUnhandledException(IInspectable const&, UnhandledExceptionEventArgs const& e) {
    winrt::hstring txt = e.Message();
    if (IsDebuggerPresent())
        __debugbreak();
    spdlog::critical(L"App: {:s}", static_cast<std::wstring_view>(txt));
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
    spdlog::info("App: {:s}", std::source_location::current().function_name());
    // note: the App will be the first which receives the events
    settings_changed_token = provider.Settings().PropertyChanged({this, &App::on_settings_changed});
    auto w = winrt::make<implementation::MainWindow>();
    w.Provider(provider);
    window = w;
    window.Activate();
}

void App::on_settings_changed(IInspectable const&, PropertyChangedEventArgs const& e) {
    // ... reserved section. nothing to do for now ...
    spdlog::debug(L"App: changed - {}", e.PropertyName());
}

void App::clear_settings_event() noexcept {
    auto viewmodel = provider.Settings();
    if (viewmodel == nullptr)
        return;
    if (settings_changed_token) {
        viewmodel.PropertyChanged(settings_changed_token);
        settings_changed_token = {};
    }
}

} // namespace winrt::App1::implementation

/// @see https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/migrate-to-windows-app-sdk/guides/applifecycle#single-instancing-in-main-or-wwinmain
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // try creation of named mutex to enforce single instance
    winrt::handle mutex{CreateMutexW(nullptr, true, L"We miss you so much")};
    if (static_cast<bool>(mutex) == false) {
        const auto ec = GetLastError();
        if (ec == ERROR_ALREADY_EXISTS) // another instance is running
            return S_OK;
        return ec; // mutex creation failed
    }

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