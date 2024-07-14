/**
 * @see https://fmt.dev/latest/api.html#format-api
 * @note support "{}" expression
 */
#pragma once
#include <filesystem>
#include <string_view>

#include <fmt/chrono.h>
#include <fmt/xchar.h>
#include <winrt/base.h>

namespace fmt {

template <>
struct fmt::formatter<std::filesystem::path> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end)
            it++;
        if (it != end && *it != '}')
            throw format_error("bad format - std::filesystem::path");
        return it;
    }

    template <typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), L"{}", p.generic_wstring());
    }
};

template <>
struct fmt::formatter<winrt::hstring> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end)
            it++;
        if (it != end && *it != '}')
            throw format_error("bad format - winrt::hstring");
        return it;
    }

    template <typename FormatContext>
    auto format(winrt::hstring txt, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), L"{}", static_cast<std::wstring_view>(txt));
    }
};

template <>
struct fmt::formatter<winrt::hresult_error> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end)
            it++;
        if (it != end && *it != '}')
            throw format_error("bad format - winrt::hresult_error");
        return it;
    }

    template <typename FormatContext>
    auto format(const winrt::hresult_error& ex, FormatContext& ctx) const -> decltype(ctx.out()) {
        uint32_t c = static_cast<uint32_t>(ex.code());
        std::wstring_view m = ex.message();
        return fmt::format_to(ctx.out(), L"{:#08x} {}", c, m);
    }
};

} // namespace fmt
