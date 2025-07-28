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
using Microsoft::UI::Xaml::ApplicationTheme;

App::App() noexcept(false) {
    InitializeComponent();
    set_log_stream("App");
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    //auto handler = std::bind(&App::OnUnhandledException, this, _1, _2);
    UnhandledException({this, &App::OnUnhandledException});
#endif
    RequestedTheme(ApplicationTheme::Dark);
    settings = winrt::make<implementation::SettingsViewModel>();
    // note: the App will be the first which receives the events
    settings_changed_token = settings.PropertyChanged({this, &App::on_settings_changed});
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
    auto impl = winrt::make<implementation::MainWindow>();
    impl.Settings(settings);
    window = impl;
    window.Activate();
}

void App::on_settings_changed(IInspectable const&, PropertyChangedEventArgs const& e) {
    // ... reserved section. nothing to do for now ...
    spdlog::debug(L"App: changed - {}", e.PropertyName());
}

void App::clear_settings_event() noexcept {
    auto viewmodel = settings;
    if (viewmodel == nullptr)
        return;
    if (settings_changed_token) {
        viewmodel.PropertyChanged(settings_changed_token);
        settings_changed_token = {};
    }
}

} // namespace winrt::App1::implementation
