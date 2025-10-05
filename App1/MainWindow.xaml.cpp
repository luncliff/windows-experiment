#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "SettingsPage.xaml.h"
#include "SupportPage.xaml.h"
#include "TestPage1.xaml.h"

#include <microsoft.ui.xaml.window.h>
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

/// @see https://learn.microsoft.com/pl-pl/windows/apps/design/controls/navigationview
MainWindow::MainWindow() noexcept(false) {
    ExtendsContentIntoTitleBar(true);
    spdlog::info("HWND {:p}", reinterpret_cast<void*>(WindowHandle()));
    // Create the shared ViewModels
    viewmodel0 = Shared1::BasicViewModel{};
}

MainWindow::~MainWindow() noexcept {
    clear_settings_event();
}

void MainWindow::clear_settings_event() noexcept {
    // Disconnect from SettingsViewModel property changes
    auto viewmodel = Settings();
    if (viewmodel == nullptr)
        return;
    if (settings_changed_token) {
        viewmodel.PropertyChanged(settings_changed_token);
        settings_changed_token = {};
    }
}

App1::SettingsViewModel MainWindow::Settings() const noexcept {
    return viewmodel1;
}

void MainWindow::Settings(App1::SettingsViewModel viewmodel) noexcept(false) {
    if (viewmodel == nullptr)
        throw winrt::hresult_invalid_argument(L"SettingsViewModel cannot be null");

    clear_settings_event();
    viewmodel1 = viewmodel;
    // Connect to new SettingsViewModel property changes
    settings_changed_token = viewmodel1.PropertyChanged({this, &MainWindow::on_settings_changed});
}

void MainWindow::on_settings_changed(IInspectable const&, PropertyChangedEventArgs const& e) {
    if (e.PropertyName() == L"Counter") {
        // ... nothing to do for now ...
    }
}

/// @see https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/retrieve-hwnd
uint64_t MainWindow::WindowHandle() const noexcept {
    auto native = this->try_as<::IWindowNative>();
    HWND hwnd = nullptr;
    if (auto hr = native->get_WindowHandle(&hwnd); FAILED(hr))
        winrt::throw_hresult(hr);
    return reinterpret_cast<uint64_t>(hwnd);
}

void MainWindow::on_window_size_changed(IInspectable const&, WindowSizeChangedEventArgs const& e) {
    auto s = e.Size();
    spdlog::info("{}: size ({:.2f},{:.2f})", "MainWindow", s.Width, s.Height);
}

void MainWindow::on_window_visibility_changed(IInspectable const&, WindowVisibilityChangedEventArgs const& e) {
    spdlog::info("{}: visibility {}", "MainWindow", e.Visible());
}

void MainWindow::on_item_invoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& e) {
    // there are very limited types for params... read the document.
    // see https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.frame.navigate
    Frame frame = ShellFrame();
    if (e.IsSettingsInvoked()) {
        // SettingsPage will use the SettingsViewModel
        frame.Navigate(xaml_typename<App1::SettingsPage>(), Settings());
        return;
    }

    auto item = e.InvokedItemContainer().as<NavigationViewItem>();
    if (item == nullptr)
        return;

    IInspectable param = viewmodel0;
    auto tag = unbox_value_or<winrt::hstring>(item.Tag(), L"");
    if (tag == L"TestPage1") {
        frame.Navigate(xaml_typename<App1::TestPage1>(), param);
        return;
    }
    if (tag == L"SupportPage") {
        frame.Navigate(xaml_typename<App1::SupportPage>(), param);
        return;
    }
}

void MainWindow::on_back_requested(NavigationView const&, NavigationViewBackRequestedEventArgs const&) {
    Frame frame = ShellFrame();
    if (frame.CanGoBack())
        frame.GoBack();
}

} // namespace winrt::App1::implementation
