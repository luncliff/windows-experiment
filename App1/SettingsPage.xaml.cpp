#include "pch.h"

#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

namespace winrt::App1::implementation {

SettingsPage::SettingsPage() {
    // ...
}

void SettingsPage::OnNavigatedTo(const NavigationEventArgs& e) {
    if (e == nullptr)
        return;
}

void SettingsPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // ... leaving the page via navigation ...
}

} // namespace winrt::App1::implementation
