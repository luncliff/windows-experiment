#include "pch.h"

#include "BasicItem.h"
#if __has_include("BasicItem.g.cpp")
#include "BasicItem.g.cpp"
#endif

using namespace winrt::Microsoft::UI::Xaml::Data;

namespace winrt::App1::implementation {

BasicItem::BasicItem() {
    // Default constructor
}

BasicItem::BasicItem(hstring const& name, Windows::Foundation::Uri const& webLink) : m_name(name), m_webLink(webLink) {
    // Parameterized constructor
}

hstring BasicItem::Name() {
    return m_name;
}

void BasicItem::Name(hstring const& value) {
    if (m_name != value) {
        m_name = value;
        RaisePropertyChanged(L"Name");
    }
}

Windows::Foundation::Uri BasicItem::WebLink() {
    return m_webLink;
}

winrt::event_token BasicItem::PropertyChanged(PropertyChangedEventHandler const& handler) {
    return m_propertyChanged.add(handler);
}

void BasicItem::PropertyChanged(winrt::event_token const& token) noexcept {
    m_propertyChanged.remove(token);
}

void BasicItem::RaisePropertyChanged(hstring const& propertyName) {
    m_propertyChanged(*this, PropertyChangedEventArgs(propertyName));
}

} // namespace winrt::App1::implementation
