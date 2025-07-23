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

SettingsPage::SettingsPage() {
    // ...
}

App1::BasicViewModel SettingsPage::ViewModel() noexcept {
    return viewmodel0;
}

fire_and_forget SettingsPage::OnNavigatedTo(const NavigationEventArgs& e) {
    IInspectable arg0 = e.Parameter();
    if (arg0 == nullptr)
        throw std::runtime_error("SettingsPage requires a ViewModel");

    viewmodel0 = arg0.try_as<App1::BasicViewModel>();
    CreateLogFolderAsync();
}

fire_and_forget SettingsPage::CreateLogFolderAsync() {
    using Microsoft::UI::Dispatching::DispatcherQueue;
    auto foreground = DispatcherQueue::GetForCurrentThread();
    co_await viewmodel0.CreateLogFolderAsync();
    co_await resume_on_ui{foreground};
    UpdateLogFolderPath();
}

void SettingsPage::UpdateLogFolderPath() {
    // ...
    LogPathTextBlock().Text(viewmodel0.GetLogFolderPath());
}

void SettingsPage::OnNavigatedFrom(const NavigationEventArgs&) {
    viewmodel0 = nullptr; // leaving. no more ViewModel access
}

void SettingsPage::on_open_log_folder_click(IInspectable const&, RoutedEventArgs const&) {
    try {
        App1::BasicViewModel viewmodel = ViewModel();
        StorageFolder folder = viewmodel.GetLogFolder();
        if (folder != nullptr)
            Launcher::LaunchFolderAsync(folder);
    } catch (const winrt::hresult_error& ex) {
        spdlog::error(L"{}", static_cast<std::wstring_view>(ex.message()));
    }
}

} // namespace winrt::App1::implementation
