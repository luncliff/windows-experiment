#include "pch.h"

#include "SupportPage.xaml.h"
#if __has_include("SupportPage.g.cpp")
#include "SupportPage.g.cpp"
#endif

namespace winrt::App1::implementation {

SupportPage::SupportPage() {
    // Initialize support page
}

App1::BasicViewModel SupportPage::ViewModel() noexcept {
    return viewmodel0;
}

void SupportPage::OnNavigatedTo(const NavigationEventArgs& e) {
    if (e == nullptr)
        return;

    // Get the ViewModel from the navigation parameter
    if (e.Parameter() != nullptr) {
        viewmodel0 = e.Parameter().try_as<App1::BasicViewModel>();
        // ViewModel is available for future use in the support page
    }
}

void SupportPage::OnNavigatedFrom(const NavigationEventArgs&) {
    // Handle leaving the page via navigation
}

} // namespace winrt::App1::implementation
