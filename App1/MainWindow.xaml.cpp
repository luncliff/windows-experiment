#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

namespace winrt::App1::implementation {

using Microsoft::UI::Xaml::Controls::Button;

MainWindow::MainWindow() {
    InitializeComponent();
}

int32_t MainWindow::MyProperty() {
    throw winrt::hresult_not_implemented();
}

void MainWindow::MyProperty(int32_t) {
    throw winrt::hresult_not_implemented();
}

void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&) {
    Button button = myButton();
    IInspectable text = winrt::box_value(L"Clicked");
    button.Content(text);
}

} // namespace winrt::App1::implementation
