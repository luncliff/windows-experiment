#include "pch.h"

#include "ViewModelProvider.h"
#if __has_include("ViewModelProvider.g.cpp")
#include "ViewModelProvider.g.cpp"
#endif

namespace winrt::App1::implementation {

Shared1::BasicViewModel ViewModelProvider::Basic() const noexcept {
    return basic;
}

App1::SettingsViewModel ViewModelProvider::Settings() const noexcept {
    return settings;
}

void ViewModelProvider::Settings(App1::SettingsViewModel viewmodel) noexcept(false) {
    if (viewmodel == nullptr)
        throw winrt::hresult_invalid_argument{L"SettingsViewModel cannot be null"};
    settings = viewmodel;
}

} // namespace winrt::App1::implementation
