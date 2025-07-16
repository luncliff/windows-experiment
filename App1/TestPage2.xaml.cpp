/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/windows.ui.xaml.media.dxinterop/nn-windows-ui-xaml-media-dxinterop-iswapchainpanelnative
 */
#include "pch.h"

#include "TestPage2.xaml.h"
#if __has_include("TestPage2.g.cpp")
#include "TestPage2.g.cpp"
#endif

#include "BasicViewModel.h"

namespace winrt::App1::implementation {

TestPage2::TestPage2() noexcept(false) {
    InitializeComponent();
}

App1::BasicViewModel TestPage2::ViewModel() noexcept {
    return viewmodel0;
}

void TestPage2::OnNavigatedTo(const NavigationEventArgs& e) {
    IInspectable arg0 = e.Parameter();
    if (arg0 == nullptr)
        throw winrt::hresult_invalid_argument(L"TestPage2 requires a ViewModel");
    // Get the ViewModel from the navigation parameter
    viewmodel0 = arg0.try_as<App1::BasicViewModel>();

    UpdateStatusText(L"Page loaded - resize to see dynamic layout");
}

void TestPage2::OnNavigatedFrom(const NavigationEventArgs&) {
    // ...
}

winrt::event_token TestPage2::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler) {
    return m_propertyChanged.add(handler);
}

void TestPage2::PropertyChanged(winrt::event_token const& token) noexcept {
    m_propertyChanged.remove(token);
}

void TestPage2::RaisePropertyChanged(winrt::hstring const& propertyName) {
    m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{propertyName});
}

void TestPage2::UpdateBoxHeight(double height) {
    if (m_boxHeight != height) {
        m_boxHeight = height;
        RaisePropertyChanged(L"BoxHeight");
    }
}

void TestPage2::UpdateStatusText(winrt::hstring const& text) {
    if (m_statusText != text) {
        m_statusText = text;
        RaisePropertyChanged(L"StatusText");
    }
}

void TestPage2::UpdateCurrentAspectRatio(winrt::hstring const& ratio) {
    if (m_currentAspectRatio != ratio) {
        m_currentAspectRatio = ratio;
        RaisePropertyChanged(L"CurrentAspectRatio");
    }
}

double TestPage2::BoxHeight() const noexcept {
    return m_boxHeight;
}
winrt::hstring TestPage2::StatusText() const noexcept {
    return m_statusText;
}
winrt::hstring TestPage2::CurrentAspectRatio() const noexcept {
    return m_currentAspectRatio;
}

void TestPage2::panel0_size_changed(IInspectable const& sender, SizeChangedEventArgs const& e) {
    // Aspect ratio constants
    constexpr double ASPECT_16_9 = 16.0 / 9.0; // 1.78
    constexpr double ASPECT_4_3 = 4.0 / 3.0;   // 1.33

    auto size = e.NewSize();
    double panelWidth = size.Width;
    double panelHeight = size.Height;

    // Get the current width of the blue box
    double boxWidth = panelWidth;
    double boxHeight = 0;
    winrt::hstring aspectRatioName;

    // Simplified logic: Use only 2 aspect ratios based on width
    if (panelWidth < 800) {
        // If width is smaller than 800, use 16:9 aspect ratio
        boxHeight = boxWidth / ASPECT_16_9;
        aspectRatioName = L"16:9";
    } else {
        // For all other cases, use 4:3 aspect ratio
        boxHeight = boxWidth / ASPECT_4_3;
        aspectRatioName = L"4:3";
    }

    // Update the dimensions and status using databinding properties
    UpdateBoxHeight(boxHeight);
    UpdateCurrentAspectRatio(aspectRatioName);

    // Update status text with current dimensions
    auto text = std::format(L"Page: {:.0f}x{:.0f}, Box: {:.0f}x{:.0f} - {}", //
                            panelWidth, panelHeight, boxWidth, boxHeight, aspectRatioName);
    UpdateStatusText(winrt::hstring{text});
}

} // namespace winrt::App1::implementation
