#pragma once
#include "App.xaml.g.h"
#include "ViewModelProvider.g.h"

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::LaunchActivatedEventArgs;
using Microsoft::UI::Xaml::UnhandledExceptionEventArgs;
using Microsoft::UI::Xaml::Window;
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
using Windows::Foundation::IInspectable;

struct App : AppT<App> {
  private:
    Window window = nullptr;
    App1::ViewModelProvider provider{};
    winrt::event_token settings_changed_token{};

  public:
    App() noexcept(false);
    ~App() noexcept;

    void OnLaunched(LaunchActivatedEventArgs const&);
    void OnUnhandledException(IInspectable const&, UnhandledExceptionEventArgs const&);

    void on_settings_changed(IInspectable const&, PropertyChangedEventArgs const&);
    void clear_settings_event() noexcept;
};

} // namespace winrt::App1::implementation
