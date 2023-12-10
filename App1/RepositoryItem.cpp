#include "pch.h"

#include "RepositoryItem.h"
#if __has_include("RepositoryItem.g.cpp")
#include "RepositoryItem.g.cpp"
#endif
#include "winrt_fmt_helper.hpp"
#include <fmt/format.h>

namespace winrt::App1::implementation {

RepositoryItem::RepositoryItem(std::wstring_view host, std::wstring_view owner, std::wstring_view name)
    : RepositoryItem{} {
    this->host = winrt::hstring{host};
    this->owner = winrt::hstring{owner};
    this->name = winrt::hstring{name};
}

winrt::event_token RepositoryItem::PropertyChanged(PropertyChangedEventHandler const& value) {
    return on_property_changed.add(value);
}

void RepositoryItem::PropertyChanged(winrt::event_token const& token) {
    on_property_changed.remove(token);
}

winrt::hstring RepositoryItem::Host() {
    return host;
}

void RepositoryItem::Host(winrt::hstring const& value) {
    if (host == value)
        return;
    host = value;
    on_property_changed(*this, PropertyChangedEventArgs{L"Host"});
}

winrt::hstring RepositoryItem::Owner() {
    return owner;
}

void RepositoryItem::Owner(winrt::hstring const& value) {
    if (owner == value)
        return;
    owner = value;
    on_property_changed(*this, PropertyChangedEventArgs{L"Owner"});
}

winrt::hstring RepositoryItem::Name() {
    return name;
}

void RepositoryItem::Name(winrt::hstring const& value) {
    if (name == value)
        return;
    name = value;
    on_property_changed(*this, PropertyChangedEventArgs{L"Name"});
}

Uri RepositoryItem::ProjectUri() {
    std::wstring value = fmt::format(L"https://{}/{}/{}",                   //
                                     static_cast<std::wstring_view>(host),  //
                                     static_cast<std::wstring_view>(owner), //
                                     static_cast<std::wstring_view>(name));
    return Uri{value};
}

} // namespace winrt::App1::implementation
