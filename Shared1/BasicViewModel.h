#pragma once

#include "BasicItem.g.h"
#include "BasicViewModel.g.h"

namespace winrt::Shared1::implementation {
using Windows::Foundation::IAsyncAction;
using Windows::Foundation::Collections::IObservableVector;
using Windows::Storage::StorageFolder;

struct BasicViewModel : BasicViewModelT<BasicViewModel> {
  private:
    IObservableVector<Shared1::BasicItem> m_items;
    StorageFolder m_log_folder{nullptr};

    void InitializeItems();

  public:
    BasicViewModel();

    IObservableVector<Shared1::BasicItem> Items();
    IAsyncAction CreateLogFolderAsync();
    StorageFolder GetLogFolder();
    winrt::hstring GetLogFolderPath();
};

} // namespace winrt::Shared1::implementation

namespace winrt::Shared1::factory_implementation {

struct BasicViewModel : BasicViewModelT<BasicViewModel, implementation::BasicViewModel> {};

} // namespace winrt::Shared1::factory_implementation
