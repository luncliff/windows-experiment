#pragma once
#include "RepositoryItem.g.h"

#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::App1::implementation {
using Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
using Microsoft::UI::Xaml::Data::PropertyChangedEventHandler;
using Windows::Foundation::Uri;
using Windows::Foundation::Collections::IObservableVector;
using Windows::Foundation::Collections::IVector;

struct RepositoryItem : RepositoryItemT<RepositoryItem> {
  private:
    winrt::hstring host;
    winrt::hstring name;
    winrt::hstring owner;
    winrt::event<PropertyChangedEventHandler> on_property_changed{};

  public:
    RepositoryItem() = default;
    RepositoryItem(std::wstring_view host, std::wstring_view owner, std::wstring_view name);

    winrt::hstring Host();
    void Host(winrt::hstring const& value);

    winrt::hstring Owner();
    void Owner(winrt::hstring const& value);

    winrt::hstring Name();
    void Name(winrt::hstring const& value);

    Uri ProjectUri();

    winrt::event_token PropertyChanged(PropertyChangedEventHandler const& value);
    void PropertyChanged(winrt::event_token const& token);
};

} // namespace winrt::App1::implementation

namespace winrt::App1::factory_implementation {
struct RepositoryItem : RepositoryItemT<RepositoryItem, implementation::RepositoryItem> {};
} // namespace winrt::App1::factory_implementation
