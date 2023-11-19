#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <microsoft.ui.xaml.window.h>

#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

using Microsoft::UI::Xaml::Controls::Button;
using Windows::UI::Xaml::Interop::TypeKind;
using Windows::UI::Xaml::Interop::TypeName;

MainWindow::MainWindow() {
    InitializeComponent();
    // see
    // https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/retrieve-hwnd
    auto native = this->try_as<::IWindowNative>();
    if (auto hr = native->get_WindowHandle(&hwnd); FAILED(hr))
        winrt::throw_hresult(hr);
    spdlog::info("{}: HWND {:p}", "MainWindow", static_cast<void*>(hwnd));
}

void MainWindow::on_window_size_changed(IInspectable const&, WindowSizeChangedEventArgs const& e) {
    auto s = e.Size();
    spdlog::info("{}: size ({:.2f},{:.2f})", "MainWindow", s.Width, s.Height);
}

void MainWindow::on_window_visibility_changed(IInspectable const&, WindowVisibilityChangedEventArgs const& e) {
    spdlog::info("{}: visibility {}", "MainWindow", e.Visible());
}

void MainWindow::on_item_invoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& e) {
    spdlog::info("{}: {}", "MainWindow", __func__);
    auto item = e.InvokedItem().as<winrt::hstring>();
    // see XamlTypeInfo.g.cpp
    // see https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.frame.navigate
    Frame frame = ShellFrame();
    // there are very limited types for params... read the document above.
    uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
    IInspectable params = winrt::box_value(ptr);
    if (item == L"TestPage1") {
        TypeName name{L"App1.TestPage1", TypeKind::Custom};
        frame.Navigate(name, params);
        return;
    }
    if (item == L"Support") {
        TypeName name{L"App1.SupportPage", TypeKind::Custom};
        frame.Navigate(name, params);
        return;
    }
}

void MainWindow::on_back_requested(NavigationView const&, NavigationViewBackRequestedEventArgs const&) {
    spdlog::info("{}: {}", "MainWindow", __func__);
    Frame frame = ShellFrame();
    if (frame.CanGoBack())
        frame.GoBack();
}

} // namespace winrt::App1::implementation
