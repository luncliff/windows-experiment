#include "pch.h"

#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

using Windows::Foundation::Uri;
using Windows::System::Launcher;
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;

App1::SettingsViewModel SettingsPage::ViewModel() noexcept {
    return viewmodel0;
}

void SettingsPage::OnNavigatedTo(const NavigationEventArgs& e) {
    IInspectable arg0 = e.Parameter();
    if (arg0 == nullptr)
        throw std::runtime_error("SettingsPage requires a ViewModel");
    viewmodel0 = arg0.try_as<App1::SettingsViewModel>();

    // Connect to property changed events
    if (viewmodel0 != nullptr) {
        property_changed_token = viewmodel0.PropertyChanged({this, &SettingsPage::on_settings_property_changed});
        UpdateCounterDisplay();
    }
}

void SettingsPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // Disconnect from property changed events
    if (viewmodel0 != nullptr && property_changed_token) {
        viewmodel0.PropertyChanged(property_changed_token);
        property_changed_token = {};
    }
    viewmodel0 = nullptr;
}

void SettingsPage::UpdateCounterDisplay() {
    if (viewmodel0 != nullptr) {
        CounterTextBlock().Text(winrt::to_hstring(viewmodel0.Counter()));
    }
}

void SettingsPage::on_settings_property_changed(IInspectable const&, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs const& e) {
    if (e.PropertyName() == L"Counter") {
        UpdateCounterDisplay();
    }
}

void SettingsPage::on_increment_counter_click(IInspectable const&, RoutedEventArgs const&) {
    if (viewmodel0 != nullptr) {
        viewmodel0.Counter(viewmodel0.Counter() + 1);
    }
}

void SettingsPage::on_decrement_counter_click(IInspectable const&, RoutedEventArgs const&) {
    if (viewmodel0 != nullptr) {
        uint32_t currentValue = viewmodel0.Counter();
        if (currentValue > 0) {
            viewmodel0.Counter(currentValue - 1);
        }
    }
}

void SettingsPage::on_reset_counter_click(IInspectable const&, RoutedEventArgs const&) {
    if (viewmodel0 != nullptr) {
        viewmodel0.ResetToDefault();
    }
}

} // namespace winrt::App1::implementation
