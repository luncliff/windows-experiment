#pragma once
#include "SupportPage.g.h"

#include <winrt/Shared1.h> // generated file from Shared1 project

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;

struct SupportPage : SupportPageT<SupportPage> {
  private:
    Shared1::BasicViewModel viewmodel0{nullptr};

  public:
    SupportPage() noexcept = default;

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    Shared1::BasicViewModel ViewModel() noexcept;

  private:
    void setup_viewmodels(IInspectable) noexcept(false);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SupportPage : SupportPageT<SupportPage, implementation::SupportPage> {};
} // namespace winrt::App1::factory_implementation
