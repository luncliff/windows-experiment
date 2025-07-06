#pragma once

#include "BasicViewModel.g.h"
#include "DeviceResources.h"
#include "TestPage1.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;
using winrt::Microsoft::UI::Xaml::SizeChangedEventArgs;
using winrt::Windows::Foundation::IInspectable;

struct TestPage1 : TestPage1T<TestPage1> {
  private:
    DX::DeviceResources resources{DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, 2};
    App1::BasicViewModel viewmodel0{nullptr};

  public:
    TestPage1();

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    void OnTestButtonClick(IInspectable const&, RoutedEventArgs const&);
    void SwapChainPanel0_SizeChanged(IInspectable const&, SizeChangedEventArgs const&);

    App1::BasicViewModel ViewModel() noexcept;
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
