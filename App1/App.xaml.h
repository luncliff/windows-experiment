#pragma once
#include "App.xaml.g.h"

namespace winrt::App1::implementation {

using Microsoft::UI::Xaml::LaunchActivatedEventArgs;
using Microsoft::UI::Xaml::UnhandledExceptionEventArgs;
using Microsoft::UI::Xaml::Window;
using Windows::Foundation::IInspectable;

struct App : AppT<App> {
    App();

    void OnLaunched(LaunchActivatedEventArgs const&);

    void OnUnhandledException(IInspectable const&, UnhandledExceptionEventArgs const&);

  private:
    Window window = nullptr;
};

} // namespace winrt::App1::implementation
