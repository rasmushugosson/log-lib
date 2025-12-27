#pragma once

/*
 * Author: Rasmus Hugosson
 * Date: 2025-12-05
 *
 * Full source at: https://github.com/rasmushugosson/log-lib
 */

#include <chrono>
#include <cstdint>
#include <exception>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <print>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef AE_WINDOWS
#include <Windows.h>
#endif

#undef ERROR

constexpr const std::string_view c_LogLibVersion = "Log Lib Version 1.1.0";

namespace ae
{
enum class LogLevel : uint8_t
{
    TRACE = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

struct LogMessage
{
    LogLevel level;
    std::chrono::system_clock::time_point time;
    std::string_view file;
    std::string_view function;
    uint_least32_t line;
    std::string message;
};

inline std::string_view GetFileName(std::string_view path) noexcept
{
    const auto pos = path.find_last_of("/\\");
    return (pos == std::string_view::npos) ? path : path.substr(pos + 1);
}

#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
typedef std::move_only_function<void(const LogMessage &) const &> LogSink;
#else
typedef std::function<void(const LogMessage &) const &> callback LogSink;
#endif

enum class LogSinkConsoleKind : uint8_t
{
    STDOUT = 0,
    STDERR,
};

class Timer
{
  public:
    Timer();
    ~Timer() = default;

    void Start();
    void Stop();
    void Reset();

    [[nodiscard]] double GetElapsedTime() const;

    template <class Dur = std::chrono::duration<double>> [[nodiscard]] inline Dur GetElapsedTimeAs() const
    {
        if (m_Running)
        {
            return std::chrono::duration_cast<Dur>(m_ElapsedTime + (std::chrono::steady_clock::now() - m_Start));
        }
        return std::chrono::duration_cast<Dur>(m_ElapsedTime);
    }

    [[nodiscard]] std::string GetElapsedTimeAsString(int32_t decimals) const;

  private:
    std::chrono::steady_clock::time_point m_Start;
    std::chrono::steady_clock::duration m_ElapsedTime;
    bool m_Running;
};

class Logger
{
  private:
    Logger();

  public:
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;
    ~Logger() noexcept;

    inline static Logger &Get()
    {
        static Logger instance;
        return instance;
    }

    template <class... Args>
    inline void Log(LogLevel level, std::source_location loc, std::format_string<Args...> fmt, Args &&...args) const
    {
        std::string message;
        message.reserve(128);
        std::format_to(std::back_inserter(message), fmt, std::forward<Args>(args)...);

        auto file = GetFileName(std::string_view{ loc.file_name() });

        LogMessage logMessage{ .level = level,
                               .time = std::chrono::system_clock::now(),
                               .file = file,
                               .function = loc.function_name(),
                               .line = loc.line(),
                               .message = std::move(message) };

        for (const auto &[name, sink] : m_Sinks)
        {
            sink(logMessage);
        }
    }

    inline void Log(LogLevel level, std::source_location loc, std::string_view fmt, std::format_args args) const
    {
        std::string message;
        message.reserve(128);
        std::vformat_to(std::back_inserter(message), fmt, args);

        auto file = GetFileName(std::string_view{ loc.file_name() });

        LogMessage logMessage{ .level = level,
                               .time = std::chrono::system_clock::now(),
                               .file = file,
                               .function = loc.function_name(),
                               .line = loc.line(),
                               .message = std::move(message) };

        for (const auto &[name, sink] : m_Sinks)
        {
            sink(logMessage);
        }
    }

    inline void Newline() const
    {
        for (const auto &[name, stream] : m_Streams)
        {
            std::println(stream, "");
        }
    }

    inline void NewlineConsole() const
    {
        for (const auto &[name, stream] : m_Streams)
        {
            if (stream == stdout || stream == stderr)
            {
                std::println(stream, "");
            }
        }
    }

    inline void NewlineFile() const
    {
        for (const auto &[name, stream] : m_FileStreams)
        {
            std::println(stream, "");
        }
    }

