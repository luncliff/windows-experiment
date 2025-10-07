#include "pch.h"

#include "SupportPage.xaml.h"
#if __has_include("SupportPage.g.cpp")
#include "SupportPage.g.cpp"
#endif
#include "ViewModelProvider.g.h"

namespace winrt::App1::implementation {

void SupportPage::setup_viewmodels(IInspectable arg0) noexcept(false) {
    if (arg0 == nullptr)
        throw winrt::hresult_invalid_argument{L"Navigation parameter is null"};
    auto provider = arg0.try_as<App1::ViewModelProvider>();
    if (provider == nullptr)
        throw winrt::hresult_invalid_argument{L"Navigation parameter is not a ViewModelProvider"};
    if ((viewmodel0 = provider.Basic()) == nullptr)
        throw winrt::hresult_invalid_argument{L"ViewModel is null"};
}

Shared1::BasicViewModel SupportPage::ViewModel() noexcept {
    return viewmodel0;
}

void SupportPage::OnNavigatedTo(const NavigationEventArgs& e) {
    // Get the ViewModel from the navigation parameter. Ensure ViewModel is always valid
    setup_viewmodels(e.Parameter());
}

void SupportPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // Handle leaving the page via navigation
    viewmodel0 = nullptr;
}

} // namespace winrt::App1::implementation
