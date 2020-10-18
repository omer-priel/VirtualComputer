#include "pch.h"
#include "Commands.h"

#include <unordered_map>

#include "HelpBody.h"
#include "Drive.h"

namespace Commands
{
    bool DoCommand(std::string& command, std::vector<std::string>& commandParts)
    {
        std::string& action = commandParts[0];
        if (!action.compare("exit"))
        {
            return false;
        }
        else if (!action.compare("help"))
        {
            Help();
        }
        else if (!action.compare("drives"))
        {
            std::cout << "Drives exists:\n";
            for (size_t i = 0; i < Drive::s_DrivesActives; i++)
            {
                std::cout << "  " << Drive::s_Drives[i]->m_DriveName << ":\n";
            }
            std::cout << "\n";
        }
        else if (!action.compare("dir"))
        {

        }
        else if (!action.compare("md"))
        {

        }
        else if (!action.compare("rd"))
        {

        }
        else if (!action.compare("cd"))
        {

        }
        else if (!action.compare("mf"))
        {

        }
        else if (!action.compare("rf"))
        {

        }
        else if (!action.compare("move"))
        {

        }
        else if (!action.compare("copy"))
        {

        }
        else if (!action.compare("rename"))
        {

        }
        else if (!action.compare("print"))
        {

        }
        else if (!action.compare("edit"))
        {

        }
        else if (!action.compare("echo"))
        {
            int i = command.find("echo") + 5;

            if (i < command.size())
            {
                std::cout << (char*)(command.c_str() + i) << "\n"; \
            }
        }
        else
        {
            std::cout << "The command \"" << action << "\" not exits!\n";
            Help();
        }
        
        return true;
    }

    void Help()
    {
        std::cout << HelpBody();
    }
};