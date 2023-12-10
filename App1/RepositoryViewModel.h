#pragma once
#include "RepositoryViewModel.g.h"

#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
using Microsoft::UI::Xaml::Data::PropertyChangedEventHandler;
using Windows::Foundation::Collections::IObservableVector;
using Windows::Foundation::Collections::IVector;

struct RepositoryViewModel : RepositoryViewModelT<RepositoryViewModel> {
private:
  IObservableVector<App1::RepositoryItem> items;

public:
  RepositoryViewModel();

  IObservableVector<App1::RepositoryItem> Repositories();
};
} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct RepositoryViewModel
    : RepositoryViewModelT<RepositoryViewModel,
                           implementation::RepositoryViewModel> {};
} // namespace winrt::App1::factory_implementation
