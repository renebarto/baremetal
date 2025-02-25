//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Logger.cpp
//
// Namespace   : baremetal
//
// Class       : Logger
//
// Description : Basic logging to a device
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#include <baremetal/Logger.h>

#include <stdlib/Util.h>
#include <baremetal/Console.h>
#include <baremetal/Format.h>
#include <baremetal/System.h>
#include <baremetal/Timer.h>
#include <baremetal/Version.h>

/// @file
/// Logger functionality implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("Logger");

/// <summary>
/// Construct a logger
/// </summary>
/// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
/// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
/// <param name="console">Console to print to, defaults to the singleton console instance</param>
Logger::Logger(LogSeverity logLevel, Timer *timer /*= nullptr*/, Console &console /*= GetConsole()*/)
    : m_isInitialized{}
    , m_timer{timer}
    , m_console{console}
    , m_level{logLevel}
{
}

/// <summary>
/// Initialize logger
/// </summary>
/// <returns>true on succes, false on failure</returns>
bool Logger::Initialize()
{
    if (m_isInitialized)
        return true;
    SetupVersion();
    m_isInitialized = true; // Stop reentrant calls from happening
    LOG_INFO(BAREMETAL_NAME " %s started on %s %d (AArch64)", BAREMETAL_VERSION_STRING, "Raspberry Pi", BAREMETAL_RPI_TARGET);

    return true;
}

/// <summary>
/// Set maximum log level (minimum log priority). Any log statements with a value below this level will be ignored
/// </summary>
/// <param name="logLevel">Maximum log level</param>
void Logger::SetLogLevel(LogSeverity logLevel)
{
    m_level = logLevel;
}

/// <summary>
/// Write a string with variable arguments to the logger
/// </summary>
/// <param name="source">Source name or file name</param>
/// <param name="line">Source line number</param>
/// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
/// <param name="message">Formatted message string, with variable arguments</param>
void Logger::Write(const char *source, int line, LogSeverity severity, const char *message, ...)
{
    va_list var;
    va_start(var, message);
    WriteV(source, line, severity, message, var);
    va_end(var);
}

/// <summary>
/// Write a string with variable arguments to the logger
/// </summary>
/// <param name="source">Source name or file name</param>
/// <param name="line">Source line number</param>
/// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
/// <param name="message">Formatted message string</param>
/// <param name="args">Variable argument list</param>
void Logger::WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args)
{
    if (static_cast<int>(severity) > static_cast<int>(m_level))
        return;

    static const size_t BufferSize = 1024;
    char buffer[BufferSize]{};

    char sourceString[BufferSize]{};
    Format(sourceString, BufferSize, " (%s:%d)", source, line);

    char messageBuffer[BufferSize]{};
    FormatV(messageBuffer, BufferSize, message, args);

    switch (severity)
    {
    case LogSeverity::Panic:
        strncat(buffer, "!Panic!", BufferSize);
        break;
    case LogSeverity::Error:
        strncat(buffer, "Error  ", BufferSize);
        break;
    case LogSeverity::Warning:
        strncat(buffer, "Warning", BufferSize);
        break;
    case LogSeverity::Info:
        strncat(buffer, "Info   ", BufferSize);
        break;
    case LogSeverity::Debug:
        strncat(buffer, "Debug  ", BufferSize);
        break;
    }

    if (m_timer != nullptr)
    {
        const size_t TimeBufferSize = 32;
        char timeBuffer[TimeBufferSize]{};
        m_timer->GetTimeString(timeBuffer, TimeBufferSize);
        if (strlen(timeBuffer) > 0)
        {
            strncat(buffer, timeBuffer, BufferSize);
            strncat(buffer, " ", BufferSize);
        }
    }

    strncat(buffer, messageBuffer, BufferSize);
    strncat(buffer, sourceString, BufferSize);
    strncat(buffer, "\n", BufferSize);

#if BAREMETAL_COLOR_OUTPUT
    switch (severity)
    {
    case LogSeverity::Panic:
        m_console.Write(buffer, ConsoleColor::BrightRed);
        break;
    case LogSeverity::Error:
        m_console.Write(buffer, ConsoleColor::Red);
        break;
    case LogSeverity::Warning:
        m_console.Write(buffer, ConsoleColor::BrightYellow);
        break;
    case LogSeverity::Info:
        m_console.Write(buffer, ConsoleColor::Cyan);
        break;
    case LogSeverity::Debug:
        m_console.Write(buffer, ConsoleColor::Yellow);
        break;
    default:
        m_console.Write(buffer, ConsoleColor::White);
        break;
    }
#else
    m_console.Write(buffer);
#endif

    if (severity == LogSeverity::Panic)
    {
        GetSystem().Halt();
    }
}

/// <summary>
/// Construct the singleton logger and initializat it if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton logger instance</returns>
Logger &baremetal::GetLogger()
{
    static LogSeverity defaultSeverity{LogSeverity::Debug};
    static Logger      logger(defaultSeverity, &GetTimer());
    logger.Initialize();
    return logger;
}
