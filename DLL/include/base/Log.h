#pragma once
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>
#include <utility>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#define _WINSOCKAPI_
#include <windows.h>

#include "Platform.h"

namespace GG
{
enum class LogLevel : unsigned char
{
    DebugPlusPlus = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,

#if defined(_DEBUG)
    Current = Debug
#else
    Current = Info
#endif
};

namespace detail
{
#if defined(_DEBUG)
inline constexpr bool IsDebugBuild = true;
#else
inline constexpr bool IsDebugBuild = false;
#endif

inline constexpr std::string_view Reset = "\x1b[0m";

[[nodiscard]] inline constexpr std::string_view LOG_LEVEL_TO_STRING(LogLevel lvl) noexcept
{
    switch (lvl)
    {
    case LogLevel::DebugPlusPlus:
        return "Debug++";
    case LogLevel::Debug:
        return "Debug";
    case LogLevel::Info:
        return "Info";
    case LogLevel::Warning:
        return "Warning";
    case LogLevel::Error:
        return "Error";
    case LogLevel::Fatal:
        return "Fatal";
    default:
        return "Unknown";
    }
}

[[nodiscard]] inline constexpr std::string_view LOG_LEVEL_COLOR(LogLevel lvl) noexcept
{
    switch (lvl)
    {
    case LogLevel::DebugPlusPlus:
    case LogLevel::Debug:
        return "\x1b[36m";
    case LogLevel::Info:
        return "";
    case LogLevel::Warning:
        return "\x1b[33m";
    case LogLevel::Error:
    case LogLevel::Fatal:
        return "\x1b[31m";
    default:
        return "";
    }
}

[[nodiscard]] inline LogLevel effective_log_level() noexcept
{
    static const LogLevel level = []() noexcept -> LogLevel {
        char env[16]{};
        DWORD len = GetEnvironmentVariableA("GG_LOG_LEVEL", env, sizeof(env));
        if (!len)
            return LogLevel::Current;
        while (len > 0 && env[len - 1] == ' ')
            env[--len] = '\0';
        if (_stricmp(env, "Debug++") == 0)
            return LogLevel::DebugPlusPlus;
        if (_stricmp(env, "Debug") == 0)
            return LogLevel::Debug;
        if (_stricmp(env, "Info") == 0)
            return LogLevel::Info;
        if (_stricmp(env, "Warning") == 0)
            return LogLevel::Warning;
        if (_stricmp(env, "Error") == 0)
            return LogLevel::Error;
        if (_stricmp(env, "Fatal") == 0)
            return LogLevel::Fatal;
        return LogLevel::Current;
    }();
    return level;
}

[[nodiscard]] inline bool should_log(LogLevel lvl) noexcept
{
    return static_cast<unsigned>(lvl) >= static_cast<unsigned>(effective_log_level());
}

[[nodiscard]] inline constexpr std::string_view filename(std::string_view path) noexcept
{
    const auto slash = path.find_last_of("/\\");
    return (slash == std::string_view::npos) ? path : path.substr(slash + 1);
}

inline void print_header(LogLevel lvl, const std::source_location& loc) noexcept
{
    std::printf("%.*s[%.*s:%.*s] [%.*s]", (int)LOG_LEVEL_COLOR(lvl).size(), LOG_LEVEL_COLOR(lvl).data(), (int)GG::Version::AppName.size(),
        GG::Version::AppName.data(), (int)GG::Version::BuildName.size(), GG::Version::BuildName.data(),
        (int)LOG_LEVEL_TO_STRING(lvl).size(), LOG_LEVEL_TO_STRING(lvl).data());

    if constexpr (IsDebugBuild)
    {
        const auto fn = filename(loc.file_name());
        std::printf(" [%.*s:%u] ", (int)fn.size(), fn.data(), loc.line());
    }
    else
    {
        (void)loc;
        std::printf(" ");
    }
}

inline void rotate_logs() noexcept
{
    static size_t s_line_count = 0;

    if (++s_line_count < 5000)
        return;

    s_line_count = 0;
    std::fflush(stdout);

    std::remove("gg_server.log.5");

    for (int i = 4; i >= 1; --i)
    {
        std::string old_name = "gg_server.log." + std::to_string(i);
        std::string new_name = "gg_server.log." + std::to_string(i + 1);
        std::rename(old_name.c_str(), new_name.c_str());
    }

    std::rename("gg_server.log", "gg_server.log.1");

    FILE* dummy;
    if (freopen_s(&dummy, "gg_server.log", "w", stdout) != 0)
    {
        std::printf("Warning: Failed to rotate log file\n");
    }
}

template<class... Args>
inline void vlogf(LogLevel lvl, const std::source_location& loc, const char* fmt, Args&&... args) noexcept
{
    if (!should_log(lvl))
        return;

    rotate_logs();

    print_header(lvl, loc);
    std::printf(fmt, std::forward<Args>(args)...);
    std::printf("%.*s\n", (int)Reset.size(), Reset.data());
    std::fflush(stdout);
}

inline void platform_fatal_dialog(const char* message) noexcept
{
    ::MessageBoxA(nullptr, message, "Fatal Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
}

[[noreturn]] inline void hard_abort() noexcept
{
    std::fflush(stdout);
#if defined(_MSC_VER) && defined(_DEBUG)
    __debugbreak();
#endif
    std::abort();
}
} // namespace detail

template<class... Args>
inline void logf(LogLevel lvl, const std::source_location& loc, const char* fmt, Args&&... args) noexcept
{
    detail::vlogf(lvl, loc, fmt, std::forward<Args>(args)...);
}

template<class... Args>
[[noreturn]] inline void fatalf(const std::source_location& loc, const char* fmt, Args&&... args) noexcept
{
    char buf[2048]{};
    std::snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);

    detail::vlogf(LogLevel::Fatal, loc, "%s", buf);
    detail::platform_fatal_dialog(buf);
    detail::hard_abort();
}
} // namespace GG

#define GG_LOG(level, fmt, ...) ::GG::logf((level), std::source_location::current(), (fmt), ##__VA_ARGS__)

#define GG_FATAL(fmt, ...) ::GG::fatalf(std::source_location::current(), (fmt), ##__VA_ARGS__)