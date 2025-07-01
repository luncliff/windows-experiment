#include "pch.h"

#include "BasicViewModel.h"
#if __has_include("BasicViewModel.g.cpp")
#include "BasicViewModel.g.cpp"
#endif
#include "BasicItem.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

namespace winrt::App1::implementation {

BasicViewModel::BasicViewModel() {
    m_items = winrt::single_threaded_observable_vector<App1::BasicItem>();
    InitializeItems();
}

IObservableVector<App1::BasicItem> BasicViewModel::Items() {
    return m_items;
}

void BasicViewModel::InitializeItems() {
    // Add some sample items
    m_items.Append(
        winrt::make<App1::implementation::BasicItem>(L"Microsoft Learn", Uri(L"https://learn.microsoft.com/")));

    m_items.Append(winrt::make<App1::implementation::BasicItem>(
        L"Windows App SDK", Uri(L"https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/")));

    m_items.Append(winrt::make<App1::implementation::BasicItem>(
        L"WinUI 3", Uri(L"https://docs.microsoft.com/en-us/windows/apps/winui/winui3/")));

    m_items.Append(winrt::make<App1::implementation::BasicItem>(
        L"C++/WinRT", Uri(L"https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/")));

    m_items.Append(
        winrt::make<App1::implementation::BasicItem>(L"Visual Studio", Uri(L"https://visualstudio.microsoft.com/")));
}

} // namespace winrt::App1::implementation
