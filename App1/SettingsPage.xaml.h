#pragma once

#include "SettingsPage.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;

struct SettingsPage : SettingsPageT<SettingsPage> {
    SettingsPage();

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage> {};
} // namespace winrt::App1::factory_implementation
