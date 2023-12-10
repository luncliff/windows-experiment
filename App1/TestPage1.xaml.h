#pragma once

#include "TestPage1.g.h"

#include "RepositoryViewModel.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Controls::Button;
using Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs;
using Microsoft::UI::Xaml::Input::PointerRoutedEventArgs;
using Microsoft::UI::Xaml::Input::TappedRoutedEventArgs;
using Windows::Foundation::IAsyncAction;
using Windows::Foundation::IInspectable;

struct TestPage1 : TestPage1T<TestPage1> {
  private:
    App1::RepositoryViewModel viewmodel0 = nullptr;
    App1::RepositoryItem selected = nullptr;

  public:
    TestPage1();

    void Page_Loaded(IInspectable const&, RoutedEventArgs const&);
    void Page_Unloaded(IInspectable const&, RoutedEventArgs const&);

    App1::RepositoryViewModel ViewModel0();

    fire_and_forget on_button_clicked(IInspectable const&, RoutedEventArgs const&);
    void on_selection_changed(IInspectable const&, SelectionChangedEventArgs const&);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
