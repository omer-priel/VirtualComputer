#include "pch.h"
#include "DebugTimer.h"

namespace Utils::Debug
{
    File DebugTrace::s_File;
    bool DebugTrace::s_FirstEvent;

	// Static
    void DebugTrace::BeginSession(const char* filepath)
    {
#if PROFILING 1
        if (s_File.IsOpened())
        {
            s_File.Close();
        }
        s_File.Open(filepath, true); // Open / Create the file

        Utils::File::Resize(filepath, 1); // Clean the file

        // json header
        s_File << "{\"otherData\": {},\"traceEvents\":[";

        s_FirstEvent = true;
#endif
    }

    void DebugTrace::EndSession()
    {
#if PROFILING 1
        // json footer
        s_File << "]}";
        s_File.Close();
#endif
    }

    void DebugTrace::AddEvent(std::string name, uint32_t threadID, long long start, long long end)
    {
#if PROFILING 1
        if (s_File.IsOpened())
        {
            if (s_FirstEvent)
            {
                s_FirstEvent = false;
            }
            else
            {
                s_File << ",";
            }

            std::replace(name.begin(), name.end(), '"', '\'');

            s_File << "{";
            s_File << "\"cat\":\"function\",";
            s_File << "\"dur\":" + std::to_string(end - start) + ',';
            s_File << "\"name\":\"" + name + "\",";
            s_File << "\"ph\":\"X\",";
            s_File << "\"pid\":0,";
            s_File << "\"tid\":" + std::to_string(threadID) + ",";
            s_File << "\"ts\":" + std::to_string(start);
            s_File << "}";
        }
#endif
    }

    void DebugTimer::Stop()
    {
        if (!m_Stopped)
        {
            // Timer End
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

#if UTILS_DEBUG_TIMER_SHOW_IN_CONSOLE
            long long dur = (end - start) / 10;
            double ms = (double)dur / 100;
            std::cout << "DebugTimer \"" << m_Name << "\" was " << ms << " ms.\n";
#endif

            DebugTrace::AddEvent(m_Name, m_ThreadID, start, end);

            m_Stopped = true;
        }
    }

}