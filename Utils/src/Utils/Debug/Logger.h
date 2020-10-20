#pragma once

#include <cstdarg>
#include <any>

namespace Utils::Debug
{
	template <typename Func>
	static void IterateVariadic(const Func&) {}

	template <typename Func, typename Arg, typename ... Args>
	static void IterateVariadic(const Func& func, Arg&& arg, Args&& ... args)
	{
		func(std::forward<Arg>(arg));
		IterateVariadic(func, std::forward<Args>(args)...);
	}

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
#ifdef _DEBUG
		static Level s_LoggerLevel;
#endif

	public:

		static void ChangeLevel(const Level& level);

		static void Error(const char* message);
		static void Warning(const char* message);
		static void Info(const char* message);
		
		static void Debug(const char* message);

		template<typename... T>
		static void Error(const T&... args)
		{
#ifdef _DEBUG
			std::cout << "[Error]: ";
			IterateVariadic([&](auto& arg)
				{
					std::cout << arg;
				},
				args...);
			std::cout << "\n";
#endif
		}

		template<typename... T>
		static void Warning(const T&... args)
		{
#ifdef _DEBUG
			if (s_LoggerLevel >= Level::Warning)
			{
				std::cout << "[Warning]: ";
				IterateVariadic([&](auto& arg)
					{
						std::cout << arg;
					},
					args...);
				std::cout << "\n";
			}
#endif
		}

		template<typename... T>
		static void Info(const T&... args)
		{
#ifdef _DEBUG
			if (s_LoggerLevel >= Level::Info)
			{
				std::cout << "[Info]: ";
				IterateVariadic([&](auto& arg)
					{
						std::cout << arg;
					},
					args...);
				std::cout << "\n";
			}
#endif
		}
		
		template<typename... T>
		static void Debug(const T&... args)
		{
#ifdef _DEBUG
			std::cout << "[Debug]: ";
			IterateVariadic([&](auto& arg)
				{
					std::cout << arg;
				},
				args...);
			std::cout << "\n";
#endif _DEBUG
		}
	};
}