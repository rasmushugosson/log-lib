#include "general/pch.h"

#include "Log.h"

#include <filesystem>
#include <print>

constexpr static std::array<std::string_view, 5> c_LevelLookup = { "TRACE", "INFO", "WARNING", "ERROR", "FATAL" };

ae::Logger ae::Logger::m_Instance;

ae::Logger::Logger()
    : m_OpenMessage(c_LogLibVersion), m_StartPoint(DateTime::SteadyNow()), m_StartDate(DateTime::DateAsString()),
      m_StartTime(DateTime::TimeAsString())
{
    m_ExecutionTimer.Start();
}

ae::Logger::~Logger() noexcept
{
    try
    {
        Close();
    }

    catch (...)
    {
        std::fputs("Unknown exception thrown while closing Logger\n", stderr);
    }
}

void ae::Logger::AddConsoleSink(const std::string &name, LogSinkConsoleKind type, LogLevel minLevel, LogLevel maxLevel)
{
#ifdef AE_DIST
    std::println("WARNING: Attempted to add a console sink to Logger. This was skipped since log system removes all "
                 "logs from dist builds, making the action redundant");
    return;
#endif // AE_DIST
    FILE *stream = nullptr;

    switch (type)
    {
    case LogSinkConsoleKind::STDOUT:
        stream = stdout;
        PrintOpenMessage(stream);
        break;
    case LogSinkConsoleKind::STDERR:
        stream = stderr;
        break;
    default:
        stream = stdout;
    };

    m_Streams.insert(std::make_pair(name, stream));

    m_Sinks.insert(std::make_pair(name,
                                  [stream, minLevel, maxLevel](const LogMessage &message)
                                  {
                                      if (message.level >= minLevel && message.level <= maxLevel)
                                      {
                                          Console::GetInstance().SetColor(message.level);

                                          if (message.level >= LogLevel::ERROR)
                                          {
                                              std::println(stream, "\n{} [{}] {}:{} - {}\n", DateTime::TimeAsString(),
                                                           c_LevelLookup[static_cast<uint32_t>(message.level)].data(),
                                                           message.file.data(), message.line, message.message);
                                          }

                                          else
                                          {
                                              std::println(stream, "{} [{}] {}:{} - {}", DateTime::TimeAsString(),
                                                           c_LevelLookup[static_cast<uint32_t>(message.level)].data(),
                                                           message.file.data(), message.line, message.message);
                                          }
                                      }
                                  }));
}

void ae::Logger::AddFileSink(const std::string &name, const std::string &path, LogLevel minLevel, LogLevel maxLevel)
{
#ifdef AE_DIST
    std::println("WARNING: Attempted to add a file sink to Logger. This was skipped since log system removes all "
                 "logs from dist builds, making the action redundant");
    return;
#endif // AE_DIST
    std::filesystem::path p = path;
    auto parent = p.parent_path();

    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);

        if (ec)
        {
            AE_THROW_FILESYSTEM_ERROR("Failed to create directories for log sink '{}'. Path: '{}'. Error: {}", name,
                                      path, ec.message());
        }
    }

    FILE *stream = nullptr;

#ifdef AE_WINDOWS
    errno_t res = fopen_s(&stream, p.string().c_str(), "w");

    if (res != 0 || stream == nullptr)
    {
        AE_THROW_FILE_OPEN_ERROR("Failed to open log sink '{}' at '{}'. Error code: {}", name, p.string(), res);
    }
#else
    stream = std::fopen(p.string().c_str(), "w");

    if (!stream)
    {
        AE_THROW_FILE_OPEN_ERROR("Failed to open log sink '{}' at '{}'", name, p.string());
    }
#endif

    PrintOpenMessage(stream);

    m_FileStreams.insert(std::make_pair(name, stream));
    m_Streams.insert(std::make_pair(name, stream));

    m_Sinks.insert(std::make_pair(name,
                                  [stream, minLevel, maxLevel](const LogMessage &message)
                                  {
                                      if (message.level >= minLevel && message.level <= maxLevel)
                                      {
                                          std::println(stream, "{} [{}] | {}:{} - {}", DateTime::TimeAsString(),
                                                       c_LevelLookup[static_cast<uint32_t>(message.level)].data(),
                                                       message.file.data(), message.line, message.message);
                                      }
                                  }));
}

void ae::Logger::RemoveSink(const std::string &name)
{
    auto it = m_Sinks.find(name);

    if (it != m_Sinks.end())
    {
        auto streamIt = m_Streams.find(name);
        m_Streams.erase(streamIt);

        auto fileIt = m_FileStreams.find(name);

        if (fileIt != m_FileStreams.end())
        {
            m_FileStreams.erase(fileIt);
        }

        m_Sinks.erase(it);
    }

    else
    {
        AE_LOG_WARNING("Tried to remove sink with name '{}' but it does not exist", name);
    }
}

void ae::Logger::Close()
{
    try
    {
        m_ExecutionTimer.Stop();

        /*
        for (auto& [name, sink] : m_Sinks)
        {
                // Cleanup if necessary
        }
        */

        m_Sinks.clear();

        for (auto &[name, stream] : m_Streams)
        {
            PrintTerminationMessage(stream);
        }

        for (auto &[name, stream] : m_FileStreams)
        {
            std::fclose(stream);
        }

        m_FileStreams.clear();
    }

    catch (std::exception &e)
    {
        std::println("Unexpected exception thrown when closing Logger\n");
        std::println("Message: {}", e.what());
    }
}

void ae::Logger::PrintOpenMessage(FILE *stream) const
{
    std::println(stream, "{}", m_OpenMessage);

    std::println(stream, "\nExecution started at:\n{} {}", m_StartDate, m_StartTime);

    std::println(stream, "\nSink opened at:\n{} {}", DateTime::DateAsString(), DateTime::TimeAsString());

    if (std::expected<std::string, TimeZoneError> tz = DateTime::TimeZoneAsString())
    {
        std::println(stream, "\nTime zone: {}\n", *tz);
    }

    else
    {
        std::println(stream, "\nUnknown time zone ({})\n", to_string(tz.error()));
    }
}

void ae::Logger::PrintCloseMessage(FILE *stream) const
{
    std::println(stream, "\nSink closed at:\n{} {}", DateTime::DateAsString(), DateTime::TimeAsString());
}

void ae::Logger::PrintTerminationMessage(FILE *stream) const
{
    std::println(stream, "\nClosed by termination at:\n{} {}", DateTime::DateAsString(), DateTime::TimeAsString());

    std::println(stream, "\nExecution time: {} s", m_ExecutionTimer.GetElapsedTimeAsString(3));
}
