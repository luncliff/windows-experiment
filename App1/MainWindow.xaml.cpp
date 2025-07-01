#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "BasicViewModel.h"
#include "SettingsPage.xaml.h"
#include "SupportPage.xaml.h"
#include "TestPage1.xaml.h"

#include <Windows.h>
#include <microsoft.ui.xaml.window.h>

#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

using Microsoft::UI::Xaml::Controls::Button;

/// @see https://learn.microsoft.com/pl-pl/windows/apps/design/controls/navigationview
MainWindow::MainWindow() {
    ExtendsContentIntoTitleBar(true);
    spdlog::info("HWND {:p}", reinterpret_cast<void*>(WindowHandle()));
    // Create the shared ViewModels
    viewmodel0 = winrt::make<implementation::BasicViewModel>();
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
        frame.Navigate(xaml_typename<App1::SettingsPage>()); // same with App1.SettingsPage
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
