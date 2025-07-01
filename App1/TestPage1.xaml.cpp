#include "pch.h"

#include "TestPage1.xaml.h"
#if __has_include("TestPage1.g.cpp")
#include "TestPage1.g.cpp"
#endif

#include "BasicViewModel.h"

namespace winrt::App1::implementation {

TestPage1::TestPage1() {
    // Initialize page
}

App1::BasicViewModel TestPage1::ViewModel() noexcept {
    return viewmodel0;
}

void TestPage1::OnNavigatedTo(const NavigationEventArgs& e) {
    if (e == nullptr)
        return;

    // Get the ViewModel from the navigation parameter
    if (e.Parameter() != nullptr) {
        viewmodel0 = e.Parameter().try_as<App1::BasicViewModel>();
        if (viewmodel0 == nullptr)
            return;
        // Update status with the number of items in the ViewModel
        auto itemCount = viewmodel0.Items().Size();
        StatusTextBlock().Text(winrt::hstring(L"ViewModel loaded with ") + winrt::to_hstring(itemCount) +
                               L" items. Click the button to test functionality.");
    }
}

void TestPage1::OnNavigatedFrom(const NavigationEventArgs&) {
    // Handle leaving the page via navigation
}

void TestPage1::OnTestButtonClick(winrt::Windows::Foundation::IInspectable const&, RoutedEventArgs const&) {
    // Update the status text when button is clicked
    if (viewmodel0) {
        auto itemCount = viewmodel0.Items().Size();
        StatusTextBlock().Text(winrt::hstring(L"Button clicked! TestPage1 is working correctly with ") +
                               winrt::to_hstring(itemCount) + L" items in the ViewModel.");
    } else {
        StatusTextBlock().Text(L"Button clicked! TestPage1 is working correctly, but no ViewModel available.");
    }
}

} // namespace winrt::App1::implementation
