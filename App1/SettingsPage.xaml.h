#pragma once

#include "SettingsPage.g.h"
#include "SettingsViewModel.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Data::PropertyChangedEventHandler;
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;
using Windows::Foundation::IInspectable;

struct SettingsPage : SettingsPageT<SettingsPage> {
  private:
    App1::SettingsViewModel viewmodel0{nullptr};
    winrt::event_token property_changed_token{};

    void UpdateCounterDisplay();
    void on_settings_property_changed(IInspectable const& sender, PropertyChangedEventArgs const& e);

  public:
    SettingsPage() noexcept = default;

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    App1::SettingsViewModel ViewModel() noexcept;
    
    void on_increment_counter_click(IInspectable const&, RoutedEventArgs const&);
    void on_decrement_counter_click(IInspectable const&, RoutedEventArgs const&);
    void on_reset_counter_click(IInspectable const&, RoutedEventArgs const&);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage> {};
} // namespace winrt::App1::factory_implementation
