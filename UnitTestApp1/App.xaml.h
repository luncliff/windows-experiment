#pragma once

#include "App.xaml.g.h"

namespace winrt::UnitTestApp1::implementation
{
    struct App : AppT<App>
    {
        App(PWSTR argv);

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
        winrt::hstring m_args;
    };
}
