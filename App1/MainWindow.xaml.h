#pragma once
#include "MainWindow.g.h"

#include "BasicViewModel.g.h"
#include "SettingsViewModel.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Window;
using Microsoft::UI::Xaml::WindowSizeChangedEventArgs;
using Microsoft::UI::Xaml::WindowVisibilityChangedEventArgs;
using Microsoft::UI::Xaml::Controls::Frame;
using Microsoft::UI::Xaml::Controls::NavigationView;
using Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs;
using Microsoft::UI::Xaml::Controls::NavigationViewItem;
using Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs;
using Microsoft::UI::Xaml::Controls::Page;
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
using Windows::Foundation::IAsyncAction;
using Windows::Foundation::IInspectable;

/// @see http://aka.ms/winui-project-info
struct MainWindow : MainWindowT<MainWindow> {
  private:
    App1::BasicViewModel viewmodel0 = nullptr;
    App1::SettingsViewModel viewmodel1 = nullptr;
    winrt::event_token settings_changed_token{};

  public:
    MainWindow() noexcept(false);
    ~MainWindow() noexcept;

    App1::SettingsViewModel Settings() const noexcept;
    void Settings(App1::SettingsViewModel) noexcept(false);

    uint64_t WindowHandle() const noexcept;

    void on_window_size_changed(IInspectable const& sender, WindowSizeChangedEventArgs const& e);
    void on_window_visibility_changed(IInspectable const& sender, WindowVisibilityChangedEventArgs const& e);
    void on_item_invoked(NavigationView const&, NavigationViewItemInvokedEventArgs const&);
    void on_back_requested(NavigationView const&, NavigationViewBackRequestedEventArgs const&);
    void on_settings_changed(IInspectable const&, PropertyChangedEventArgs const&);
    void clear_settings_event() noexcept;
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {

struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {};

} // namespace winrt::App1::factory_implementation
