#include "pch.h"

#include "SupportPage.xaml.h"
#if __has_include("SupportPage.g.cpp")
#include "SupportPage.g.cpp"
#endif
#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {
using namespace Microsoft::UI::Xaml;

SupportPage::SupportPage() {
    // ...
}

void SupportPage::myButton_Click(IInspectable const&, RoutedEventArgs const&) {
    myButton().Content(box_value(L"Support!"));
}

} // namespace winrt::App1::implementation
