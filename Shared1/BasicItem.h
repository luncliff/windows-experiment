#pragma once

#include "BasicItem.g.h"

namespace winrt::Shared1::implementation {

struct BasicItem : BasicItemT<BasicItem> {
  private:
    hstring m_name{};
    Windows::Foundation::Uri m_webLink{nullptr};
    winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    void RaisePropertyChanged(hstring const& propertyName);

  public:
    BasicItem();
    BasicItem(hstring const& name, Windows::Foundation::Uri const& webLink);

    hstring Name();
    void Name(hstring const& value);

    Windows::Foundation::Uri WebLink();

    winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;
};

} // namespace winrt::Shared1::implementation

namespace winrt::Shared1::factory_implementation {

struct BasicItem : BasicItemT<BasicItem, implementation::BasicItem> {};

} // namespace winrt::Shared1::factory_implementation
