#pragma once

#include "TestPage1.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::SizeChangedEventArgs;
using Microsoft::UI::Xaml::Controls::SwapChainPanel;
using Microsoft::UI::Xaml::Input::PointerRoutedEventArgs;
using Microsoft::UI::Xaml::Input::TappedRoutedEventArgs;
using Windows::Foundation::IAsyncAction;
using Windows::Foundation::IInspectable;

struct TestPage1 : TestPage1T<TestPage1> {
    TestPage1();

    void Page_Loaded(IInspectable const&, RoutedEventArgs const&);
    void Page_Unloaded(IInspectable const&, RoutedEventArgs const&);

    void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
