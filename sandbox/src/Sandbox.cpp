#include "Log.h"

#include <print>

void Demo()
{
    // Author: Rasmus Hugosson
    // Date: 2025-12-06

    // Description: This is a simple example of how to use this library

    // Let's start by adding some sinks where the logs will show up
    // Messages are added to all sinks with matching severity
    // You can add both console and file sinks and adjust what type of logs end up there
    ae::Logger::Get().AddConsoleSink("Console", ae::LogSinkConsoleKind::STDOUT, AE_TRACE,
                                     AE_WARNING); // Trace - warnings will show up here
    ae::Logger::Get().AddConsoleSink("Error console", ae::LogSinkConsoleKind::STDERR,
                                     AE_ERROR); // Only errors and fatal errors will show up here
    ae::Logger::Get().AddFileSink("Error file", "logs/errors.txt",
                                  AE_ERROR); // Only errors and fatal errors will be recorded here

    // Now we can log a simple message with the following macro
    AE_LOG(AE_INFO, "Hello World!");
    // A special macro is provided for blank lines since printing "\n" manually can result in incorrect formatting
    AE_LOG_NEWLINE();

    // There are 5 log levels: Trace, Info, Warning, Error, Fatal
    // The log levels are color coded and a tag is displayed before the message
    AE_LOG(AE_TRACE, "This message is not important");
    AE_LOG(AE_INFO, "This is an infomtion message");
    AE_LOG(AE_WARNING, "This is a warning!");
    AE_LOG(AE_ERROR, "This is an error!");
    AE_LOG(AE_FATAL, "This is a fatal error!");

    AE_LOG_NEWLINE();

    // By default, the log messages are only written in debug mode
    // But this can also be explicetly specified
    // The macros are removed when building for a different target
    AE_LOG(AE_INFO, "This is a debug mode info message");
    AE_LOG_DEBUG(AE_INFO, "This is a debug mode info message");
    AE_LOG_NEWLINE_DEBUG();

    // In release mode, only the release log messages are displayed
    AE_LOG_RELEASE(AE_INFO, "This is a release mode info message");
    AE_LOG_NEWLINE_RELEASE();

    // Messages can also be logged for both debug and release mode
    AE_LOG_BOTH(AE_INFO, "This is a message for both debug and release mode");
    AE_LOG_NEWLINE_BOTH();

    // All log messages can be formatted through std::format, like using normal std::print
    AE_LOG(AE_TRACE, "The answer to life, the universe and everything is {}", 42);
    AE_LOG(AE_TRACE, "{} is the value of pi", 3.14159265359);

    // This library also provides a way to throw exceptions with a message
    try
    {
        // The exception messages are specified in the same way as the log messages
        // Errors are formatted in the same way as normal log messages through std::format
        AE_THROW_MATH_ERROR("Division by zero. {}/{} is not a vaild operation", 1, 0);
    }

    catch (const std::exception &e)
    {
        // The exception can be caught as usual and the message can then be logged
        AE_LOG_BOTH(AE_ERROR, "{}", e.what());
    }

    // Execution time can be measured with the Timer class
    ae::Timer timer;
    timer.Start();

    // We can then make the thread sleep for a while using the DateTime singleton
    ae::DateTime::Wait(1.0);

    // And then measure the elapsed time
    AE_LOG(AE_INFO, "Elapsed time: {} s", timer.GetElapsedTime());

#ifdef AE_DIST
    // No messages are written for distribution builds
    // If something is to be logged in a distribution build, it can be done with standard C++ functions
    std::println("This message is only displayed in distribution builds");
#endif // AE_DIST
}

int main()
{
    try
    {
        Demo();
        return EXIT_SUCCESS;
    }

    catch (...)
    {
        std::fputs("Fatal error: unknown exception\n", stderr);
        return EXIT_FAILURE;
    }
}
