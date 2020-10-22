#include "pch.h"
#include "Commands.h"

#include <unordered_map>

#include "HelpBody.h"
#include "Drive.h"
#include "Directory.h"
#include "File.h"

namespace VirtualComputer::User
{
    static char s_StartLine[MAX_COMMAND_SIZE - 2];

    static struct CurrentDirectory
    {
        Drive* Drive = nullptr;
        Directory* Directory = nullptr;

        void Change(VirtualComputer::Drive* drive)
        {
            Drive = drive;
            Directory = nullptr;
        }

        void Change(VirtualComputer::Directory* directory)
        {
            Drive = nullptr;
            Directory = directory;
        }
        
        bool IsDrive() const
        {
            return Drive != nullptr;
        }

        bool IsDirectory() const
        {
            return Directory != nullptr;
        }

        DirectoryBody* GetBody() const
        {
            if (IsDrive())
            {
                return Drive;
            }
            else
            {
                return Directory;
            }
        }
    }
    s_CurrentDirectory;

    static void ChangeStartLine(const std::string& line)
    {
        memcpy(s_StartLine, line.c_str(), line.size());
        s_StartLine[line.size()] = 0;
    }

    static void GetCommand(std::string& command)
    {
        std::cout << s_StartLine << "> ";
        std::getline(std::cin, command);
    }

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
}

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

    static void HelpDir()
    {
        std::cout
            << "Displays a list of files and subdirectories in a directory.\n"
            << "\n"
            << "    dir - Displays current directory.\n"
            << "    dir [path] - Displays directory by path.\n"
            << "        path - Path of directory\n";
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

    static void CommandDir(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() <= 2)
        {
            DirectoryBody* directory;
            Drive* drive = Drive::s_DriveCurrent;

            if (commandParts.size() == 1)
            {
                directory = User::s_CurrentDirectory.GetBody();
            }
            else
            {
                std::string_view path = commandParts[1];

                // Get DirectoryBody* by path
                // TODO
                // for Dubeg
                directory = User::s_CurrentDirectory.GetBody();
            }

            std::string directories[MAX_DIRECTORIES];
            for (size_t i = 0; i < MAX_DIRECTORIES; i++)
            {
                directories[i] = directory->m_DirectoriesNames[i].GetName(drive->m_FileStream);
            }

            std::string files[MAX_FILES];
            for (size_t i = 0; i < MAX_FILES; i++)
            {
                files[i] = directory->m_FilesNames[i].GetName(drive->m_FileStream);
            }

            std::sort(directories, directories + MAX_DIRECTORIES);
            std::sort(files, files + MAX_FILES);

            int directoriesIndex = 0;
            int filesIndex = 0;

            do
            {
                while (directoriesIndex < MAX_DIRECTORIES && directories[directoriesIndex].empty())
                {
                    directoriesIndex++;
                }

                while (filesIndex < MAX_DIRECTORIES && files[filesIndex].empty())
                {
                    filesIndex++;
                }

                if (directoriesIndex < MAX_DIRECTORIES && filesIndex < MAX_FILES)
                {
                    std::string a = directories[directoriesIndex];
                    std::string b = files[filesIndex];

                    if (directories[directoriesIndex].compare(files[filesIndex]) < 0)
                    {
                        std::cout << "\t<DIR> \t" << directories[directoriesIndex] << "\n";
                        directoriesIndex++;
                    }
                    else
                    {
                        std::cout << "\t<FILE>\t" << files[filesIndex] << "\n";
                        filesIndex++;
                    }
                }
                else if (directoriesIndex < MAX_DIRECTORIES)
                {
                    std::cout << "\t<DIR> \t" << directories[directoriesIndex] << "\n";
                    directoriesIndex++;
                }
                else if (filesIndex < MAX_FILES)
                {
                    std::cout << "\t<FILE>\t" << files[filesIndex] << "\n";
                    filesIndex++;
                }
                else
                {
                    break;
                }

            } while (true);
        }
        else
        {
            HelpDir();
        }
    }

    // Public Functions
    void Load()
    {
        User::s_CurrentDirectory.Change(Drive::s_DriveCurrent);

        std::string firstLine("A:\\");
        firstLine[0] = Drive::s_DriveCurrent->m_DriveName;
        User::ChangeStartLine(firstLine);
    }

    void Loop()
    {
        bool running = true;
        while (running)
        {
            // Get Command
            std::string command;
            User::GetCommand(command);

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
    }

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
            if (helpMode)
            {
                HelpDir();
            }
            else
            {
                CommandDir(command, commandParts);
            }
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
        else if (!action.compare("clear"))
        {
            if (helpMode)
            {
                std::cout << "Clear the window.\n";
            }
            else
            {
                system("CLS");
            }
        }
        else
        {
            std::cout << "The command \"" << action << "\" not exits!";
            Help();
        }

        return true;
    }

    void Help()
    {
        std::cout << HelpBody();
    }
};