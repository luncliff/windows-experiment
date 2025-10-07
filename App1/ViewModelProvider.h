#pragma once
#include "ViewModelProvider.g.h"

#include "SettingsViewModel.g.h"

namespace winrt::App1::implementation {

struct ViewModelProvider : ViewModelProviderT<ViewModelProvider> {
    Shared1::BasicViewModel basic{};
    App1::SettingsViewModel settings{nullptr};

  public:
    ViewModelProvider() noexcept = default;

    Shared1::BasicViewModel Basic() const noexcept;
    App1::SettingsViewModel Settings() const noexcept;
    void Settings(App1::SettingsViewModel) noexcept(false);
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {

struct ViewModelProvider : ViewModelProviderT<ViewModelProvider, implementation::ViewModelProvider> {};

} // namespace winrt::App1::factory_implementation
