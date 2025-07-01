#pragma once

#include "BasicItem.g.h"
#include "BasicViewModel.g.h"

namespace winrt::App1::implementation {
using Windows::Foundation::Collections::IObservableVector;

struct BasicViewModel : BasicViewModelT<BasicViewModel> {
  private:
    IObservableVector<App1::BasicItem> m_items;

    void InitializeItems();

  public:
    BasicViewModel();

    IObservableVector<App1::BasicItem> Items();
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {

struct BasicViewModel : BasicViewModelT<BasicViewModel, implementation::BasicViewModel> {};

} // namespace winrt::App1::factory_implementation
