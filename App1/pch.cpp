#include "pch.h"

#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <iostream>

namespace winrt::App1 {

auto make_logger(const char* name, std::ostream& out) noexcept(false) {
    using namespace spdlog::sinks;
    std::vector<spdlog::sink_ptr> sinks{};
#if defined(_DEBUG)
    sinks.emplace_back(std::make_shared<msvc_sink_st>());
#endif
    sinks.emplace_back(std::make_shared<ostream_sink_st>(out, false));
    return std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
}

auto make_logger(const char* name, FILE* fout) noexcept(false) {
    using namespace spdlog::sinks;
    std::vector<spdlog::sink_ptr> sinks{};
#if defined(_DEBUG)
    sinks.emplace_back(std::make_shared<msvc_sink_st>());
#endif
    if (fout == stdout)
        sinks.emplace_back(std::make_shared<wincolor_stdout_sink_st>());
    else if (fout == stderr)
        sinks.emplace_back(std::make_shared<wincolor_stderr_sink_st>());
    else {
        using mutex_t = spdlog::details::console_nullmutex;
        using sink_t = spdlog::sinks::stdout_sink_base<mutex_t>;
        sinks.emplace_back(std::make_shared<sink_t>(fout));
    }
    return std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
}

void set_log_stream(const char* name) noexcept(false) {
    std::shared_ptr logger = make_logger(name, std::cout);
    logger->set_pattern("%T.%e [%L] %8t %v");
#if defined(_DEBUG)
    logger->set_level(spdlog::level::level_enum::debug);
#endif
    spdlog::set_default_logger(logger);
}

DWORD get_module_path(WCHAR* path, UINT capacity) noexcept(false) {
    if (path == nullptr)
        throw std::invalid_argument{__func__};
    DWORD size = GetModuleFileNameW(nullptr, path, capacity);
    if (size == 0)
        throw std::system_error{static_cast<int>(GetLastError()), std::system_category(), "GetModuleFileNameW"};
    return size;
}

std::filesystem::path get_module_path() noexcept(false) {
    wchar_t buf[MAX_PATH]{};
    auto len = get_module_path(buf, MAX_PATH);
    return std::wstring_view{buf, len};
}

} // namespace winrt::App1
