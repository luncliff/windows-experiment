/// @see http://aka.ms/winui-project-info
#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include <CppUnitTest.h>
#include <winrt/Microsoft.VisualStudio.TestPlatform.TestExecutor.WinRTCore.h>

using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::VisualStudio::TestPlatform::TestExecutor::WinRTCore;

namespace winrt::UnitTestApp1::implementation {

App::App(PWSTR argv) {
    m_args = argv;

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e) {
        if (IsDebuggerPresent()) {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif
}

void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e) {
    UnitTestClient::CreateDefaultUI();
    window = make<MainWindow>();
    window.Activate();
    UnitTestClient::Run(m_args);
}

} // namespace winrt::UnitTestApp1::implementation

#ifdef DISABLE_XAML_GENERATED_MAIN
using winrt::Microsoft::UI::Xaml::Application;

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR argv, int) {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    Application::Start([argv](auto&&) { ::winrt::make<::winrt::UnitTestApp1::implementation::App>(argv); });
    return 0;
}

#endif