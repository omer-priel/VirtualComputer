#include "pch.h"

#include "Drive.h"
#include "Commands.h"

namespace VirtualComputer
{
    static EntityName CreateName(const char* name)
    {
        EntityName ret;
        ret.fill(0);
        for (size_t i = 0; i < MAX_ENTITY_NAME; i++)
        {
            ret[i] = name[i];
            if (name[i] == 0)
                break;
        }

        ret[MAX_ENTITY_NAME] = 0;
        return ret;
    }

    static char s_StartLine[MAX_COMMAND_SIZE];

    static void ChangeStartLine(const char* line, size_t size)
    {
        memcpy(s_StartLine, line, size);
        s_StartLine[size] = ' ';
        s_StartLine[size + 1] = 0;
    }
}

    int main()
    {
        using namespace VirtualComputer;

        Utils::Debug::DebugTrace::BeginSession();
        Logger::ChangeLevel(Logger::Level::Warning);

        // Load Compuer

        // Load Drives
        bool haveError = false;
        Drive::LoadDrives(haveError);

        if (haveError)
        {
            std::cin.get();
            return 1;
        }

        //load StartLine
        ChangeStartLine("A:\\>", 4);

        // Start Runing
        bool running = true;
        while (running)
        {
            // Get Command
            std::string command;

            std::cout << s_StartLine;
            std::getline(std::cin, command);

            // do
            if (command.size() > MAX_COMMAND_SIZE)
            {
                Logger::Error("The cammand can't be bigger then ", MAX_COMMAND_SIZE, "!");
                continue;
            }

            std::vector<std::string> commandParts;

            char commandPart[MAX_COMMAND_SIZE];
            int partIndex = 0;
            bool inString = false;

            for (char tv : command)
            {
                if (inString)
                {
                    if (tv == '\"')
                    {
                        inString = false;
                    }
                    else
                    {
                        commandPart[partIndex] = tv;
                        partIndex++;
                    }
                }
                else
                {
                    if (tv == '\"')
                    {
                        inString = true;
                    }
                    else if (tv == ' ' || tv == '\t')
                    {
                        if (partIndex == 0)
                        {
                            continue;
                        }
                        else
                        {
                            commandPart[partIndex] = 0;
                            commandParts.emplace_back(commandPart);

                            partIndex = 0;
                            continue;
                        }
                    }
                    else
                    {
                        commandPart[partIndex] = tv;
                        partIndex++;
                    }
                }
            }

            if (inString)
            {
                Logger::Error("Command syntex error!");
                continue;
            }

            if (partIndex != 0)
            {
                commandPart[partIndex] = 0;
                commandParts.emplace_back(commandPart);
            }

            if (commandParts.empty())
            {
                continue;
            }

            // make command lower ("ECHO" to "echo")
            for (auto& tv : commandParts[0])
            {
                tv = std::tolower(tv);
            }

            running = Commands::DoCommand(command, commandParts);
        }

        Utils::Debug::DebugTrace::EndSession();
    }