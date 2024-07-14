#include <CppUnitTest.h>

#include <Windows.h>
#include <winrt/windows.foundation.h>

#include <cstdio>
#include <memory>
#include <string_view>

#include "winrt_fmt_helper.hpp"
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

using Microsoft::VisualStudio::CppUnitTestFramework::Logger;

bool has_env(const char* key) noexcept {
    size_t len = 0;
    char buf[40]{};
    if (auto ec = getenv_s(&len, buf, key); ec != 0)
        return false;
    std::string_view value{buf, len};
    return value.empty() == false;
}

/**
 * @brief Redirect spdlog messages to `Logger::WriteMessage`
 */
class vstest_sink final : public spdlog::sinks::sink {
    std::unique_ptr<spdlog::formatter> formatter;

  public:
    void log(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t buf{};
        formatter->format(msg, buf);
        std::string txt = fmt::to_string(buf);
        Logger::WriteMessage(txt.c_str());
    }
    void flush() override {
        Logger::WriteMessage(L"\n");
    }
    void set_pattern(const std::string& p) override {
        formatter = std::make_unique<spdlog::pattern_formatter>(p);
    }
    void set_formatter(std::unique_ptr<spdlog::formatter> f) override {
        formatter = std::move(f);
    }
};

/**
 * @see https://docs.microsoft.com/en-us/windows/win32/sysinfo/getting-the-system-version
 */
TEST_MODULE_INITIALIZE(Initialize) {
    auto logger = std::make_shared<spdlog::logger>( //
        "UnitTests1", spdlog::sinks_init_list{
                          std::make_shared<spdlog::sinks::stdout_color_sink_st>(), //
                          std::make_shared<vstest_sink>()                          //
                      });
    logger->set_pattern("%T.%e [%L] %8t %v");
    logger->set_level(spdlog::level::level_enum::debug);
    spdlog::set_default_logger(logger);

    winrt::init_apartment();
    spdlog::info("C++/WinRT: {:s}", CPPWINRT_VERSION);
}

TEST_MODULE_CLEANUP(Cleanup) {
    winrt::uninit_apartment();
}
