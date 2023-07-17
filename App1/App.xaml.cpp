#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

namespace winrt::App1::implementation {

using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;

App::App() {
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
        if (IsDebuggerPresent() == false)
            // ...
            return;

        auto message = e.Message();
        __debugbreak();
    });
#endif
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
    window = make<MainWindow>();
    window.Activate();
}

} // namespace winrt::App1::implementation