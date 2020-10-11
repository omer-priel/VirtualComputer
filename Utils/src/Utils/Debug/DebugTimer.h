#pragma once

#include <chrono>
#include <algorithm>
#include <thread>

#include "Utils\File.h"

#define UTILS_DEBUG_TIMER_SHOW_IN_CONSOLE 1
#define PROFILING 0

#if PROFILING 1
#define PROFILE_SCOPE(name) Utils::Debug::DebugTimer debugTimer##__LINE__(name)
#define PROFILE_FUNCTION PROFILE_SCOPE(__FUNCSIG__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION
#endif

namespace Utils::Debug
{
    class DebugTrace
    {
        // Static
    private:
        static File s_File;
        static bool s_FirstEvent;

    public:
        static void BeginSession(const char* filepath = "DebugTrace.json");

        static void EndSession();

        static void AddEvent(std::string name, uint32_t threadID, long long start, long long end);
    };
    
    class DebugTimer
	{
    private:
        std::string m_Name;
        uint32_t m_ThreadID;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        bool m_Stopped;

    public:
        DebugTimer()
            : m_Name(""), m_Stopped(false)
        {
            m_ThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

            // Timer Start
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        explicit DebugTimer(const std::string& name)
            : m_Name(name), m_Stopped(false)
        {
            m_ThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

            // Timer Start
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        virtual ~DebugTimer()
        {
            Stop();
        }

        void Stop();
	};
}