#include "pch.h"
#include "Logger.h"

namespace Utils::Debug
{
	// Static
#ifdef _DEBUG
	Logger::Level Logger::s_LoggerLevel = Logger::Level::Info;
#endif

	void Logger::ChangeLevel(const Logger::Level& level)
	{
#ifdef _DEBUG
		s_LoggerLevel = level;
#endif
	}

	void Logger::Error(const char* message)
	{
#ifdef _DEBUG
		std::cout << "[ERROR]: " << message << "\n";
#endif
	}

	void Logger::Warning(const char* message)
	{
#ifdef _DEBUG
		if (s_LoggerLevel >= Level::Warning)
		{
			std::cout << "[WARNING]: " << message << "\n";
		}
#endif
	}

	void Logger::Info(const char* message)
	{
#ifdef _DEBUG
		if (s_LoggerLevel >= Level::Info)
		{
			std::cout << "[Info]: " << message << "\n";
		}
#endif
	}

	void Logger::Debug(const char* message)
	{
#ifdef _DEBUG
		std::cout << "[Debug]: " << message << "\n";
#endif
	}
}