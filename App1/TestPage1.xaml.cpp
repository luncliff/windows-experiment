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
    viewmodel0 = winrt::make<App1::implementation::RepositoryViewModel>();
    //...
}

/// @see after OnNavigatedTo, prepare some resources
void TestPage1::Page_Loaded(IInspectable const&, RoutedEventArgs const&) {
    spdlog::info("{}: {}", "TestPage1", __func__);
}

void TestPage1::Page_Unloaded(IInspectable const&, RoutedEventArgs const&) {
    spdlog::info("{}: {}", "TestPage1", __func__);
}

App1::RepositoryViewModel TestPage1::ViewModel0() {
    return viewmodel0;
}

fire_and_forget TestPage1::on_button_clicked(IInspectable const& s, RoutedEventArgs const&) {
    if (auto button = s.try_as<Button>(); button != OpenButton())
        spdlog::debug("{}: {}", "TestPage1", "sender is OpenButton");
    if (selected == nullptr)
        co_return;
    auto launched = co_await Windows::System::Launcher::LaunchUriAsync(selected.ProjectUri());
    if (launched == false)
        spdlog::error("{}: {}", "TestPage1", "launch failed");
}

void TestPage1::on_selection_changed(IInspectable const& s, SelectionChangedEventArgs const&) {
    auto view = s.try_as<Microsoft::UI::Xaml::Controls::ListView>();
    if (view == nullptr)
        return;
    auto item = view.SelectedItem();
    selected = item.try_as<RepositoryItem>();
    spdlog::info("{}: selected {}", "TestPage1", winrt::to_string(selected.Name()));
}
} // namespace winrt::App1::implementation
