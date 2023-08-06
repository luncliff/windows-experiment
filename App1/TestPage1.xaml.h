#pragma once
#include "TestPage1.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Window;
using Windows::Foundation::IInspectable;

struct TestPage1 : TestPage1T<TestPage1> {
    TestPage1();

    int32_t MyProperty();
    void MyProperty(int32_t value);

    void myButton_Click(IInspectable const&, RoutedEventArgs const&);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage1 : TestPage1T<TestPage1, implementation::TestPage1> {};
} // namespace winrt::App1::factory_implementation
