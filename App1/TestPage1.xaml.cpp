/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/windows.ui.xaml.media.dxinterop/nn-windows-ui-xaml-media-dxinterop-iswapchainpanelnative
 */
#include "pch.h"

#include "TestPage1.xaml.h"
#if __has_include("TestPage1.g.cpp")
#include "TestPage1.g.cpp"
#endif

#include <WinPixEventRuntime/pix3.h>

#include "BasicViewModel.h"
#include "StepTimer.h"

namespace winrt::App1::implementation {

TestPage1::TestPage1() noexcept(false) {
    // Initialize page, resources ...
    resources.CreateDeviceResources();

    // Initialize timer0 and set up the event handler
    timer0 = DispatcherTimer();
    timer0.Interval(TimeSpan{std::chrono::seconds{1}}); // 1 second interval
    timer0.Tick({this, &TestPage1::on_timer_tick});
}

App1::BasicViewModel TestPage1::ViewModel() noexcept {
    return viewmodel0;
}

void TestPage1::OnDeviceLost() noexcept {
    // render targets, swapchains will be removed. disconnect
    std::ignore = bridge->SetSwapChain(nullptr);
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

    if (bridge == nullptr) // note the sender is SwapChainPanel
        sender.as(bridge);
    if (IDXGISwapChain* swapchain = resources.GetSwapChain(); swapchain != nullptr)
        bridge->SetSwapChain(swapchain);
}

void TestPage1::OnNavigatedTo(const NavigationEventArgs& e) {
    // we are now visible. connect for the future notifications
    resources.RegisterDeviceNotify(this);

    IInspectable arg0 = e.Parameter();
    if (arg0 == nullptr)
        throw winrt::hresult_invalid_argument(L"TestPage1 requires a ViewModel");

    // Get the ViewModel from the navigation parameter
    viewmodel0 = arg0.try_as<App1::BasicViewModel>();
    if (viewmodel0 == nullptr) {
        StatusTextBlock().Text(L"ViewModel is empty");
        return;
    }
    StatusTextBlock().Text(L"ViewModel loaded");

    // Start the timer when navigating to the page
    timer0.Start();
}

void TestPage1::OnNavigatedFrom(const NavigationEventArgs&) {
    // Stop the timer when navigating away
    timer0.Stop();

    // the page will be destroyed soon. remove the connection
    resources.RegisterDeviceNotify(nullptr);
    OnDeviceLost();
    bridge = nullptr;
}

void TestPage1::on_test_button_click(IInspectable const&, RoutedEventArgs const&) {
    App1::BasicViewModel viewmodel = ViewModel();
    if (viewmodel) {
        auto itemCount = viewmodel.Items().Size();
        auto message = std::format(L"TestPage1 has {} items in the ViewModel.", itemCount);
        StatusTextBlock().Text(message);
    } else {
        StatusTextBlock().Text(L"TestPage1 is working correctly, but no ViewModel available.");
    }
}

// todo: perform rendering on each timer tick
void TestPage1::on_timer_tick(IInspectable const&, IInspectable const&) {
    ID3D12CommandQueue* command_queue = resources.GetCommandQueue();
    PIXScopedEvent(command_queue, PIX_COLOR_DEFAULT, L"on_timer_tick");
    resources.Prepare();
    // ...Rendering logic here...
    {
        PIXBeginEvent(command_queue, PIX_COLOR_DEFAULT, L"Present");
        resources.Present();
        PIXEndEvent(command_queue);
    }
}

} // namespace winrt::App1::implementation
