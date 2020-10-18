#include "pch.h"

#include "Drive.h"

EntityName CreateName(const char* name)
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

char s_StartLine[50];

void ChangeStartLine(const char* line, size_t size)
{    
    memcpy(s_StartLine, line, size);
    s_StartLine[size] = ' ';
    s_StartLine[size + 1] = 0;
}

int main()
{
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

    ChangeStartLine("A:\\>", 4);

    // Start Runing
    while (true)
    {
        // Get Command
        std::string command;
        
        std::cout << s_StartLine;
        std::getline(std::cin, command);

        // do
        if (command.empty())
        {
            continue;
        }

        std::vector<std::string_view> commandParts;

        int startPart = 0;
        int i = 0;
        bool inString = false;

        while (i < command.size())
        {
            if (inString)
            {
                if (command[i] == '\"')
                {
                    inString = false;
                }
                i++;
            }
            else
            {
                if (command[i] == '\"')
                {
                    inString = true;
                }
                else if (command[i] == ' ' || command[i] == '\t')
                {
                    if (i == startPart)
                    {
                        i++;
                        startPart++;
                        continue;
                    }
                    else
                    {
                        commandParts.emplace_back(command.c_str() + startPart, i - startPart - 1);
                        
                        i++;
                        startPart = i;
                        continue;
                    }
                }
                i++;
            }
        }

        std::string_view action(command.c_str(), 1);
    }

    Utils::Debug::DebugTrace::EndSession();
    system("PAUSE");
}