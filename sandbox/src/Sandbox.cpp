#include "Log.h"

int main()
{
	// Author: Rasmus Hugosson
	// Date: 2025-02-05

	// Description: This is a simple example of how to use the logging system

	// Let's start by logging a simple message with the following macro
	AE_LOG_CONSOLE(AE_INFO, "Hello World!");
	AE_LOG_CONSOLE_NEWLINE();

	// There are 5 log levels: Trace, Info, Warning, Error, Fatal
	// The log levels are color coded and a tag is displayed before the message
	AE_LOG_CONSOLE(AE_TRACE, "This message is not important");
	AE_LOG_CONSOLE(AE_INFO, "This is an info message");
	AE_LOG_CONSOLE(AE_WARNING, "This is a warning!");
	AE_LOG_CONSOLE(AE_ERROR, "This is an error!");
	AE_LOG_CONSOLE(AE_FATAL, "This is a fatal error!");

	AE_LOG_CONSOLE_NEWLINE();

	// By default, the log messages are only displayed in the console in debug mode
	// But this can also be explicetly specified
	AE_LOG_CONSOLE_DEBUG(AE_INFO, "This is a debug mode info message");
	AE_LOG_CONSOLE_NEWLINE_DEBUG();

	// In release mode, only the release log messages are displayed
	AE_LOG_CONSOLE_RELEASE(AE_INFO, "This is a release mode info message");
	AE_LOG_CONSOLE_NEWLINE_RELEASE();

	// Messages can also be logged for both debug and release mode
	AE_LOG_CONSOLE_ALL(AE_INFO, "This is a message for both debug and release mode");
	AE_LOG_CONSOLE_NEWLINE_ALL();

	// Any data type with an overloaded << operator for std::ostream can be logged
	AE_LOG_CONSOLE(AE_TRACE, "The answer to life, the universe and everything is " << 42);
	AE_LOG_CONSOLE(AE_TRACE, 3.14159265359 << " is the value of pi");

	// This library also provides a way to throw exceptions with a message
	try
	{
		// The exception messages are specified in the same way as the log messages
		AE_THROW_MATH_ERROR("Division by zero. " << 1 << "/" << 0 << " is not a vaild operation");
	}

	catch (const std::exception& e)
	{
		// The exception can be caught as usual and the message can be logged
		AE_LOG_CONSOLE_ALL(AE_ERROR, e.what());
	}

#ifdef AE_DIST
	// No messages are printed for distribution builds
	// If something is to be logged in a distribution build, it can be done with standard C++ functions
	std::cout << "This message is only displayed in distribution builds" << std::endl;
#endif // AE_DIST
	return 0;
}
