#pragma once

#include "SettingsViewModel.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Data::PropertyChangedEventHandler;
using Windows::Foundation::IAsyncAction;
using Windows::Foundation::Collections::IObservableVector;
using Windows::Foundation::Collections::IPropertySet;
using Windows::Storage::ApplicationData;
using Windows::Storage::ApplicationDataContainer;
using Windows::Storage::StorageFolder;

/**
 * @details Helps SettingsPage to work with the application settings
 * @note The class won't access User or Machine settings
 * @see https://learn.microsoft.com/en-us/windows/apps/design/app-settings/guidelines-for-app-settings
 * @see https://learn.microsoft.com/en-us/windows/apps/design/app-settings/store-and-retrieve-app-data
 * @see Windows.Storage.ApplicationDataContainer
 * @see Windows.Storage.StorageFolder
 */
struct SettingsViewModel : SettingsViewModelT<SettingsViewModel> {
  private:
    uint32_t counter = 0;

    winrt::event<PropertyChangedEventHandler> m_propertyChanged;

    void SaveSettings() noexcept;
    void LoadSettings() noexcept;
    ApplicationDataContainer GetLocalSettings() noexcept(false);
    void RaisePropertyChanged(winrt::hstring const& propertyName);

  public:
    SettingsViewModel();

    uint32_t Counter() const noexcept;
    void Counter(uint32_t value) noexcept;

    void ResetToDefault() noexcept;

    // INotifyPropertyChanged implementation
    winrt::event_token PropertyChanged(PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {

struct SettingsViewModel : SettingsViewModelT<SettingsViewModel, implementation::SettingsViewModel> {};

} // namespace winrt::App1::factory_implementation
