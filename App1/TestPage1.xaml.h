#pragma once

#include "BasicViewModel.g.h"
#include "DeviceResources.h"
#include "TestPage1.g.h"

#include <microsoft.ui.xaml.media.dxinterop.h> // ISwapChainPanelNative for Microsoft namespace

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;
using Microsoft::UI::Xaml::DispatcherTimer;
using winrt::Microsoft::UI::Xaml::SizeChangedEventArgs;
using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::Foundation::TimeSpan;

struct TestPage1 : TestPage1T<TestPage1>, DX::IDeviceNotify {
  private:
    DX::DeviceResources resources{DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, 2};
    winrt::com_ptr<ISwapChainPanelNative> bridge = nullptr;
    App1::BasicViewModel viewmodel0{nullptr};
    DispatcherTimer timer0{};

  public:
    TestPage1() noexcept(false);

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    void on_test_button_click(IInspectable const&, RoutedEventArgs const&);
    void panel0_size_changed(IInspectable const&, SizeChangedEventArgs const&);
    void on_timer_tick(IInspectable const&, IInspectable const&);

    App1::BasicViewModel ViewModel() noexcept;

    void OnDeviceLost() noexcept override;
    void OnDeviceRestored() noexcept override;
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
