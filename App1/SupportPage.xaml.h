#pragma once

#include "SupportPage.g.h"

namespace winrt::App1::implementation {

struct SupportPage : SupportPageT<SupportPage> {
    SupportPage();

    void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct SupportPage : SupportPageT<SupportPage, implementation::SupportPage> {};
} // namespace winrt::App1::factory_implementation
