#include "pch.h"

#include "BasicViewModel.h"
#if __has_include("BasicViewModel.g.cpp")
#include "BasicViewModel.g.cpp"
#endif
#include "BasicItem.h"

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;

namespace winrt::Shared1::implementation {

BasicViewModel::BasicViewModel() {
    m_items = winrt::single_threaded_observable_vector<Shared1::BasicItem>();
    InitializeItems();
}

IObservableVector<Shared1::BasicItem> BasicViewModel::Items() {
    return m_items;
}

StorageFolder BasicViewModel::GetLogFolder() {
    return m_log_folder;
}

winrt::hstring BasicViewModel::GetLogFolderPath() {
    if (m_log_folder == nullptr)
        throw winrt::hresult_error{E_FAIL, L"call CreateLogFolderAsync before this function"};
    return m_log_folder.Path();
}

IAsyncAction BasicViewModel::CreateLogFolderAsync() {
    try {
        StorageFolder appdata = ApplicationData::Current().LocalFolder();
        m_log_folder = co_await appdata.CreateFolderAsync(L"logs", CreationCollisionOption::OpenIfExists);
        spdlog::info(L"Log folder created: {}", static_cast<std::wstring_view>(GetLogFolderPath()));
    } catch (const winrt::hresult_error& ex) {
        spdlog::error(L"Failed to create logs folder: {}", static_cast<std::wstring_view>(ex.message()));
    }
}

void BasicViewModel::InitializeItems() {
    // Add some sample items
    m_items.Append(
        winrt::make<Shared1::implementation::BasicItem>(L"Microsoft Learn", Uri(L"https://learn.microsoft.com/")));

    m_items.Append(winrt::make<Shared1::implementation::BasicItem>(
        L"Windows App SDK", Uri(L"https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/")));

    m_items.Append(winrt::make<Shared1::implementation::BasicItem>(
        L"WinUI 3", Uri(L"https://docs.microsoft.com/en-us/windows/apps/winui/winui3/")));

    m_items.Append(winrt::make<Shared1::implementation::BasicItem>(
        L"C++/WinRT", Uri(L"https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/")));

    m_items.Append(
        winrt::make<Shared1::implementation::BasicItem>(L"Visual Studio", Uri(L"https://visualstudio.microsoft.com/")));
}

} // namespace winrt::Shared1::implementation
