#include "pch.h"

#include "RepositoryViewModel.h"
#if __has_include("RepositoryViewModel.g.cpp")
#include "RepositoryViewModel.g.cpp"
#endif
#include "RepositoryItem.h"

#include <spdlog/spdlog.h>

namespace winrt::App1::implementation {

RepositoryViewModel::RepositoryViewModel() {
  items = winrt::single_threaded_observable_vector<App1::RepositoryItem>();
  items.Append(
      winrt::make<implementation::RepositoryItem>(L"github.com", L"microsoft", L"vcpkg"));
  items.Append(
      winrt::make<implementation::RepositoryItem>(L"github.com",L"conan-io", L"conan"));
}

IObservableVector<App1::RepositoryItem> RepositoryViewModel::Repositories() {
  return items;
}

} // namespace winrt::App1::implementation