    void AddConsoleSink(const std::string &name, LogSinkConsoleKind type = LogSinkConsoleKind::STDOUT,
                        LogLevel minLevel = LogLevel::TRACE, LogLevel maxLevel = LogLevel::FATAL);
    void AddFileSink(const std::string &name, const std::string &path, LogLevel minLevel = LogLevel::TRACE,
                     LogLevel maxLevel = LogLevel::FATAL);

    void RemoveSink(const std::string &name);

    inline void SetOpenMessage(const std::string &message)
    {
        m_OpenMessage = message;
    }

  private:
    void Close();

    void PrintOpenMessage(FILE *stream) const;
    void PrintCloseMessage(FILE *stream) const;
    void PrintTerminationMessage(FILE *stream) const;

  private:
    std::unordered_map<std::string, LogSink> m_Sinks;
    std::unordered_map<std::string, FILE *> m_Streams;
    std::unordered_map<std::string, FILE *> m_FileStreams;
    std::string m_OpenMessage;

    std::chrono::steady_clock::time_point m_StartPoint;
    std::string m_StartDate;
    std::string m_StartTime;
    Timer m_ExecutionTimer;
};

class Console
{
  private:
    Console();

  public:
    Console(const Console &) = delete;
    Console &operator=(const Console &) = delete;
    Console(Console &&) = delete;
    Console &operator=(Console &&) = delete;
    ~Console();

    inline static Console &GetInstance()
    {
        static Console instance;
        return instance;
    }

    void SetColor(LogLevel level);

  private:
    void Update() const;

