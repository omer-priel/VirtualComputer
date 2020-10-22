#include "pch.h"
#include "Commands.h"

#include <unordered_map>
#include <string_view>

#include "HelpBody.h"
#include "Drive.h"
#include "Directory.h"
#include "File.h"

namespace VirtualComputer::User
{
    static char s_StartLine[MAX_COMMAND_SIZE - 2];

    static struct CurrentDirectory
    {
        Directory* directory = nullptr;
        std::vector<unsigned int> path;

        void Change(VirtualComputer::Drive* drive)
        {
            Drive::s_DriveCurrent = drive;
            path.clear();
            directory = nullptr;
        }
        
        bool IsDrive() const
        {
            return directory == nullptr;
        }

        bool IsDirectory() const
        {
            return directory != nullptr;
        }

        DirectoryBody* GetBody() const
        {
            if (directory == nullptr)
            {
                return Drive::s_DriveCurrent;
            }
            else
            {
                return directory;
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
    // Utils functions
    static bool GetDirectory(std::string& path, Drive*& drive, unsigned int& chankIndex)
    {
        drive = nullptr;
        chankIndex = 0;

        int i = 0;

        // Get Drive
        if (path.size() >= 2)
        {
            if (path[1] == ':')
            {
                if (path.size() == 2 || path[2] == '/' || path[2] == '\\')
                {
                    if ('a' <= path[0] && path[0] <= 'z')
                    {
                        drive = Drive::s_Drives[path[0] - 'a'];
                    }
                    else if ('A' <= path[0] && path[0] <= 'Z')
                    {
                        drive = Drive::s_Drives[path[0] - 'A'];
                    }

                    if (drive == nullptr)
                    {
                        return false;
                    }
                    
                    if (path.size() == 2)
                    {
                        return true;
                    }
                    else
                    {
                        i = 3;
                    }
                }
                else
                {
                    return false;
                }
            }
        }

        std::string_view pathView(path);
        std::vector<unsigned int> pathInChanks;

        if (drive == nullptr) // Start from current directory
        {
            drive = Drive::s_DriveCurrent;
            
            for (auto item : User::s_CurrentDirectory.path)
            {
                pathInChanks.push_back(item);
            }

            chankIndex = User::s_CurrentDirectory.GetBody()->m_ChankIndex;
        }

        unsigned int startNameIndex = i;
        while (i < path.size())
        {
            char tv = path[i];
            if ((path[i] == '/' || path[i] == '\\') || (i == path.size() - 1))
            {
                std::string_view name;
                if (path[i] == '/' || path[i] == '\\')
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex);

                    if (name.empty()) // can't be empty
                    {
                        return false;
                    }
                }
                else // last name
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                    if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                    {
                        return false;
                    }
                }

                if (!name.compare("."))
                {
                    // Do nothing
                }
                else if (!name.compare(".."))
                {
                    // Back
                    if (pathInChanks.empty())
                    {
                        return false;
                    }
                    else
                    {
                        pathInChanks.pop_back();
                        if (pathInChanks.empty())
                        {
                            chankIndex = 0;
                        }
                        else
                        {
                            chankIndex = pathInChanks[pathInChanks.size() - 1];
                        }
                    }
                }
                else
                {
                    // into sub directory
                    if (chankIndex == 0)
                    {
                        drive->GoToChank(chankIndex);
                    }
                    else
                    {
                        drive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                    }

                    unsigned char count = drive->m_FileStream.Read<unsigned char>();

                    bool found = false;
                    for (unsigned char j = 0; j < count; j++)
                    {
                        chankIndex = drive->m_FileStream.Read<unsigned int>();
                        size_t index = drive->m_FileStream.GetIndex();
                        drive->GoToChank(chankIndex);

                        char* checkName = SmartEntityName().GetName(drive->m_FileStream);
                        found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                        if (found)
                        {
                            break;
                        }

                        drive->m_FileStream.ChangeIndex(index);
                    }

                    if (found)
                    {
                        pathInChanks.push_back(chankIndex);
                    }
                    else
                    {
                        return false;
                    }
                }

                i++;
                startNameIndex = i;
            }
            else if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n')
            {
                return false;
            }
            else
            {
                i++;
            }
        }

        return true;
    }

    // help
    static void Help()
    {
        std::cout << HelpBody();
    }

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
            DirectoryBody* directory = nullptr;
            Drive* drive = Drive::s_DriveCurrent;

            if (commandParts.size() == 1)
            {
                directory = User::s_CurrentDirectory.GetBody();
            }
            else
            {
                unsigned int chankIndex;

                if (GetDirectory(commandParts[1], drive, chankIndex))
                {
                    if (chankIndex == 0)
                    {
                        directory = drive;
                    }
                    else
                    {
                        directory = new Directory(chankIndex, drive);
                    }
                }
                else
                {
                    std::cout << "The directory \"" << commandParts[1] << "\" not found.\n";
                    return;
                }
            }

            std::string directories[MAX_DIRECTORIES];
            for (size_t i = 0; i < directory->m_DirectoriesCount; i++)
            {
                directories[i] = directory->m_DirectoriesNames[i].GetName();
            }

            std::string files[MAX_FILES];
            for (size_t i = 0; i < directory->m_FilesCount; i++)
            {
                files[i] = directory->m_FilesNames[i].GetName();
            }

            std::sort(directories, directories + directory->m_DirectoriesCount);
            std::sort(files, files + directory->m_FilesCount);

            int directoriesIndex = 0;
            int filesIndex = 0;

            while (directoriesIndex < directory->m_DirectoriesCount && filesIndex < directory->m_FilesCount)
            {
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

            while (directoriesIndex < directory->m_DirectoriesCount)
            {
                std::cout << "\t<DIR> \t" << directories[directoriesIndex] << "\n";
                directoriesIndex++;
            }
            
            while (directoriesIndex < directory->m_DirectoriesCount)
            {
                std::cout << "\t<FILE>\t" << files[filesIndex] << "\n";
                filesIndex++;
            }
        }
        else
        {
            HelpDir();
        }
    }

    static bool DoCommand(std::string& command, std::vector<std::string>& commandParts)
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
            Drive* drive = Drive::s_DriveCurrent;

            drive->CreateDirectory(User::CreateName("a c"));
            drive->CreateFile(User::CreateName("a c.txt"), 0);

            drive->CreateDirectory(User::CreateName("bcc"));
            drive->CreateFile(User::CreateName("bcc.txt"), 0);
            
            unsigned int chankIndex = drive->CreateDirectory(User::CreateName("abc"));
            drive->CreateFile(User::CreateName("abc.bat"), 0);

            drive->CreateDirectory(User::CreateName("b b1"));
            drive->CreateFile(User::CreateName("b b1.zip"), 0);

            Directory* directory = new Directory(chankIndex, drive);
            directory->CreateDirectory(User::CreateName("a c"));
            directory->CreateFile(User::CreateName("a c.txt"), 0);

            directory->CreateDirectory(User::CreateName("bcc"));
            directory->CreateFile(User::CreateName("bcc.txt"), 0);

            directory->CreateDirectory(User::CreateName("abc"));
            directory->CreateFile(User::CreateName("abc.bat"), 0);

            directory->CreateDirectory(User::CreateName("b b1"));
            directory->CreateFile(User::CreateName("b b1.zip"), 0);
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

    void Close()
    {
        delete User::s_CurrentDirectory.directory;
    }
};