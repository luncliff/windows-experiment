#pragma once
#include "MainWindow.g.h"

namespace winrt::App1::implementation {

using Microsoft::UI::Xaml::RoutedEventArgs;
using Windows::Foundation::IInspectable;

/// @see http://aka.ms/winui-project-info
struct MainWindow : MainWindowT<MainWindow> {
    MainWindow();

    int32_t MyProperty();
    void MyProperty(int32_t value);

    void myButton_Click(IInspectable const& sender, RoutedEventArgs const& args);
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {

struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {};

} // namespace winrt::App1::factory_implementation
