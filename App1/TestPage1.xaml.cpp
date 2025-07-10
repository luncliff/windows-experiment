/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/windows.ui.xaml.media.dxinterop/nn-windows-ui-xaml-media-dxinterop-iswapchainpanelnative
 */
#include "pch.h"

#include "TestPage1.xaml.h"
#if __has_include("TestPage1.g.cpp")
#include "TestPage1.g.cpp"
#endif

#include "BasicViewModel.h"
#include "StepTimer.h"

namespace winrt::App1::implementation {

TestPage1::TestPage1() noexcept(false) {
    // Initialize page, resources ...
    resources.CreateDeviceResources();
}

App1::BasicViewModel TestPage1::ViewModel() noexcept {
    return viewmodel0;
}

void TestPage1::OnDeviceLost() noexcept {
    // render targets, swapchains will be removed. disconnect
    bridge->SetSwapChain(nullptr);
}

void TestPage1::OnDeviceRestored() noexcept {
    // ... resources.CreateDeviceResources() is already done ...
    // CreateWindowSizeDependentResources ...
    if (IDXGISwapChain* swapchain = resources.GetSwapChain(); swapchain != nullptr)
        bridge->SetSwapChain(swapchain);
}

void TestPage1::panel0_size_changed(IInspectable const& sender, SizeChangedEventArgs const& e) {
    // get the new size and update the resources
    auto size = e.NewSize();
    resources.CreateWindowSizeDependentResources(static_cast<UINT>(size.Width), static_cast<UINT>(size.Height));
    // note the sender is SwapChainPanel
    sender.as(bridge);
    if (IDXGISwapChain* swapchain = resources.GetSwapChain(); swapchain != nullptr)
        bridge->SetSwapChain(swapchain);
}

void TestPage1::OnNavigatedTo(const NavigationEventArgs& e) {
    // we are now visible. connect for the future notifications
    resources.RegisterDeviceNotify(this);

    // no parameter means no ViewModel is forwarded
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
    // the page will be destroyed soon. remove the connection
    OnDeviceLost();
    resources.RegisterDeviceNotify(nullptr);
}

void TestPage1::on_test_button_click(IInspectable const&, RoutedEventArgs const&) {
    // Update the status text when button is clicked
    App1::BasicViewModel viewmodel = ViewModel();
    if (viewmodel) {
        auto itemCount = viewmodel.Items().Size();
        auto message = std::format(L"Button clicked! TestPage1 has {} items in the ViewModel.", itemCount);
        StatusTextBlock().Text(message);
    } else {
        StatusTextBlock().Text(L"Button clicked! TestPage1 is working correctly, but no ViewModel available.");
    }
    // todo: add PIX capture
    // todo: add basic rendering logic
    resources.Prepare();
    // ...
    resources.Present();
}

} // namespace winrt::App1::implementation
