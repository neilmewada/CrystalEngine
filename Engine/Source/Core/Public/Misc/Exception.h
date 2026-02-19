#pragma once

#include "CoreTypes.h"

#include <exception>
#include "cpptrace/cpptrace.hpp"

namespace CE
{

    class CORE_API Exception : public std::exception
    {
    public:
        Exception() : message("Unknown error"), stackTrace(cpptrace::generate_trace())
        {
	        
        }

        Exception(const String& message) : message(message), stackTrace(cpptrace::generate_trace())
        {
	        
        }

        const char* what() const throw () 
    	{
            return message.GetCString();
        }

        const cpptrace::stacktrace& GetStackTrace() const
        {
            return stackTrace;
		}

        String GetStackTraceString(bool useColors) const
        {
            return std::move(String(stackTrace.to_string(useColors)));
		}

    private:
        String message{};

		cpptrace::stacktrace stackTrace;
    };

    class CORE_API ParseFailedException : public Exception
    {
    public:
        ParseFailedException() : Exception("Failed to parse given input")
        {}

        ParseFailedException(const String& message) : Exception(message)
        {}
    };

    class CORE_API NullPointerException : public Exception
    {
    public:
        NullPointerException() : Exception("NullPointerException")
        {}

        NullPointerException(const String& message) : Exception("NullPointerException: " + message)
        {}

    };

    class CORE_API IndexOutOfRangeException : public Exception
    {
    public:

        IndexOutOfRangeException() : Exception("IndexOutOfRangeException")
        {}

        IndexOutOfRangeException(const String& message) : Exception("IndexOutOfRangeException: " + message)
        {}
    };
    
} // namespace CE