  private:
#ifdef AE_WINDOWS
    using ConsoleColorCode = WORD;
#else
    using ConsoleColorCode = int; // ANSI code number
#endif
    ConsoleColorCode m_ForegroundColor;
    ConsoleColorCode m_BackgroundColor;
};

#define AE_TRACE ae::LogLevel::TRACE
#define AE_INFO ae::LogLevel::INFO
#define AE_WARNING ae::LogLevel::WARNING
#define AE_ERROR ae::LogLevel::ERROR
#define AE_FATAL ae::LogLevel::FATAL

#ifdef AE_DEBUG

#define AE_LOG(lv, fmt, ...) ae::Logger::Get().Log(lv, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_TRACE(fmt, ...)                                                                                         \
    ae::Logger::Get().Log(ae::LogLevel::TRACE, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_INFO(fmt, ...)                                                                                          \
    ae::Logger::Get().Log(ae::LogLevel::INFO, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_WARNING(fmt, ...)                                                                                       \
    ae::Logger::Get().Log(ae::LogLevel::WARNING, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_ERROR(fmt, ...)                                                                                         \
    ae::Logger::Get().Log(ae::LogLevel::ERROR, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_FATAL(fmt, ...)                                                                                         \
    ae::Logger::Get().Log(ae::LogLevel::FATAL, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

#define AE_LOG_RELEASE(lv, fmt, ...)
#define AE_LOG_RELEASE_TRACE(fmt, ...)
#define AE_LOG_RELEASE_INFO(fmt, ...)
#define AE_LOG_RELEASE_WARNING(fmt, ...)
#define AE_LOG_RELEASE_ERROR(fmt, ...)
#define AE_LOG_RELEASE_FATAL(fmt, ...)

#define AE_LOG_BOTH(lv, fmt, ...)                                                                                      \
    ae::Logger::Get().Log(lv, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_TRACE(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::TRACE, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_INFO(fmt, ...)                                                                                     \
    ae::Logger::Get().Log(ae::LogLevel::INFO, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_WARNING(fmt, ...)                                                                                  \
    ae::Logger::Get().Log(ae::LogLevel::WARNING, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_ERROR(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::ERROR, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_FATAL(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::FATAL, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

#define AE_LOG_NEWLINE_BOTH() ae::Logger::Get().Newline()
#define AE_LOG_NEWLINE_BOTH_CONSOLE() ae::Logger::Get().NewlineConsole()
#define AE_LOG_NEWLINE_BOTH_FILE() ae::Logger::Get().NewlineFile()

#define AE_LOG_NEWLINE() ae::Logger::Get().Newline()
#define AE_LOG_NEWLINE_CONSOLE() ae::Logger::Get().NewlineConsole()
#define AE_LOG_NEWLINE_FILE() ae::Logger::Get().NewlineFile()
#define AE_LOG_NEWLINE_RELEASE()
#define AE_LOG_NEWLINE_RELEASE_CONSOLE()
#define AE_LOG_NEWLINE_RELEASE_FILE()

#elif AE_RELEASE // AE_DEBUG

#define AE_LOG(lv, fmt, ...)
#define AE_LOG_TRACE(fmt, ...)
#define AE_LOG_INFO(fmt, ...)
#define AE_LOG_WARNING(fmt, ...)
#define AE_LOG_ERROR(fmt, ...)
#define AE_LOG_FATAL(fmt, ...)

#define AE_LOG_RELEASE(lv, fmt, ...)                                                                                   \
    ae::Logger::Get().Log(lv, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_RELEASE_TRACE(fmt, ...)                                                                                 \
    ae::Logger::Get().Log(ae::LogLevel::TRACE, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_RELEASE_INFO(fmt, ...)                                                                                  \
    ae::Logger::Get().Log(ae::LogLevel::INFO, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_RELEASE_WARNING(fmt, ...)                                                                               \
    ae::Logger::Get().Log(ae::LogLevel::WARNING, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_RELEASE_ERROR(fmt, ...)                                                                                 \
    ae::Logger::Get().Log(ae::LogLevel::ERROR, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_RELEASE_FATAL(fmt, ...)                                                                                 \
    ae::Logger::Get().Log(ae::LogLevel::FATAL, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

#define AE_LOG_BOTH(lv, fmt, ...)                                                                                      \
    ae::Logger::Get().Log(lv, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_TRACE(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::TRACE, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_INFO(fmt, ...)                                                                                     \
    ae::Logger::Get().Log(ae::LogLevel::INFO, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_WARNING(fmt, ...)                                                                                  \
    ae::Logger::Get().Log(ae::LogLevel::WARNING, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_ERROR(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::ERROR, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define AE_LOG_BOTH_FATAL(fmt, ...)                                                                                    \
    ae::Logger::Get().Log(ae::LogLevel::FATAL, std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

#define AE_LOG_NEWLINE_BOTH() ae::Logger::Get().Newline()
#define AE_LOG_NEWLINE_BOTH_CONSOLE() ae::Logger::Get().NewlineConsole()
#define AE_LOG_NEWLINE_BOTH_FILE() ae::Logger::Get().NewlineFile()

#define AE_LOG_NEWLINE()
#define AE_LOG_NEWLINE_CONSOLE()
#define AE_LOG_NEWLINE_FILE()
#define AE_LOG_NEWLINE_RELEASE() ae::Logger::Get().Newline()
#define AE_LOG_NEWLINE_RELEASE_CONSOLE() ae::Logger::Get().NewlineConsole()
#define AE_LOG_NEWLINE_RELEASE_FILE() ae::Logger::Get().NewlineFile()

#else // AE_DEBUG

#define AE_LOG(lv, fmt, ...)
#define AE_LOG(lv, fmt, ...)
#define AE_LOG_TRACE(fmt, ...)
#define AE_LOG_INFO(fmt, ...)
#define AE_LOG_WARNING(fmt, ...)
#define AE_LOG_ERROR(fmt, ...)
#define AE_LOG_FATAL(fmt, ...)

#define AE_LOG_RELEASE(lv, fmt, ...)
#define AE_LOG_RELEASE(lv, fmt, ...)
#define AE_LOG_RELEASE_TRACE(fmt, ...)
#define AE_LOG_RELEASE_INFO(fmt, ...)
#define AE_LOG_RELEASE_WARNING(fmt, ...)
#define AE_LOG_RELEASE_ERROR(fmt, ...)
#define AE_LOG_RELEASE_FATAL(fmt, ...)

#define AE_LOG_BOTH(lv, fmt, ...)
#define AE_LOG_BOTH_TRACE(fmt, ...)
#define AE_LOG_BOTH_INFO(fmt, ...)
#define AE_LOG_BOTH_WARNING(fmt, ...)
#define AE_LOG_BOTH_ERROR(fmt, ...)
#define AE_LOG_BOTH_FATAL(fmt, ...)

#define AE_LOG_NEWLINE_BOTH()
#define AE_LOG_NEWLINE_BOTH_CONSOLE()
#define AE_LOG_NEWLINE_BOTH_FILE()

#define AE_LOG_NEWLINE()
#define AE_LOG_NEWLINE_CONSOLE()
#define AE_LOG_NEWLINE_FILE()
#define AE_LOG_NEWLINE_RELEASE()
#define AE_LOG_NEWLINE_RELEASE_CONSOLE()
#define AE_LOG_NEWLINE_RELEASE_FILE()

#endif // AE_DEBUG

#define AE_LOG_DEBUG AE_LOG
#define AE_LOG_DEBUG_TRACE AE_LOG_TRACE
#define AE_LOG_DEBUG_INFO AE_LOG_INFO
#define AE_LOG_DEBUG_WARNING AE_LOG_WARNING
#define AE_LOG_DEBUG_ERROR AE_LOG_ERROR
#define AE_LOG_DEBUG_FATAL AE_LOG_FATAL

#define AE_LOG_NEWLINE_DEBUG AE_LOG_NEWLINE
#define AE_LOG_NEWLINE_DEBUG_CONSOLE AE_LOG_NEWLINE_CONSOLE
#define AE_LOG_NEWLINE_DEBUG_FILE AE_LOG_NEWLINE_FILE

// Exceptions
// ---------------------------------------------------------------------------------------------------------------------------------------

template <class... Args>
inline std::string FormatError(std::string_view type, std::source_location loc, std::format_string<Args...> fmt,
                               Args &&...args)
{
    std::string content;
    content.reserve(192);

    std::format_to(std::back_inserter(content), fmt, std::forward<Args>(args)...);

    std::string message;
    message.reserve(255);

    std::format_to(std::back_inserter(message), "\n\n[{}]\n\nIn:\t{}:{} ({})\nWhat:\t{}\n", type,
                   GetFileName(std::string_view{ loc.file_name() }), loc.line(), loc.function_name(), content);

    return message;
}

inline std::string FormatError(std::string type, std::source_location loc, std::string_view fmt, std::format_args args)
{
    std::string content;
    content.reserve(192);

    std::vformat_to(std::back_inserter(content), fmt, args);

    std::string message;
    message.reserve(255);

    std::format_to(std::back_inserter(message), "\n\n[{}]\n\nIn:\t{}:{} ({})\nWhat:\t{}\n", type,
                   GetFileName(std::string_view{ loc.file_name() }), loc.line(), loc.function_name(), content);

    return message;
}

class LogicError : public std::logic_error
{
  public:
    template <class... Args>
    explicit LogicError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::logic_error(FormatError("Logic error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit LogicError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::logic_error(FormatError("Logic error", loc, fmt, args))
    {
    }
};

#define AE_THROW_LOGIC_ERROR(fmt, ...)                                                                                 \
    throw ae::LogicError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class InvalidArgument : public std::invalid_argument
{
  public:
    template <class... Args>
    explicit InvalidArgument(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::invalid_argument(FormatError("Invalid error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit InvalidArgument(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::invalid_argument(FormatError("Invalid error", loc, fmt, args))
    {
    }
};

#define AE_THROW_INVALID_ARGUMENT(fmt, ...)                                                                            \
    throw ae::InvalidArgument(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class MathError : public std::domain_error
{
  public:
    template <class... Args>
    explicit MathError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::domain_error(FormatError("Math error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit MathError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::domain_error(FormatError("Math error", loc, fmt, args))
    {
    }
};

#define AE_THROW_MATH_ERROR(fmt, ...)                                                                                  \
    throw ae::MathError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class LengthError : public std::length_error
{
  public:
    template <class... Args>
    explicit LengthError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::length_error(FormatError("Length error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit LengthError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::length_error(FormatError("Length error", loc, fmt, args))
    {
    }
};

#define AE_THROW_LENGTH_ERROR(fmt, ...)                                                                                \
    throw ae::LengthError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class OutOfRangeError : public std::out_of_range
{
  public:
    template <class... Args>
    explicit OutOfRangeError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::out_of_range(FormatError("Out of range error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit OutOfRangeError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::out_of_range(FormatError("Out of range error", loc, fmt, args))
    {
    }
};

#define AE_THROW_OUT_OF_RANGE_ERROR(fmt, ...)                                                                          \
    throw ae::OutOfRangeError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class RuntimeError : public std::runtime_error
{
  public:
    template <class... Args>
    explicit RuntimeError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(FormatError("Runtime error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit RuntimeError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::runtime_error(FormatError("Runtime error", loc, fmt, args))
    {
    }
};

#define AE_THROW_RUNTIME_ERROR(fmt, ...)                                                                               \
    throw ae::RuntimeError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class RangeError : public std::range_error
{
  public:
    template <class... Args>
    explicit RangeError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::range_error(FormatError("Range error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit RangeError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::range_error(FormatError("Range error", loc, fmt, args))
    {
    }
};

#define AE_THROW_RANGE_ERROR(fmt, ...)                                                                                 \
    throw ae::RangeError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class OverflowError : public std::overflow_error
{
  public:
    template <class... Args>
    explicit OverflowError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::overflow_error(FormatError("Overflow error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit OverflowError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::overflow_error(FormatError("Overflow error", loc, fmt, args))
    {
    }
};

#define AE_THROW_OVERFLOW_ERROR(fmt, ...)                                                                              \
    throw ae::OverflowError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class UnderflowError : public std::underflow_error
{
  public:
    template <class... Args>
    explicit UnderflowError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::underflow_error(FormatError("Underflow error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit UnderflowError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::underflow_error(FormatError("Underflow error", loc, fmt, args))
    {
    }
};

#define AE_THROW_UNDERFLOW_ERROR(fmt, ...)                                                                             \
    throw ae::UnderflowError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class FileNotFoundError : public std::runtime_error
{
  public:
    template <class... Args>
    explicit FileNotFoundError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(FormatError("File not found error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit FileNotFoundError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::runtime_error(FormatError("File not found error", loc, fmt, args))
    {
    }
};

#define AE_THROW_FILE_NOT_FOUND_ERROR(fmt, ...)                                                                        \
    throw ae::FileNotFoundError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class FilesystemError : public std::runtime_error
{
  public:
    template <class... Args>
    explicit FilesystemError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(FormatError("Filesystem error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit FilesystemError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::runtime_error(FormatError("Filesystem error", loc, fmt, args))
    {
    }
};

#define AE_THROW_FILESYSTEM_ERROR(fmt, ...)                                                                            \
    throw ae::FilesystemError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

class FileOpenError : public std::runtime_error
{
  public:
    template <class... Args>
    explicit FileOpenError(std::source_location loc, std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(FormatError("File open error", loc, fmt, std::forward<Args>(args)...))
    {
    }

    explicit FileOpenError(std::source_location loc, std::string_view fmt, std::format_args args)
        : std::runtime_error(FormatError("File open error", loc, fmt, args))
    {
    }
};

#define AE_THROW_FILE_OPEN_ERROR(fmt, ...)                                                                             \
    throw ae::FileOpenError(std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

// Time
// ---------------------------------------------------------------------------------------------------------------------------------------------

enum class TimeZoneError : uint8_t
{
    UNKNOWN = 0,
    TZDB_UNAVAILABLE,            // TZ database not present / unreadable
    CANNOT_DETERMINE_LOCAL_ZONE, // Theoretically if current_zone() returned null
};

constexpr std::string_view to_string(TimeZoneError e)
{
    switch (e)
    {
    case TimeZoneError::UNKNOWN:
        return "Failed to get time zone: Unknown error";
    case TimeZoneError::TZDB_UNAVAILABLE:
        return "Failed to get time zone: TZ database unavailable on system";
    case TimeZoneError::CANNOT_DETERMINE_LOCAL_ZONE:
        return "Failed to get time zone: Cannot determine local time zone";
    default:
        return "Unknown time zone error";
    }
}

class DateTime
{
  public:
    inline static void Wait(double seconds)
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
    }

    template <class Rep, class Period> inline static void Wait(std::chrono::duration<Rep, Period> d)
    {
        std::this_thread::sleep_for(d);
    }

    inline static void WaitUntil(std::chrono::steady_clock::time_point timePoint)
    {
        std::this_thread::sleep_until(timePoint);
    }

    static std::chrono::steady_clock::time_point SteadyNow()
    {
        return std::chrono::steady_clock::now();
    }

    static std::chrono::system_clock::time_point SystemNow()
    {
        return std::chrono::system_clock::now();
    }

    enum class ZoneKind : uint8_t
    {
        LOCAL,
        UTC
    };

    inline static std::string FormatNow(std::string_view fmt, ZoneKind where)
    {
        const auto tp = std::chrono::floor<std::chrono::milliseconds>(SystemNow());

        if (where == ZoneKind::LOCAL)
        {
            std::chrono::zoned_time zt{ std::chrono::current_zone(), tp };
            return std::vformat(fmt, std::make_format_args(zt));
        }

        else
        {
            return std::vformat(
                fmt, std::make_format_args(tp)); // %Ez becomes "+00:00" when no zone: omit it in fmt for pure UTC
        }
    }

    [[nodiscard]] inline static std::string NowAsString()
    {
        return FormatNow("{:%F %T%Ez}", ZoneKind::LOCAL);
    }
    [[nodiscard]] inline static std::string NowAsUTCString()
    {
        return FormatNow("{:%FT%TZ}", ZoneKind::UTC);
    }

    [[nodiscard]] inline static std::string TimeAsString()
    {
        return FormatNow("{:%T}", ZoneKind::LOCAL);
    }
    [[nodiscard]] inline static std::string TimeAsUTCString()
    {
        return FormatNow("{:%TZ}", ZoneKind::UTC);
    }

    [[nodiscard]] inline static std::string DateAsString()
    {
        return FormatNow("{:%F}", ZoneKind::LOCAL);
    }
    [[nodiscard]] inline static std::string DateAsUTCString()
    {
        return FormatNow("{:%F}", ZoneKind::UTC);
    }

    [[nodiscard]] inline static std::string DateTimeAsString()
    {
        return FormatNow("{:%F %T%Ez}", ZoneKind::LOCAL);
    }
    [[nodiscard]] inline static std::string DateTimeAsUTCString()
    {
        return FormatNow("{:%FT%TZ}", ZoneKind::UTC);
    }

    [[nodiscard]] inline static std::expected<std::string, TimeZoneError> TimeZoneAsString() noexcept
    {
        try
        {
            const auto *z = std::chrono::current_zone();
            if (z)
            {
                return std::string(z->name());
            }
            return std::unexpected(TimeZoneError::CANNOT_DETERMINE_LOCAL_ZONE);
        }

        catch (const std::runtime_error &)
        {
            return std::unexpected(TimeZoneError::TZDB_UNAVAILABLE);
        }

        catch (...)
        {
            return std::unexpected(TimeZoneError::UNKNOWN);
        }
    }
};
} // namespace ae

namespace std
{
template <> struct formatter<ae::TimeZoneError, char> : formatter<std::string_view, char>
{
    auto format(ae::TimeZoneError e, auto &ctx) const
    {
        return formatter<std::string_view, char>::format(ae::to_string(e), ctx);
    }
};
} // namespace std
