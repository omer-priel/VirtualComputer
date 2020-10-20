#include "pch.h"
#include "Commands.h"

#include <unordered_map>

#include "HelpBody.h"
#include "Drive.h"

namespace VirtualComputer::Commands
{
    // commands help functions
    static void HelpHelp()
    {
        std::cout
            << "Show help information for commands.\n"
            << "\n"
            << "    help [command]\n"
            << "        command - displays help information on that command.\n";
    }
    
    static void HelpDrives()
    {
        std::cout
            << "Mange the Drives.\n"
            << "\n"
            << "    drives [/s | /show] - Show the drives.\n"
            
            << "    drives [/c | /create] - Create new drive.\n"
            << "    drives [/c | /create] [name] - Create new drive.\n"
            << "        name - Drive name\n"

            << "    drives [/d | /delete] [name] - Delete drive.\n"
            << "        name - Drive name\n";
    }

    // commands functions
    static void CommandDrives(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 1 || !commandParts[1].compare("/s") || !commandParts[1].compare("/show")) // drives [/s | /show]
        {
            std::cout << "Drives exists:\n";
            for (Drive* drive : Drive::s_Drives)
            {
                if (drive != nullptr)
                {
                    std::cout << "  " << drive->m_DriveName << ":\n";
                }
            }
            std::cout << "\n";
        }
        else
        {
            if (!commandParts[1].compare("/c") || !commandParts[1].compare("/create")) // drives [/c | /create] [name]
            {
                if (commandParts.size() == 2)
                {
                    Drive* created = Drive::CreateDrive();
                    if (created != nullptr)
                    {
                        std::cout << "The drive " << created->m_DriveName << ": created.\n";
                    }
                }
                else if (commandParts.size() == 3)
                {
                    const char* name = commandParts[2].c_str();
                    Drive* created = Drive::CreateDrive(name);
                    if (created != nullptr)
                    {
                        std::cout << "The drive " << created->m_DriveName << ": created.\n";
                    }
                }
                else
                {
                    HelpDrives();
                }
            }
            else if (!commandParts[1].compare("/d") || !commandParts[1].compare("/delete")) // drives [/d | /delete] [name]
            {
                if (commandParts.size() == 3)
                {
                    const char* name = commandParts[2].c_str();
                    char deleted = Drive::DeleteDrive(name);
                    if (deleted != 0)
                    {
                        std::cout << "The drive " << deleted << ": deleted.\n";
                    }
                }
                else
                {
                    HelpDrives();
                }
            }
            else
            {
                HelpDrives();
            }
        }
    }

    // Do Command
    bool DoCommand(std::string& command, std::vector<std::string>& commandParts)
    {
        std::string_view action(commandParts[0]);
        
        bool helpMode = false;
        if (commandParts.size() > 1)
        {
            if (!commandParts[1].compare("/?")) // [command] /?
            {
                helpMode = true;
            }
            else if (!action.compare("help")) // help [command]
            {
                action = std::string_view(commandParts[1]);
                helpMode = true;
            }   
        }

        if (!action.compare("exit"))
        {
            if (helpMode)
            {
                std::cout << "Quits the Virtual Computer.\n";
            }
            else
            {
                return false;
            }
        }
        else if (!action.compare("help"))
        {
            if (helpMode)
            {
                HelpHelp();
            }
            else
            {
                Help();
            }
        }
        else if (!action.compare("drives"))
        {
            if (helpMode)
            {
                HelpDrives();
            }
            else
            {
                CommandDrives(command, commandParts);
            }
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
            if (helpMode)
            {
                std::cout << "Displays messages.\n";
            }
            else
            {
                int i = command.find("echo") + 5;

                if (i < command.size())
                {
                    std::cout << (char*)(command.c_str() + i) << "\n"; \
                }
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