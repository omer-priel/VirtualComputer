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
    static char s_StartLine[MAX_COMMAND_SIZE - 10];

    static struct CurrentDirectory
    {
        Directory* directory = nullptr;
        std::vector<PathItem> path;

        void Change()
        {
            s_StartLine[0] = Drive::s_DriveCurrent->m_DriveName;
            s_StartLine[1] = ':';
            s_StartLine[2] = '\\';
            s_StartLine[3] = 0;
        }

        void Change(Drive* drive, unsigned int chankIndex, std::vector<PathItem>* pathInChanks)
        {            
            if (Drive::s_DriveCurrent != drive || chankIndex != GetBody()->m_ChankIndex)
            {
                Drive::s_DriveCurrent = drive;
                
                if (directory != nullptr)
                {
                    delete directory;
                }
                if (chankIndex == 0)
                {
                    path.clear();
                    directory = nullptr;
                }
                else
                {
                    path.clear();
                    for (PathItem& chankIndex : *pathInChanks)
                    {
                        path.push_back(chankIndex);
                    }
                    directory = new Directory(chankIndex, drive);
                }

                s_StartLine[0] = drive->m_DriveName;
                s_StartLine[1] = ':';
                s_StartLine[2] = '\\';
                size_t i = 3;
                for (auto& item : path)
                {
                    if (i + item.m_Name.size() < MAX_COMMAND_SIZE - 12)
                    {
                        memcpy(s_StartLine + i, &item.m_Name[0], item.m_Name.size());
                        i += item.m_Name.size();
                        s_StartLine[i] = '\\';
                        i++;
                    }
                    else
                    {
                        memcpy(s_StartLine + i, &item.m_Name[0], 10);
                        i += 10;
                        s_StartLine[i] = '\\';
                        break;
                    }
                }
                s_StartLine[i] = 0;
            }
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
    static bool GetDrive(std::string& path, Drive*& drive)
    {
        drive = nullptr;
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
                }

                return drive != nullptr;
            }
        }
        return true;
    }

    static bool GetDirectory(std::string& path, Drive*& drive, unsigned int& chankIndex, std::vector<PathItem>& pathInChanks)
    {
        chankIndex = 0;

        int i = 0;

        // Get Drive
        if (GetDrive(path, drive))
        {
            if (path.size() >= 2 && path[1] == ':')
            {
                i = 3;
            }
        }
        else
        {
            return false;
        }

        if (drive == nullptr) // Start from current directory
        {
            drive = Drive::s_DriveCurrent;
            
            for (PathItem& item : User::s_CurrentDirectory.path)
            {
                pathInChanks.push_back(item);
            }

            chankIndex = User::s_CurrentDirectory.GetBody()->m_ChankIndex;
        }

        std::string_view pathView(path);

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
                            chankIndex = pathInChanks[pathInChanks.size() - 1].m_chankIndex;
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
                            pathInChanks.emplace_back(chankIndex, checkName);
                            break;
                        }

                        drive->m_FileStream.ChangeIndex(index);
                    }

                    if (!found)
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

    static bool GetDirectory(std::string& path, Drive*& drive, unsigned int& chankIndex)
    {
        std::vector<PathItem> pathInChanks;
        return GetDirectory(path, drive, chankIndex, pathInChanks);
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

    static void HelpCd()
    {
        std::cout
            << "Changes current directory.\n"
            << "\n"
            << "    dir [path] - Changes current directory.\n"
            << "        path - Path of directory\n";
    }

    static void HelpMd()
    {
        std::cout
            << "Create new directory.\n"
            << "\n"
            << "    md [path] - Create new directory.\n"
            << "        path - Path of the new directory\n";
    }

    static void HelpRd()
    {
        std::cout
            << "Delete directory.\n"
            << "\n"
            << "    rd [path] - Delete directory.\n"
            << "        path - Path of the directory\n";
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

            if (directory->m_ChankIndex != 0)
            {
                delete directory;
            }
        }
        else
        {
            HelpDir();
        }
    }

    static void CommandCd(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            Drive* drive;
            unsigned int chankIndex;
            std::vector<PathItem> pathInChanks;
            
            if (GetDirectory(commandParts[1], drive, chankIndex, pathInChanks))
            {
                User::s_CurrentDirectory.Change(drive, chankIndex, &pathInChanks);
            }
            else
            {
                std::cout << "The directory \"" << commandParts[1] << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpCd();
        }
    }

    static void CommandMd_SubDir(const std::string_view& name, unsigned int& chankIndex, Drive* drive, bool middleDirectory)
    {
        drive->GoToChank(chankIndex);

        drive->m_FileStream.Write(&name[0], name.size());
        if (name.size() < MAX_ENTITY_NAME)
        {
            drive->m_FileStream.Write<unsigned char>(0);
            drive->m_FileStream += MAX_ENTITY_NAME - 1 - name.size();
        }

        if (middleDirectory)
        {
            drive->m_FileStream.Write<unsigned char>(1);

            chankIndex = drive->GenerateChank();

            Logger::Info("Generate Chank ", chankIndex, " for Directory");

            drive->m_FileStream.Write(chankIndex);
        }
        else
        {
            drive->m_FileStream.Write<unsigned char>(0);
            drive->m_FileStream.Write<unsigned int>(0);
        }

        std::array<char, (MAX_DIRECTORIES - 1) * 4> data1;
        drive->m_FileStream.Write(&data1[0], data1.size());
        
        drive->m_FileStream.Write<unsigned char>(0);

        std::array<char, MAX_FILES * 4> data2;
        drive->m_FileStream.Write(&data2[0], data2.size());
    }

    static void CommandMd(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            Drive* drive;
            unsigned int chankIndex = 0;
            std::vector<unsigned int> pathInChanks;

            std::string& path = commandParts[1];

            int i = 0;

            // Get Drive
            if (GetDrive(path, drive))
            {
                if (path.size() >= 2 && path[1] == ':')
                {
                    i = 3;
                }
            }
            else
            {
                HelpMd();
                return;
            }

            if (drive == nullptr) // Start from current directory
            {
                drive = Drive::s_DriveCurrent;

                for (PathItem& item : User::s_CurrentDirectory.path)
                {
                    pathInChanks.push_back(item.m_chankIndex);
                }

                chankIndex = User::s_CurrentDirectory.GetBody()->m_ChankIndex;
            }

            // Get path
            std::string_view pathView(path);

            bool exist = true;
            std::string_view firstToCreate;
            std::vector<std::string_view> neadToCreate;
                        
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
                            HelpMd();
                            return;
                        }
                    }
                    else // last name
                    {
                        name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                        if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                        {
                            HelpMd();
                            return;
                        }
                    }

                    if (name.size() > MAX_ENTITY_NAME)
                    {
                        HelpMd();
                        return;
                    }

                    if (!name.compare(".")) // Do nothing
                    {
                        if (!exist)
                        {
                            HelpMd();
                            return;
                        }
                    }
                    else if (!name.compare("..")) // Back
                    {
                        if (!exist)
                        {
                            HelpMd();
                            return;
                        }

                        if (pathInChanks.empty())
                        {
                            HelpMd();
                            return;
                        }
                        
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
                    else // Into sub directory or need to be created
                    {
                        if (exist) //Into sub directory
                        {
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
                                unsigned int chankIndexCheak = drive->m_FileStream.Read<unsigned int>();
                                size_t index = drive->m_FileStream.GetIndex();
                                drive->GoToChank(chankIndexCheak);

                                char* checkName = SmartEntityName().GetName(drive->m_FileStream);
                                found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                                if (found)
                                {
                                    chankIndex = chankIndexCheak;
                                    pathInChanks.push_back(chankIndex);
                                    break;
                                }

                                drive->m_FileStream.ChangeIndex(index);
                            }

                            if (!found)
                            {
                                firstToCreate = name;
                                exist = false;
                            }
                        }
                        else
                        {
                            neadToCreate.push_back(name);
                        }
                    }

                    i++;
                    startNameIndex = i;
                }
                else if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n')
                {
                    HelpMd();
                    return;
                }
                else
                {
                    i++;
                }
            }

            // Create Directory
            if (exist)
            {
                std::cout << "The directory already exist.\n";
            }
            else
            {
                const char* error;
                bool isCurrentDirectory = (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.GetBody()->m_ChankIndex);
                
                EntityName name;
                memcpy(&name, &firstToCreate[0], firstToCreate.size());
                name[firstToCreate.size()] = 0;
                if (chankIndex == 0)
                {
                    chankIndex = drive->CreateDirectory(name, error);

                    if (chankIndex != 0)
                    {
                        drive->LoadBody();
                    }
                    else
                    {
                        std::cout << error << "\n";
                        return;
                    }
                }
                else
                {
                    if (isCurrentDirectory)
                    {
                        chankIndex = User::s_CurrentDirectory.directory->CreateDirectory(name, error);

                        if (chankIndex != 0)
                        {
                            drive->LoadBody();
                        }
                        else
                        {
                            std::cout << error << "\n";
                            return;
                        }
                    }
                    else
                    {
                        Directory directory(chankIndex, drive);
                        chankIndex = directory.CreateDirectory(name, error);

                        if (chankIndex == -1)
                        {
                            std::cout << error << "\n";
                            return;
                        }
                    }
                }

                if (!neadToCreate.empty())
                {
                    std::string_view lastToCreate = neadToCreate[neadToCreate.size() - 1];
                    neadToCreate.pop_back();

                    CommandMd_SubDir(firstToCreate, chankIndex, drive, true);

                    for (const std::string_view& name : neadToCreate)
                    {
                        CommandMd_SubDir(name, chankIndex, drive, true);
                    }

                    CommandMd_SubDir(lastToCreate, chankIndex, drive, false);
                }
            }
        }
        else
        {
            HelpMd();
        }
    }

    static void CommandRd(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            Drive* drive;
            unsigned int chankIndex = 0;
            std::vector<unsigned int> pathInChanks;

            std::string& path = commandParts[1];
            unsigned char directoryIndex = 0;

            int i = 0;

            // Get Drive
            if (GetDrive(path, drive))
            {
                if (path.size() >= 2 && path[1] == ':')
                {
                    i = 3;
                }
            }
            else
            {
                std::cout << "The directory \"" << path << "\" not found.\n";
                return;
            }


            if (drive == nullptr) // Start from current directory
            {
                drive = Drive::s_DriveCurrent;

                for (PathItem& item : User::s_CurrentDirectory.path)
                {
                    pathInChanks.push_back(item.m_chankIndex);
                }

                chankIndex = User::s_CurrentDirectory.GetBody()->m_ChankIndex;
            }

            std::string_view pathView(path);


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
                            std::cout << "The directory \"" << path << "\" not found.\n";
                            return;
                        }
                    }
                    else // last name
                    {
                        name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                        if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                        {
                            std::cout << "The directory \"" << path << "\" not found.\n";
                            return;
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
                            std::cout << "The directory \"" << path << "\" not found.\n";
                            return;
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
                                pathInChanks.push_back(chankIndex);
                                directoryIndex = j;
                                break;
                            }

                            drive->m_FileStream.ChangeIndex(index);
                        }

                        if (!found)
                        {
                            std::cout << "The directory \"" << path << "\" not found.\n";
                            return;
                        }
                    }

                    i++;
                    startNameIndex = i;
                }
                else if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n')
                {
                    std::cout << "The directory \"" << path << "\" not found.\n";
                    return;
                }
                else
                {
                    i++;
                }
            }

            if (pathInChanks.empty())
            {
                std::cout << "The directory \"" << path << "\" not found.\n";
                return;
            }

            if (drive == Drive::s_DriveCurrent)
            {
                if (pathInChanks.size() <= User::s_CurrentDirectory.path.size())
                {
                    bool same = true;
                    for (int i = 0; same && i < pathInChanks.size(); i++)
                    {
                        same = (pathInChanks[i] == User::s_CurrentDirectory.path[i].m_chankIndex);
                    }

                    if (same)
                    {
                        std::cout << "This directory opend!\n";
                        return;
                    }
                }
            }

            // Delete directory
            if (pathInChanks.size() == 1)
            {
                drive->DeleteDirectory(directoryIndex);
            }
            else
            {
                chankIndex = pathInChanks[pathInChanks.size() - 2];
                Directory directory(chankIndex, drive);
                directory.DeleteDirectory(directoryIndex);
            }
        }
        else
        {
            HelpCd();
        }
    }

    // DoCommand
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
            if (helpMode)
            {
                HelpMd();
            }
            else
            {
                CommandMd(command, commandParts);
            }
        }
        else if (!action.compare("rd"))
        {
            if (helpMode)
            {
                HelpRd();
            }
            else
            {
                CommandRd(command, commandParts);
            }
        }
        else if (!action.compare("cd"))
        {
            if (helpMode)
            {
                HelpCd();
            }
            else
            {
                CommandCd(command, commandParts);
            }
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

#ifdef _DEBUG
    static void PrintFile(int tabs, unsigned int chankIndex)
    {
        File* file = new File(chankIndex, Drive::s_DriveCurrent);

        for (size_t i = 0; i < tabs; i++)
        {
            std::cout << "  ";
        }

        std::cout << file->m_Name.GetName() << "\n";

        delete file;
    }

    static void PrintDirectory(int tabs, unsigned int chankIndex, bool displayFiles)
    {
        Directory* directory = new Directory(chankIndex, Drive::s_DriveCurrent);

        for (size_t i = 0; i < tabs; i++)
        {
            std::cout << "  ";
        }

        std::cout << directory->m_Name.GetName() << "\n";

        for (int i = 0; i < directory->m_DirectoriesCount; i++)
        {
            PrintDirectory(tabs + 1, directory->m_DirectoriesLocations[i]);
        }

        for (int i = 0; i < directory->m_FilesCount; i++)
        {
            PrintFile(tabs + 1, directory->m_FilesLocations[i]);
        }

        delete directory;
    }

    static void PrintDrive(bool displayFiles)
    {
        std::cout << Drive::s_DriveCurrent->m_DriveName << ":\n";
        
        for (int i = 0; i < Drive::s_DriveCurrent->m_DirectoriesCount; i++)
        {
            PrintDirectory(1, Drive::s_DriveCurrent->m_DirectoriesLocations[i]);
        }

        for (int i = 0; i < Drive::s_DriveCurrent->m_FilesCount; i++)
        {
            PrintFile(1, Drive::s_DriveCurrent->m_FilesLocations[i]);
        }

        std::cout << "\n";
    }
#else
    static void PrintDrive(bool displayFiles)
    {

    }
#endif

    // Public Functions
    void Load()
    {
        User::s_CurrentDirectory.Change();
    }

    void Loop()
    {
        bool running = true;
        while (running)
        {
            //PrintDrive(false);
            
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