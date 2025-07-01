#pragma once

#include "BasicViewModel.g.h"
#include "SupportPage.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;

struct SupportPage : SupportPageT<SupportPage> {
  private:
    App1::BasicViewModel viewmodel0{nullptr};

  public:
    SupportPage();

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    App1::BasicViewModel ViewModel() noexcept;
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SupportPage : SupportPageT<SupportPage, implementation::SupportPage> {};
} // namespace winrt::App1::factory_implementation
