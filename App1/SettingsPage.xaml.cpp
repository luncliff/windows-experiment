#include "pch.h"

#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "ViewModelProvider.g.h"

namespace winrt::App1::implementation {

App1::SettingsViewModel SettingsPage::ViewModel() noexcept {
    return viewmodel0;
}

void SettingsPage::setup_viewmodels(IInspectable arg0) noexcept(false) {
    if (arg0 == nullptr)
        throw winrt::hresult_invalid_argument{L"Navigation parameter is null"};
    auto provider = arg0.try_as<App1::ViewModelProvider>();
    if (provider == nullptr)
        throw winrt::hresult_invalid_argument{L"Navigation parameter is not a ViewModelProvider"};
    if ((viewmodel0 = provider.Settings()) == nullptr)
        throw winrt::hresult_invalid_argument{L"ViewModel is null"};
}

void SettingsPage::OnNavigatedTo(const NavigationEventArgs& e) {
    // Get the ViewModel from the navigation parameter. Ensure ViewModel is always valid
    setup_viewmodels(e.Parameter());

    // Connect to property changed events
    settings_changed_token = ViewModel().PropertyChanged({this, &SettingsPage::on_settings_property_changed});
    UpdateCounterDisplay();
}

void SettingsPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // Disconnect from property changed events
    if (settings_changed_token) {
        viewmodel0.PropertyChanged(settings_changed_token);
        settings_changed_token = {};
    }
    viewmodel0 = nullptr;
}

void SettingsPage::UpdateCounterDisplay() {
    CounterTextBlock().Text(winrt::to_hstring(ViewModel().Counter()));
}

void SettingsPage::on_settings_property_changed(IInspectable const&, PropertyChangedEventArgs const& e) {
    if (e.PropertyName() == L"Counter") {
        UpdateCounterDisplay();
    }
}

void SettingsPage::on_increment_counter_click(IInspectable const&, RoutedEventArgs const&) {
    ViewModel().Counter(ViewModel().Counter() + 1);
}

void SettingsPage::on_decrement_counter_click(IInspectable const&, RoutedEventArgs const&) {
    uint32_t value = ViewModel().Counter();
    if (value > 0)
        ViewModel().Counter(value - 1);
}

void SettingsPage::on_reset_counter_click(IInspectable const&, RoutedEventArgs const&) {
    ViewModel().ResetToDefault();
}

} // namespace winrt::App1::implementation
