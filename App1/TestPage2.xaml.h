#pragma once

#include "BasicViewModel.g.h"
#include "TestPage2.g.h"

#include <microsoft.ui.xaml.media.dxinterop.h> // ISwapChainPanelNative for Microsoft namespace

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::DispatcherTimer;
using Microsoft::UI::Xaml::RoutedEventArgs;
using Microsoft::UI::Xaml::Controls::SwapChainPanel;
using Microsoft::UI::Xaml::Navigation::NavigationEventArgs;
using winrt::Microsoft::UI::Xaml::SizeChangedEventArgs;
using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::Foundation::TimeSpan;

struct TestPage2 : TestPage2T<TestPage2> {
  private:
    App1::BasicViewModel viewmodel0{nullptr};

    // Properties for databinding
    double m_boxHeight{450.0};
    winrt::hstring m_statusText{L"Resize the window to see layout changes"};
    winrt::hstring m_currentAspectRatio{L"16:9"};

    // Property change notification
    winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

  public:
    TestPage2() noexcept(false);

    void OnNavigatedTo(const NavigationEventArgs&);
    void OnNavigatedFrom(const NavigationEventArgs&);

    void panel0_size_changed(IInspectable const&, SizeChangedEventArgs const&);

    App1::BasicViewModel ViewModel() noexcept;

    // Properties for databinding
    double BoxHeight() const noexcept;
    winrt::hstring StatusText() const noexcept;
    winrt::hstring CurrentAspectRatio() const noexcept;

    // PropertyChanged event
    winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    void RaisePropertyChanged(winrt::hstring const& propertyName);
    void UpdateBoxHeight(double height);
    void UpdateStatusText(winrt::hstring const& text);
    void UpdateCurrentAspectRatio(winrt::hstring const& ratio);
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct TestPage2 : TestPage2T<TestPage2, implementation::TestPage2> {};
} // namespace winrt::App1::factory_implementation
