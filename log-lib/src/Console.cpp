#include "general/pch.h"

#include "Log.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <print>

#ifdef AE_WINDOWS

using ColorWord = uint16_t;

enum class ConsoleForegroundColor : ColorWord
{
    BLACK = 0,
    RED = static_cast<ColorWord>(FOREGROUND_RED),
    GREEN = static_cast<ColorWord>(FOREGROUND_GREEN),
    BLUE = static_cast<ColorWord>(FOREGROUND_BLUE),
    YELLOW = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_GREEN),
    MAGENTA = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_BLUE),
    CYAN = static_cast<ColorWord>(FOREGROUND_GREEN | FOREGROUND_BLUE),
    LIGHT_GRAY = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE),
    GRAY = static_cast<ColorWord>(FOREGROUND_INTENSITY),
    LIGHT_RED = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_INTENSITY),
    LIGHT_GREEN = static_cast<ColorWord>(FOREGROUND_GREEN | FOREGROUND_INTENSITY),
    LIGHT_BLUE = static_cast<ColorWord>(FOREGROUND_BLUE | FOREGROUND_INTENSITY),
    LIGHT_YELLOW = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY),
    LIGHT_MAGENTA = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY),
    LIGHT_CYAN = static_cast<ColorWord>(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY),
    WHITE = static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
};

enum class ConsoleBackgroundColor : ColorWord
{
    BLACK = 0,
    RED = static_cast<ColorWord>(BACKGROUND_RED),
    GREEN = static_cast<ColorWord>(BACKGROUND_GREEN),
    BLUE = static_cast<ColorWord>(BACKGROUND_BLUE),
    YELLOW = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_GREEN),
    MAGENTA = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_BLUE),
    CYAN = static_cast<ColorWord>(BACKGROUND_GREEN | BACKGROUND_BLUE),
    LIGHT_GRAY = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE),
    GRAY = static_cast<ColorWord>(BACKGROUND_INTENSITY),
    LIGHT_RED = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_INTENSITY),
    LIGHT_GREEN = static_cast<ColorWord>(BACKGROUND_GREEN | BACKGROUND_INTENSITY),
    LIGHT_BLUE = static_cast<ColorWord>(BACKGROUND_BLUE | BACKGROUND_INTENSITY),
    LIGHT_YELLOW = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY),
    LIGHT_MAGENTA = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY),
    LIGHT_CYAN = static_cast<ColorWord>(BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY),
    WHITE = static_cast<ColorWord>(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY)
};

constexpr std::array<ColorWord, 5> c_ForegroundLookup = {
    static_cast<ColorWord>(ConsoleForegroundColor::GRAY),         // Trace
    static_cast<ColorWord>(ConsoleForegroundColor::GREEN),        // Info
    static_cast<ColorWord>(ConsoleForegroundColor::LIGHT_YELLOW), // Warn
    static_cast<ColorWord>(ConsoleForegroundColor::RED),          // Error
    static_cast<ColorWord>(ConsoleForegroundColor::BLACK)         // Fatal (with red bg)
};

constexpr std::array<ColorWord, 5> c_BackgroundLookup = {
    static_cast<ColorWord>(ConsoleBackgroundColor::BLACK), // Trace (unused, see SetColor)
    static_cast<ColorWord>(ConsoleBackgroundColor::BLACK), // Info
    static_cast<ColorWord>(ConsoleBackgroundColor::BLACK), // Warn
    static_cast<ColorWord>(ConsoleBackgroundColor::BLACK), // Error
    static_cast<ColorWord>(ConsoleBackgroundColor::RED)    // Fatal
};

namespace
{
[[nodiscard]] ColorWord GetDefaultConsoleAttributes()
{
    CONSOLE_SCREEN_BUFFER_INFO info{};
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(handle, &info) != 0)
    {
        return static_cast<ColorWord>(info.wAttributes);
    }

