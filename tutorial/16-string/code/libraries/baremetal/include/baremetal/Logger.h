//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Logger.h
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

#pragma once

#include "baremetal/Console.h"
#include "stdlib/StdArg.h"
#include "stdlib/Types.h"

/// @file
/// Logger functionality

namespace baremetal {

/// <summary>
/// Logging severity classes
/// </summary>
enum class LogSeverity
{
    /// @brief Halt the system after processing this message
    Panic,
    /// @brief Severe error in this component, system may continue to work
    Error,
    /// @brief Non-severe problem, component continues to work
    Warning,
    /// @brief Informative message, which is interesting for the system user
    Info,
    /// @brief Message, which is only interesting for debugging this component
    Debug
};

class Timer;

/// <summary>
/// Logger class
/// </summary>
class Logger
{
    /// <summary>
    /// Construct the singleton Logger instance if needed, and return a reference to the instance. This is a friend function of class Logger
    /// </summary>
    /// <returns>Reference to the singleton logger instance</returns>
    friend Logger& GetLogger();

private:
    /// @brief True if class is already initialized
    bool m_isInitialized;
    /// @brief Pointer to timer instance
    Timer* m_timer;
    /// @brief Currently set logging severity level
    LogSeverity m_level;
    /// @brief Singleton console instance
    static Console s_console;

    explicit Logger(LogSeverity logLevel, Timer* timer = nullptr);

public:
    static bool HaveLogger();

    bool Initialize();
    void SetLogLevel(LogSeverity logLevel);

    void Log(const char* from, int line, LogSeverity severity, const char* message, ...);
    void LogV(const char* from, int line, LogSeverity severity, const char* message, va_list args);

    void Trace(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
    void TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
};

Logger& GetLogger();

/// @brief Define the static variable From to the specified name, to support printing a different file specification in LOG_* macros
#define LOG_MODULE(name)         static const char From[] = name

/// @brief Log a panic message
#define LOG_PANIC(...)           GetLogger().Log(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
/// @brief Log an error message
#define LOG_ERROR(...)           GetLogger().Log(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
/// @brief Log a warning message
#define LOG_WARNING(...)         GetLogger().Log(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
/// @brief Log a info message
#define LOG_INFO(...)            GetLogger().Log(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
/// @brief Log a debug message
#define LOG_DEBUG(...)           GetLogger().Log(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)

/// @brief Log a message with specified severity and message string
#define LOG(severity, message)   GetLogger().Log(From, __LINE__, severity, message);

/// @brief Log a warning message
#define TRACE_WARNING(...)       GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
/// @brief Log a info message
#define TRACE_INFO(...)          GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
/// @brief Log a debug message
#define TRACE_DEBUG(...)         GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)

/// @brief Log a message with specified severity and message string
#define TRACE(severity, message) GetLogger().Write(From, __FILE_NAME__, __LINE__, __func__, severity, message);

} // namespace baremetal
