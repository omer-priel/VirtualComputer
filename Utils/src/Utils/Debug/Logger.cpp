#include "pch.h"
#include "Logger.h"

namespace Utils::Debug
{
	// Static
	void Logger::ChangeLevel(const Logger::Level& level)
	{
		s_LoggerLevel = level;
	}

	void Logger::Error(const char* message)
	{
		std::cout << "[ERROR]: " << message << "\n";
	}

	void Logger::Warning(const char* message)
	{
		if (s_LoggerLevel >= Level::Warning)
		{
			std::cout << "[WARNING]: " << message << "\n";
		}
	}

	void Logger::Info(const char* message)
	{
		if (s_LoggerLevel >= Level::Info)
		{
			std::cout << "[Info]: " << message << "\n";
		}
	}

	void Logger::Debug(const char* message)
	{
		std::cout << "[Debug]: " << message << "\n";
	}

	Logger::Level Logger::s_LoggerLevel = Logger::Level::Info;
}