    // Fallback: white on black
    return static_cast<ColorWord>(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

const ColorWord g_DefaultAttrs = GetDefaultConsoleAttributes();
const ColorWord g_DefaultBg = static_cast<ColorWord>(g_DefaultAttrs & 0xF0u);
const ColorWord g_DefaultFg = static_cast<ColorWord>(g_DefaultAttrs & 0x0Fu);
} // namespace

#else // AE_WINDOWS

using FgAnsiCode = uint8_t;
using BgAnsiCode = int8_t;

enum class ConsoleForegroundColor : FgAnsiCode
{
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    LIGHT_GRAY = 37,
    GRAY = 90,
    LIGHT_RED = 91,
    LIGHT_GREEN = 92,
    LIGHT_YELLOW = 93,
    LIGHT_BLUE = 94,
    LIGHT_MAGENTA = 95,
    LIGHT_CYAN = 96,
    WHITE = 97
};

enum class ConsoleBackgroundColor : BgAnsiCode
{
    DEFAULT = 49, // reset to terminal default background
    BLACK = 40,
    RED = 41,
    GREEN = 42,
    YELLOW = 43,
    BLUE = 44,
    MAGENTA = 45,
    CYAN = 46,
    LIGHT_GRAY = 47,
    GRAY = 100,
    LIGHT_RED = 101,
    LIGHT_GREEN = 102,
    LIGHT_YELLOW = 103,
    LIGHT_BLUE = 104,
    LIGHT_MAGENTA = 105,
    LIGHT_CYAN = 106,
    WHITE = 107
};

constexpr std::array<FgAnsiCode, 5> c_ForegroundLookup = {
    static_cast<FgAnsiCode>(ConsoleForegroundColor::GRAY),         // Trace
    static_cast<FgAnsiCode>(ConsoleForegroundColor::GREEN),        // Info
    static_cast<FgAnsiCode>(ConsoleForegroundColor::LIGHT_YELLOW), // Warn
    static_cast<FgAnsiCode>(ConsoleForegroundColor::RED),          // Error
    static_cast<FgAnsiCode>(ConsoleForegroundColor::BLACK)         // Fatal (with red bg)
};

constexpr std::array<BgAnsiCode, 5> c_BackgroundLookup = {
    static_cast<BgAnsiCode>(ConsoleBackgroundColor::DEFAULT), // Trace
    static_cast<BgAnsiCode>(ConsoleBackgroundColor::DEFAULT), // Info
    static_cast<BgAnsiCode>(ConsoleBackgroundColor::DEFAULT), // Warn
    static_cast<BgAnsiCode>(ConsoleBackgroundColor::DEFAULT), // Error
    static_cast<BgAnsiCode>(ConsoleBackgroundColor::RED)      // Fatal
};

#endif // AE_WINDOWS

ae::Console::Console()
#ifdef AE_WINDOWS
    : m_ForegroundColor(static_cast<ConsoleColorCode>(g_DefaultFg)),
      m_BackgroundColor(static_cast<ConsoleColorCode>(g_DefaultBg))
#else
    : m_ForegroundColor(static_cast<ConsoleColorCode>(static_cast<int>(ConsoleForegroundColor::WHITE))),
      m_BackgroundColor(static_cast<ConsoleColorCode>(static_cast<int>(ConsoleBackgroundColor::DEFAULT)))
#endif
{
}

ae::Console::~Console()
{
#ifdef AE_WINDOWS
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, static_cast<WORD>(g_DefaultAttrs));
#else
    if (isatty(STDOUT_FILENO) != 0)
    {
        std::fputs("\x1b[0m", stdout);
        std::fflush(stdout);
    }
#endif
}

void ae::Console::SetColor(LogLevel level)
{
    const auto idx = static_cast<size_t>(level);

#ifdef AE_WINDOWS
    const ColorWord fg = c_ForegroundLookup[idx];

    ColorWord bg = g_DefaultBg;
    if (level == LogLevel::Fatal)
    {
        bg = c_BackgroundLookup[idx]; // red background for fatal
    }

    m_ForegroundColor = static_cast<ConsoleColorCode>(fg);
    m_BackgroundColor = static_cast<ConsoleColorCode>(bg);
#else
    m_ForegroundColor = static_cast<ConsoleColorCode>(static_cast<int>(c_ForegroundLookup[idx]));
    m_BackgroundColor = static_cast<ConsoleColorCode>(static_cast<int>(c_BackgroundLookup[idx]));
#endif

    Update();
}

void ae::Console::Update() const
{
#ifdef AE_WINDOWS
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

    const auto fg = static_cast<ColorWord>(m_ForegroundColor);
    const auto bg = static_cast<ColorWord>(m_BackgroundColor);
    const ColorWord attrs = static_cast<ColorWord>(fg | bg);

    SetConsoleTextAttribute(handle, static_cast<WORD>(attrs));
#else
    if (isatty(STDOUT_FILENO) == 0)
    {
        return;
    }

    const int fg = static_cast<int>(m_ForegroundColor);
    const int bg = static_cast<int>(m_BackgroundColor);

    std::print(stdout, "\x1b[{};{}m", bg, fg);
    std::fflush(stdout);
#endif
}
