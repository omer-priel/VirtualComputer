#pragma once

namespace Utils::Debug
{
	class Logger
	{
	public:
		enum class Level
		{
			Error = 0,
			Warning = 1,
			Info = 2
		};

	// Static
	private:
		static Level s_LoggerLevel;

	public:

		static void ChangeLevel(const Level& level);

		static void Error(const char* message);
		static void Warning(const char* message);
		static void Info(const char* message);

		static void Debug(const char* message);
	};
}