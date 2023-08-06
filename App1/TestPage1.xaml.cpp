#include "pch.h"

#include "TestPage1.xaml.h"
#if __has_include("TestPage1.g.cpp")
#include "TestPage1.g.cpp"
#endif

#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {
using namespace Microsoft::UI::Xaml;

TestPage1::TestPage1() {
    InitializeComponent();
}

int32_t TestPage1::MyProperty() {
    throw hresult_not_implemented();
}

void TestPage1::MyProperty(int32_t /* value */) {
    throw hresult_not_implemented();
}

void TestPage1::myButton_Click(IInspectable const&, RoutedEventArgs const&) {
    myButton().Content(box_value(L"Clicked"));
}

} // namespace winrt::App1::implementation
