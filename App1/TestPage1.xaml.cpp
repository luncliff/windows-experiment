#include "pch.h"

#include "TestPage1.xaml.h"
#if __has_include("TestPage1.g.cpp")
#include "TestPage1.g.cpp"
#endif
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {
using namespace Microsoft::UI::Xaml;

TestPage1::TestPage1() {
    //...
}

/// @see after OnNavigatedTo, prepare some resources
void TestPage1::Page_Loaded(IInspectable const&, RoutedEventArgs const&) {
    spdlog::info("{}: {}", "TestPage1", __func__);
}

void TestPage1::Page_Unloaded(IInspectable const&, RoutedEventArgs const&) {
    spdlog::info("{}: {}", "TestPage1", __func__);
}

void TestPage1::myButton_Click(IInspectable const&, RoutedEventArgs const&) {
    myButton().Content(box_value(L"Clicked"));
}
} // namespace winrt::App1::implementation
