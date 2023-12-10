#include "pch.h"

#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "MainWindow.xaml.h"

namespace winrt::App1::implementation {
using namespace Microsoft::UI::Xaml;

SettingsPage::SettingsPage() {
    // ...
}

void SettingsPage::OnNavigatedTo(const NavigationEventArgs& e) {
    auto ptr = winrt::unbox_value<uintptr_t>(e.Parameter());
    auto window = reinterpret_cast<implementation::MainWindow*>(ptr);
    if (window == nullptr)
        return;
    HWND hwnd = reinterpret_cast<HWND>(window->WindowHandle());
    if (hwnd == nullptr)
        return;
}

void SettingsPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // ... leaving the page via navigation ...
}

} // namespace winrt::App1::implementation
