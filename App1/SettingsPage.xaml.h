#pragma once

#include "BasicViewModel.h"
#include "SettingsPage.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;
using Windows::Foundation::IInspectable;

struct SettingsPage : SettingsPageT<SettingsPage> {
  private:
    App1::BasicViewModel viewmodel0{nullptr};

    void UpdateLogFolderPath();

  public:
    SettingsPage();

    fire_and_forget OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    fire_and_forget CreateLogFolderAsync();
    App1::BasicViewModel ViewModel() noexcept;

    void on_open_log_folder_click(IInspectable const&, RoutedEventArgs const&);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage> {};
} // namespace winrt::App1::factory_implementation
