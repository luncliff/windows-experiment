#include "pch.h"

#include "SettingsViewModel.h"
#if __has_include("SettingsViewModel.g.cpp")
#include "SettingsViewModel.g.cpp"
#endif

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Microsoft::UI::Xaml::Data;

namespace winrt::App1::implementation {

SettingsViewModel::SettingsViewModel() {
    LoadSettings();
}

ApplicationDataContainer SettingsViewModel::GetLocalSettings() noexcept(false) {
    return ApplicationData::Current().LocalSettings();
}

uint32_t SettingsViewModel::Counter() const noexcept {
    return counter;
}

void SettingsViewModel::Counter(uint32_t value) noexcept {
    if (counter != value) {
        counter = value;
        SaveSettings();
        RaisePropertyChanged(L"Counter");
    }
}

void SettingsViewModel::ResetToDefault() noexcept {
    Counter(0);
    SaveSettings();
}

void SettingsViewModel::SaveSettings() noexcept {
    try {
        auto settings = GetLocalSettings();
        if (settings == nullptr) {
            spdlog::error(L"Local settings are not available.");
            return;
        }
        IPropertySet values = settings.Values();
        values.Insert(L"Counter", winrt::box_value(counter));
    } catch (const winrt::hresult_error& ex) {
        spdlog::error(L"SaveSettings: {}", ex.message());
    }
}

void SettingsViewModel::LoadSettings() noexcept {
    counter = 0; // Initialize to default value
    try {
        auto settings = GetLocalSettings();
        if (settings == nullptr) {
            spdlog::error(L"Local settings are not available.");
            return;
        }
        IPropertySet values = settings.Values();
        if (const auto key = L"Counter"; values.HasKey(key)) {
            if (auto boxed = values.Lookup(key); boxed != nullptr)
                counter = winrt::unbox_value<uint32_t>(boxed);
        }
    } catch (const winrt::hresult_error& ex) {
        spdlog::error(L"LoadSettings: {}", ex.message());
    }
}

winrt::event_token SettingsViewModel::PropertyChanged(PropertyChangedEventHandler const& handler) {
    return m_propertyChanged.add(handler);
}

void SettingsViewModel::PropertyChanged(winrt::event_token const& token) noexcept {
    m_propertyChanged.remove(token);
}

void SettingsViewModel::RaisePropertyChanged(winrt::hstring const& propertyName) {
    m_propertyChanged(*this, PropertyChangedEventArgs(propertyName));
}

} // namespace winrt::App1::implementation
