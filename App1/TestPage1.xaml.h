#pragma once

#include "BasicViewModel.g.h"
#include "TestPage1.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;

struct TestPage1 : TestPage1T<TestPage1> {
  private:
    App1::BasicViewModel viewmodel0{nullptr};

  public:
    TestPage1();

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);
    void OnTestButtonClick(winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& e);

    App1::BasicViewModel ViewModel() noexcept;
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
