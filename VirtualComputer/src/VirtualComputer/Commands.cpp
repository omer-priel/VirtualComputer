#include "pch.h"
#include "Commands.h"

#include <unordered_map>
#include <string_view>
#include <optional>

#include "HelpBody.h"
#include "HardDrive.h"
#include "Drive.h"
#include "Directory.h"
#include "File.h"
#include "EntityType.h"
#include "ErrorMessages.h"

namespace VirtualComputer::User
{
    static char s_StartLine[MAX_COMMAND_SIZE - 10];
    static const char s_Split = '\\';

    static struct CurrentDirectory
    {
        DirectoryBody* directory = nullptr;
        std::vector<PathItem> path;

        void Change()
        {
            s_StartLine[0] = Drive::s_DriveCurrent->m_DriveName;
            s_StartLine[1] = ':';
            s_StartLine[2] = s_Split;
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

        void Change(Drive* drive, unsigned int& chankIndex, std::vector<PathItem>* pathInChanks)
        {            
            if (Drive::s_DriveCurrent != drive || chankIndex != directory->m_ChankIndex)
            {
                Drive::s_DriveCurrent = drive;
                
                if (directory->m_ChankIndex != 0)
                {
                    delete directory;
                }

                if (chankIndex == 0)
                {
                    path.clear();
                    directory = drive;
                }
                else
                {
                    path.clear();
                    for (PathItem& chankIndex : *pathInChanks)
                    {
                        path.push_back(chankIndex);
                    }
                    directory = new Directory(chankIndex, drive->m_Drive);
                }

                Change();
            }
        }

        void Update(unsigned int index, EntityName newName)
        {
            path[index].m_Name = std::string(&newName[0]);
            Change();
        }
        
        bool IsDrive() const
        {
            return directory->m_ChankIndex == 0;
        }

        bool IsDirectory() const
        {
            return directory->m_ChankIndex != 0;
        }
    }
    s_CurrentDirectory;

    static void GetCommand(std::string& command)
    {
        std::cout << s_StartLine << "> ";
        std::getline(std::cin, command);
    }
}

namespace VirtualComputer::Commands
{
    // Subclasses
    typedef void (HelpEvent)();

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

            chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
        }

        HardDrive*& hardDrive = drive->m_Drive;

        std::string_view pathView(path);

        unsigned int startNameIndex = i;
        while (i < path.size())
        {
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
                        hardDrive->GoToChank(chankIndex);
                    }
                    else
                    {
                        hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                    }

                    unsigned char count = hardDrive->m_FileStream.Read<unsigned char>();

                    bool found = false;
                    for (unsigned char j = 0; j < count; j++)
                    {
                        chankIndex = hardDrive->m_FileStream.Read<unsigned int>();
                        size_t index = hardDrive->m_FileStream.GetIndex();
                        hardDrive->GoToChank(chankIndex);

                        char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                        found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                        if (found)
                        {
                            pathInChanks.emplace_back(chankIndex, checkName);
                            break;
                        }

                        hardDrive->m_FileStream.ChangeIndex(index);
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

    static EntityType GetEntity(std::string& path, Drive*& drive, unsigned int& chankIndex, std::vector<unsigned int>& pathInChanks, std::optional<unsigned char>& entityIndex)
    {
        chankIndex = 0;
        entityIndex.reset();

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
            return EntityType::NotExists;
        }

        pathInChanks.clear();

        if (drive == nullptr) // Start from current directory
        {
            drive = Drive::s_DriveCurrent;

            for (PathItem& item : User::s_CurrentDirectory.path)
            {
                pathInChanks.push_back(item.m_chankIndex);
            }

            chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
        }

        HardDrive*& hardDrive = drive->m_Drive;

        std::string_view pathView(path);

        unsigned int startNameIndex = i;
        while (i < path.size())
        {
            if ((path[i] == '/' || path[i] == '\\') || (i == path.size() - 1))
            {
                std::string_view name;
                if (path[i] == '/' || path[i] == '\\')
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex);

                    if (name.empty()) // can't be empty
                    {
                        return EntityType::NotExists;
                    }
                }
                else // last name
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                    if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                    {
                        return EntityType::NotExists;
                    }
                }

                if (!name.compare("."))
                {
                    // Do nothing
                }
                else if (!name.compare(".."))
                {
                    // Back
                    entityIndex.reset();
                    if (pathInChanks.empty())
                    {
                        return EntityType::NotExists;
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
                            chankIndex = pathInChanks.back();
                        }
                    }
                }
                else
                {
                    // into sub directory
                    if (chankIndex == 0)
                    {
                        hardDrive->GoToChank(chankIndex);
                    }
                    else
                    {
                        hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                    }

                    unsigned char count = hardDrive->m_FileStream.Read<unsigned char>(); // Directory count

                    bool found = false;
                    for (unsigned char j = 0; j < count; j++)
                    {
                        chankIndex = hardDrive->m_FileStream.Read<unsigned int>();
                        size_t index = hardDrive->m_FileStream.GetIndex();
                        hardDrive->GoToChank(chankIndex);

                        char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                        found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                        if (found)
                        {
                            entityIndex.emplace(j);
                            pathInChanks.push_back(chankIndex);
                            break;
                        }

                        hardDrive->m_FileStream.ChangeIndex(index);
                    }

                    if (!found && i == path.size() - 1) // last name
                    {
                        hardDrive->m_FileStream += (MAX_DIRECTORIES - count) * 4;
                        count = hardDrive->m_FileStream.Read<unsigned char>(); // File count

                        for (unsigned char j = 0; j < count; j++)
                        {
                            chankIndex = hardDrive->m_FileStream.Read<unsigned int>();
                            size_t index = hardDrive->m_FileStream.GetIndex();
                            hardDrive->GoToChank(chankIndex);

                            char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                            found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                            if (found)
                            {
                                entityIndex.emplace(j);
                                pathInChanks.push_back(chankIndex);
                                return EntityType::File;
                            }

                            hardDrive->m_FileStream.ChangeIndex(index);
                        }
                    }

                    if (!found)
                    {
                        return EntityType::NotExists;
                    }
                }

                i++;
                startNameIndex = i;
            }
            else if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n')
            {
                return EntityType::NotExists;
            }
            else
            {
                i++;
            }
        }

        return EntityType::Directory;
    }

    static bool TextToNumber(const std::string& text, unsigned int& output)
    {
        output = 0;
        unsigned int v = 1;
        for (int i = text.size() - 1; i >= 0; i--)
        {
            const char& tv = text[i];
            if ('0' <= tv && tv <= '9')
            {
                output += v * (tv - '0');
                v *= 10;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    static void CreateSubDirectory(const std::string_view& name, unsigned int& chankIndex, HardDrive* drive, bool middleDirectory)
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
        data1.fill(0);
        drive->m_FileStream.Write(&data1[0], data1.size());

        drive->m_FileStream.Write<unsigned char>(0);

        std::array<char, MAX_FILES * 4> data2;
        data2.fill(0);
        drive->m_FileStream.Write(&data2[0], data2.size());
    }

    static bool CreateDirectory(std::string& path, bool cantBeExist, HelpEvent& help, Drive*& drive, unsigned int& chankIndex)
    {
        drive = nullptr;
        chankIndex = 0;
        std::vector<unsigned int> pathInChanks;

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
            help();
            return false;
        }

        if (drive == nullptr) // Start from current directory
        {
            drive = Drive::s_DriveCurrent;

            for (PathItem& item : User::s_CurrentDirectory.path)
            {
                pathInChanks.push_back(item.m_chankIndex);
            }

            chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
        }

        HardDrive*& hardDrive = drive->m_Drive;

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
                        help();
                        return false;
                    }
                }
                else // last name
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                    if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                    {
                        help();
                        return false;
                    }
                }

                if (name.size() > MAX_ENTITY_NAME)
                {
                    help();
                    return false;
                }

                if (!name.compare(".")) // Do nothing
                {
                    if (!exist)
                    {
                        help();
                        return false;
                    }
                }
                else if (!name.compare("..")) // Back
                {
                    if (!exist)
                    {
                        help();
                        return false;
                    }

                    if (pathInChanks.empty())
                    {
                        help();
                        return false;
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
                            hardDrive->GoToChank(chankIndex);
                        }
                        else
                        {
                            hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                        }

                        unsigned char count = hardDrive->m_FileStream.Read<unsigned char>();

                        bool found = false;
                        for (unsigned char j = 0; j < count; j++)
                        {
                            unsigned int chankIndexCheak = hardDrive->m_FileStream.Read<unsigned int>();
                            size_t index = hardDrive->m_FileStream.GetIndex();
                            hardDrive->GoToChank(chankIndexCheak);

                            char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                            found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                            if (found)
                            {
                                chankIndex = chankIndexCheak;
                                pathInChanks.push_back(chankIndex);
                                break;
                            }

                            hardDrive->m_FileStream.ChangeIndex(index);
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
                help();
                return false;
            }
            else
            {
                i++;
            }
        }

        // Create Directory
        if (exist)
        {
            if (cantBeExist)
            {
                std::cout << "The directory already exist.\n";
                return false;
            }
            return true;
        }

        const char* error;
        bool isCurrentDirectory = (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.directory->m_ChankIndex);

        EntityName name;
        memcpy(&name, &firstToCreate[0], firstToCreate.size());
        name[firstToCreate.size()] = 0;
        if (chankIndex == 0)
        {
            chankIndex = drive->CreateDirectory(name, error);

            if (chankIndex == 0)
            {
                std::cout << error << "\n";
                return false;
            }
        }
        else
        {
            if (isCurrentDirectory)
            {
                chankIndex = User::s_CurrentDirectory.directory->CreateDirectory(name, error);

                if (chankIndex == 0)
                {
                    std::cout << error << "\n";
                    return false;
                }
            }
            else
            {
                Directory directory(chankIndex, hardDrive);
                chankIndex = directory.CreateDirectory(name, error);

                if (chankIndex == -1)
                {
                    std::cout << error << "\n";
                    return false;
                }
            }
        }

        if (!neadToCreate.empty())
        {
            std::string_view lastToCreate = neadToCreate[neadToCreate.size() - 1];
            neadToCreate.pop_back();

            CreateSubDirectory(firstToCreate, chankIndex, hardDrive, true);

            for (const std::string_view& name : neadToCreate)
            {
                CreateSubDirectory(name, chankIndex, hardDrive, true);
            }

            CreateSubDirectory(lastToCreate, chankIndex, hardDrive, false);
        }

        return true;
    }

    static bool CreateDirectory(std::string& path, bool cantBeExist, Drive* cantBeDrive, unsigned int& cantBeChankIndex, HelpEvent& help, Drive*& drive, unsigned int& chankIndex)
    {
        drive = nullptr;
        chankIndex = 0;
        std::vector<unsigned int> pathInChanks;

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
            help();
            return false;
        }

        if (drive == nullptr) // Start from current directory
        {
            drive = Drive::s_DriveCurrent;

            for (PathItem& item : User::s_CurrentDirectory.path)
            {
                pathInChanks.push_back(item.m_chankIndex);
            }

            chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
        }

        // Get path
        HardDrive*& hardDrive = drive->m_Drive;
        
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
                        help();
                        return false;
                    }
                }
                else // last name
                {
                    name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                    if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                    {
                        help();
                        return false;
                    }
                }

                if (name.size() > MAX_ENTITY_NAME)
                {
                    help();
                    return false;
                }

                if (!name.compare(".")) // Do nothing
                {
                    if (!exist)
                    {
                        help();
                        return false;
                    }
                }
                else if (!name.compare("..")) // Back
                {
                    if (!exist)
                    {
                        help();
                        return false;
                    }

                    if (pathInChanks.empty())
                    {
                        help();
                        return false;
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
                            hardDrive->GoToChank(chankIndex);
                        }
                        else
                        {
                            hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                        }

                        unsigned char count = hardDrive->m_FileStream.Read<unsigned char>();

                        bool found = false;
                        for (unsigned char j = 0; j < count; j++)
                        {
                            unsigned int chankIndexCheak = hardDrive->m_FileStream.Read<unsigned int>();
                            size_t index = hardDrive->m_FileStream.GetIndex();
                            hardDrive->GoToChank(chankIndexCheak);

                            char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                            found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                            if (found)
                            {
                                if (cantBeDrive == drive && cantBeChankIndex == chankIndex)
                                {
                                    help();
                                    return false;
                                }

                                chankIndex = chankIndexCheak;
                                pathInChanks.push_back(chankIndex);
                                break;
                            }

                            hardDrive->m_FileStream.ChangeIndex(index);
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
                help();
                return false;
            }
            else
            {
                i++;
            }
        }

        // Create Directory
        if (exist)
        {
            if (cantBeExist)
            {
                std::cout << "The directory already exist.\n";
                return false;
            }
            return true;
        }

        const char* error;
        bool isCurrentDirectory = (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.directory->m_ChankIndex);

        EntityName name;
        memcpy(&name, &firstToCreate[0], firstToCreate.size());
        name[firstToCreate.size()] = 0;
        if (chankIndex == 0)
        {
            chankIndex = drive->CreateDirectory(name, error);

            if (chankIndex == 0)
            {
                std::cout << error << "\n";
                return false;
            }
        }
        else
        {
            if (isCurrentDirectory)
            {
                chankIndex = User::s_CurrentDirectory.directory->CreateDirectory(name, error);

                if (chankIndex == 0)
                {
                    std::cout << error << "\n";
                    return false;
                }
            }
            else
            {
                Directory directory(chankIndex, hardDrive);
                chankIndex = directory.CreateDirectory(name, error);

                if (chankIndex == -1)
                {
                    std::cout << error << "\n";
                    return false;
                }
            }
        }

        if (!neadToCreate.empty())
        {
            std::string_view lastToCreate = neadToCreate[neadToCreate.size() - 1];
            neadToCreate.pop_back();

            CreateSubDirectory(firstToCreate, chankIndex, hardDrive, true);

            for (const std::string_view& name : neadToCreate)
            {
                CreateSubDirectory(name, chankIndex, hardDrive, true);
            }

            CreateSubDirectory(lastToCreate, chankIndex, hardDrive, false);
        }

        return true;
    }

    static bool CreateDirectory(std::string& path, bool cantBeExist, HelpEvent& help)
    {
        Drive* drive;
        unsigned int chankIndex;
        return CreateDirectory(path, cantBeExist, help, drive, chankIndex);
    }

    static void CopyFile(HardDrive*& driveFrom, const unsigned int& chankIndexEntity,
        HardDrive*& driveTo, const unsigned int& chankIndexTarget)
    {
        driveFrom->GoToChank(chankIndexEntity);
        File file(chankIndexTarget, driveFrom);

        driveTo->GoToChank(chankIndexTarget);
        
        driveTo->m_FileStream.Write(&file.m_Name.m_Name[0], MAX_ENTITY_NAME);
        driveTo->m_FileStream.Write<unsigned int>(file.m_Size);
        driveTo->m_FileStream.Write(file.m_FirstBodyChank, FIRST_FILE_BODY_SIZE);
        for (auto& chank : file.m_BodyChanks)
        {
            unsigned nextChankIndex = driveTo->GenerateChank();
            driveTo->m_FileStream.Write<unsigned int>(nextChankIndex);

            driveTo->GoToChank(nextChankIndex);

            driveTo->m_FileStream.Write(chank->Body, CHANK_SIZE - 4);
        }
        driveTo->m_FileStream.Write<unsigned int>(0);
    }
    
    static void CopyDirectory(HardDrive*& driveFrom, const unsigned int& chankIndexEntity,
        HardDrive*& driveTo, const unsigned int& chankIndexTarget)
    {
        Chank data;
        data.fill(0);

        // Read
        char name[MAX_ENTITY_NAME];
        unsigned char directoriesCount;
        unsigned int directories[MAX_DIRECTORIES];
        unsigned char filesCount;
        unsigned int files[MAX_FILES];

        driveFrom->GoToChank(chankIndexEntity);
        driveFrom->m_FileStream.Read(name, MAX_ENTITY_NAME);
        driveFrom->m_FileStream.Read<unsigned char>(directoriesCount);
        driveFrom->m_FileStream.Read((char*)directories, MAX_DIRECTORIES * 4);
        driveFrom->m_FileStream.Read<unsigned char>(filesCount);
        driveFrom->m_FileStream.Read((char*)files, MAX_FILES * 4);

        // Write
        driveTo->GoToChank(chankIndexTarget);
        driveTo->m_FileStream.Write(name, MAX_ENTITY_NAME);
        
        driveTo->m_FileStream.Write(directoriesCount);
        for (size_t i = 0; i < directoriesCount; i++)
        {
            unsigned int chankIndexTarget = driveTo->GenerateChank();
            driveTo->m_FileStream.Write<unsigned int>(chankIndexTarget);
            
            size_t index = driveTo->m_FileStream.GetIndex();
            CopyDirectory(driveFrom, directories[i], driveTo, chankIndexTarget);
            driveTo->m_FileStream.ChangeIndex(index);
        }

        driveTo->m_FileStream.Write(&data[0], (MAX_DIRECTORIES - directoriesCount) * 4);

        driveTo->m_FileStream.Write(filesCount);
        for (size_t i = 0; i < filesCount; i++)
        {
            unsigned int chankIndexTarget = driveTo->GenerateChank();
            driveTo->m_FileStream.Write<unsigned int>(chankIndexTarget);

            size_t index = driveTo->m_FileStream.GetIndex();
            CopyFile(driveFrom, files[i] , driveTo, chankIndexTarget);
            driveTo->m_FileStream.ChangeIndex(index);
        }

        driveTo->m_FileStream.Write(&data[0], (MAX_FILES - filesCount) * 4);
    }
    
    static bool CopyEntity(HardDrive*& driveFrom, unsigned int& chankIndexEntity, EntityType& type, HardDrive*& driveTo, DirectoryBody* target)
    {
        if (type == EntityType::Directory)
        {
            if (target->m_DirectoriesCount == MAX_DIRECTORIES)
            {
                std::cout << "The target driectory can't have more then " << MAX_DIRECTORIES << " directories!.\n";
                return false;
            }
        }
        else // File
        {
            if (target->m_FilesCount == MAX_FILES)
            {
                std::cout << "The target driectory can't have more then " << MAX_FILES << " files!.\n";
                return false;
            }
        }

        EntityName entityName;
        driveFrom->GoToChank(chankIndexEntity);
        driveFrom->m_FileStream.Read(&entityName[0], MAX_ENTITY_NAME);

        if (target->ExistName(entityName))
        {
            std::cout << ErrorMessages::NameAlreadyExist << "\n";
            return false;
        }

        // Copy
        unsigned int chankIndexTarget = driveTo->GenerateChank();
        target->AddEntity(type, chankIndexTarget, entityName);
        if (type == EntityType::Directory)
        {
            CopyDirectory(driveFrom, chankIndexEntity, driveTo, chankIndexTarget);
        }
        else // File
        {
            CopyFile(driveFrom, chankIndexEntity, driveTo, chankIndexTarget);
        }
    }

    static bool CopyEntity(Drive*& driveFrom, unsigned int& chankIndexEntity, EntityType& type, Drive*&  driveTo, unsigned int&  chankIndexTo)
    {
        if (chankIndexTo == 0)
        {
            return CopyEntity(driveFrom->m_Drive, chankIndexEntity, type, driveTo->m_Drive, driveTo);
        }
        else
        {
            if (driveTo == Drive::s_DriveCurrent && chankIndexTo == User::s_CurrentDirectory.directory->m_ChankIndex)
            {
                return CopyEntity(driveFrom->m_Drive, chankIndexEntity, type, driveTo->m_Drive, User::s_CurrentDirectory.directory);
            }
            else
            {
                Directory directoryTo(chankIndexTo, driveTo->m_Drive);
                return CopyEntity(driveFrom->m_Drive, chankIndexEntity, type, driveTo->m_Drive, &directoryTo);
            }
        }
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

    static void HelpMf()
    {
        std::cout
            << "Create new file.\n"
            << "\n"
            << "    mf [path] - Create new file.\n"
            << "        path - Path of the new file\n"
            << "    mf [path] [content] - Create new file with content.\n"
            << "        path - Path of the new file\n"
            << "        content - Content of the new file\n";
    }

    static void HelpRf()
    {
        std::cout
            << "Delete file.\n"
            << "\n"
            << "    rd [path] - Delete file.\n"
            << "        path - Path of the file\n";
    }

    static void HelpMove()
    {
        std::cout
            << "Move directory or file to other directory.\n"
            << "\n"
            << "    move [path] [target] - Move directory or file to other directory.\n"
            << "        path - Path of directory or file\n"
            << "        target - Path of the target directory\n";
    }

    static void HelpCopy()
    {
        std::cout
            << "Copy directory or file to other directory.\n"
            << "\n"
            << "    move [path] [target] - Copy directory or file to other directory.\n"
            << "        path - Path of directory or file\n"
            << "        target - Path of the target directory\n";
    }

    static void HelpRename()
    {
        std::cout
            << "Rename directory or file.\n"
            << "\n"
            << "    rename [path] [new name] - Rename directory or file.\n"
            << "        path - Path of directory or file\n"
            << "        new name - New name for the directory or the file\n";
    }

    static void HelpPrint()
    {
        std::cout
            << "Display file.\n"
            << "\n"
            << "    rd [path] - Display file.\n"
            << "        path - Path of the file\n";
    }

    static void HelpEdit()
    {
        std::cout
            << "Edit file.\n"
            << "\n"
            << "    rd [path] [text] - Write text in the start of the file.\n"
            << "        path - Path of the file\n"
            << "        text - The text to write\n"
            << "    rd [path] [/a | /append] [text] - Append text to file.\n"
            << "        path  - Path of the file\n"
            << "        text  - The text to write\n"
            << "    rd [path] [start] [text] - Write text in the middel of the file.\n"
            << "        path  - Path of the file\n"
            << "        start - The plase to start the write\n"
            << "        text  - The text to write\n"
            << "    rd [path] [/r | /resize] [size] - Resize file.\n"
            << "        path - Path of the file\n"
            << "        size - The new size of the file\n"
            << "    rd [path] [/c | /clean] - Clean file.\n"
            << "        path - Path of the file\n";
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
            Drive* drive;

            if (commandParts.size() == 1)
            {
                drive = Drive::s_DriveCurrent;
                directory = User::s_CurrentDirectory.directory;
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
                        directory = new Directory(chankIndex, drive->m_Drive);
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
            
            while (filesIndex < directory->m_FilesCount)
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

    static void CommandMd(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            CreateDirectory(commandParts[1], true, HelpMd);
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

                chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
            }

            HardDrive*& hardDrive = drive->m_Drive;

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
                            hardDrive->GoToChank(chankIndex);
                        }
                        else
                        {
                            hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                        }

                        unsigned char count = hardDrive->m_FileStream.Read<unsigned char>();

                        bool found = false;
                        for (unsigned char j = 0; j < count; j++)
                        {
                            chankIndex = hardDrive->m_FileStream.Read<unsigned int>();
                            size_t index = hardDrive->m_FileStream.GetIndex();
                            hardDrive->GoToChank(chankIndex);

                            char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                            found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                            if (found)
                            {
                                pathInChanks.push_back(chankIndex);
                                directoryIndex = j;
                                break;
                            }

                            hardDrive->m_FileStream.ChangeIndex(index);
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
                if (User::s_CurrentDirectory.directory->m_ChankIndex == chankIndex)
                {
                    User::s_CurrentDirectory.directory->DeleteDirectory(directoryIndex);
                }
                else
                {
                    Directory directory(chankIndex, hardDrive);
                    directory.DeleteDirectory(directoryIndex);
                }
            }
        }
        else
        {
            HelpRd();
        }
    }

    static void CommandMf_CreateFile(Drive*& drive, unsigned int& chankIndex, bool& isCurrentDirectory,
        EntityName& fileName, char* contentPtr, size_t& contentSize)
    {
        const char* error = nullptr;
        if (chankIndex == 0)
        {
            chankIndex = drive->CreateFile(fileName, contentPtr, contentSize, error);

            if (chankIndex == 0)
            {
                std::cout << error << "\n";
                return;
            }
        }
        else
        {
            if (isCurrentDirectory)
            {
                chankIndex = User::s_CurrentDirectory.directory->CreateFile(fileName, contentPtr, contentSize, error);

                if (chankIndex == 0)
                {
                    std::cout << error << "\n";
                    return;
                }
            }
            else
            {
                Directory directory(chankIndex, drive->m_Drive);
                chankIndex = directory.CreateFile(fileName, contentPtr, contentSize, error);

                if (chankIndex == -1)
                {
                    std::cout << error << "\n";
                    return;
                }
            }
        }
    }

    static void CommandMf(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2 || commandParts.size() == 3)
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
                HelpMf();
                return;
            }

            if (drive == nullptr) // Start from current directory
            {
                drive = Drive::s_DriveCurrent;

                for (PathItem& item : User::s_CurrentDirectory.path)
                {
                    pathInChanks.push_back(item.m_chankIndex);
                }

                chankIndex = User::s_CurrentDirectory.directory->m_ChankIndex;
            }

            // Get path
            HardDrive*& hardDrive = drive->m_Drive;
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
                            HelpMf();
                            return;
                        }
                    }
                    else // last name
                    {
                        name = pathView.substr(startNameIndex, i - startNameIndex + 1);
                        if (path[i] == ':' || path[i] == '\'' || path[i] == '\"' || path[i] == '\n') // cheak last char
                        {
                            HelpMf();
                            return;
                        }
                    }

                    if (name.size() > MAX_ENTITY_NAME)
                    {
                        HelpMf();
                        return;
                    }

                    if (!name.compare(".")) // Do nothing
                    {
                        if (!exist)
                        {
                            HelpMf();
                            return;
                        }
                    }
                    else if (!name.compare("..")) // Back
                    {
                        if (!exist)
                        {
                            HelpMf();
                            return;
                        }

                        if (pathInChanks.empty())
                        {
                            HelpMf();
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
                                hardDrive->GoToChank(chankIndex);
                            }
                            else
                            {
                                hardDrive->GoToChank(chankIndex, MAX_ENTITY_NAME);
                            }

                            unsigned char count = hardDrive->m_FileStream.Read<unsigned char>();

                            bool found = false;
                            for (unsigned char j = 0; j < count; j++)
                            {
                                unsigned int chankIndexCheak = hardDrive->m_FileStream.Read<unsigned int>();
                                size_t index = hardDrive->m_FileStream.GetIndex();
                                hardDrive->GoToChank(chankIndexCheak);

                                char* checkName = SmartEntityName().GetName(hardDrive->m_FileStream);
                                found = SmartEntityName::IsEqual(path.c_str() + startNameIndex, checkName, name.size(), strlen(checkName));

                                if (found)
                                {
                                    chankIndex = chankIndexCheak;
                                    pathInChanks.push_back(chankIndex);
                                    break;
                                }

                                hardDrive->m_FileStream.ChangeIndex(index);
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

            // Create File and Directories
            if (exist)
            {
                std::cout << "Exist directory with the same path.\n";
                return;
            }

            const char* error;
            bool isCurrentDirectory = (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.directory->m_ChankIndex);

            EntityName fileName;
            if (neadToCreate.empty())
            {
                memcpy(&fileName, &firstToCreate[0], firstToCreate.size());
                fileName[firstToCreate.size()] = 0;
            }
            else
            {
                std::string_view& lastToCreate = neadToCreate[neadToCreate.size() - 1];
                neadToCreate.pop_back();

                memcpy(&fileName, &lastToCreate[0], lastToCreate.size());
                fileName[lastToCreate.size()] = 0;

                // Create Directories
                EntityName name;
                memcpy(&name, &firstToCreate[0], firstToCreate.size());
                name[firstToCreate.size()] = 0;
                if (chankIndex == 0)
                {
                    chankIndex = drive->CreateDirectory(name, error);

                    if (chankIndex == 0)
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

                        if (chankIndex == 0)
                        {
                            std::cout << error << "\n";
                            return;
                        }
                    }
                    else
                    {
                        Directory directory(chankIndex, hardDrive);
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
                    std::string_view& lastDirectory = neadToCreate[neadToCreate.size() - 1];
                    neadToCreate.pop_back();

                    CreateSubDirectory(firstToCreate, chankIndex, hardDrive, true);

                    for (const std::string_view& name : neadToCreate)
                    {
                        CreateSubDirectory(name, chankIndex, hardDrive, true);
                    }

                    CreateSubDirectory(lastDirectory, chankIndex, hardDrive, false);
                }

                isCurrentDirectory = false;
            }

            // Create File
            char* contentPtr = nullptr;
            size_t contentSize = 0;

            if (commandParts.size() == 3)
            {
                contentPtr = (char*)commandParts[2].c_str();
                contentSize = commandParts[2].size();
            }

            CommandMf_CreateFile(drive, chankIndex, isCurrentDirectory, fileName, contentPtr, contentSize);
        }
        else
        {
            HelpMf();
        }
    }
    
    static void CommandRf(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            DirectoryBody* directory;
            Drive* drive;
            unsigned int chankIndex;
            std::optional<unsigned char> fileIndex;
            std::vector<unsigned int> pathInChanks;
            if (GetEntity(commandParts[1], drive, chankIndex, pathInChanks, fileIndex) == EntityType::File)
            {
                if (pathInChanks.size() == 1) // In drive
                {
                    drive->DeleteFile(fileIndex.value());
                }
                else // In Directory
                {
                    chankIndex = pathInChanks[pathInChanks.size() - 2];
                    if (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.directory->m_ChankIndex)
                    {
                        User::s_CurrentDirectory.directory->DeleteFile(fileIndex.value());
                    }
                    else
                    {
                        Directory directory = Directory(chankIndex, drive->m_Drive);
                        directory.DeleteFile(fileIndex.value());
                    }
                }
            }
            else
            {
                std::cout << "The file \"" << commandParts[1] << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpRf();
        }
    }

    static bool CommandMove_Add(Drive*& drive, unsigned int& chankIndexEntity, EntityType& type,
        unsigned int& chankIndexTo, std::string& target)
    {
        HardDrive*& hardDrive = drive->m_Drive;

        // Add
        hardDrive->GoToChank(chankIndexEntity);

        EntityName entityName;
        hardDrive->m_FileStream.Read(&entityName[0], MAX_ENTITY_NAME);
        entityName[MAX_ENTITY_NAME] = 0;

        if (chankIndexTo == 0)
        {
            if (drive->m_DirectoriesCount == MAX_DIRECTORIES)
            {
                std::cout << "The drive " << drive->m_DriveName << ": can't have more then 255 directories!.\n";
                return false;
            }

            if (drive->ExistName(entityName))
            {
                std::cout << ErrorMessages::NameAlreadyExist << "\n";
                return false;
            }

            // Add to Drive
            drive->AddEntity(type, chankIndexEntity, entityName);
        }
        else
        {
            if (chankIndexTo == User::s_CurrentDirectory.directory->m_ChankIndex)
            {
                if (User::s_CurrentDirectory.directory->m_DirectoriesCount == MAX_DIRECTORIES)
                {
                    std::cout << "The directory \"" << target << "\" can't have more then 255 directories!.\n";
                    return false;
                }

                if (User::s_CurrentDirectory.directory->ExistName(entityName))
                {
                    std::cout << ErrorMessages::NameAlreadyExist << "\n";
                    return false;
                }

                // Add to Current Directory
                User::s_CurrentDirectory.directory->AddEntity(type, chankIndexEntity, entityName);
            }
            else
            {
                Directory directoryTo(chankIndexTo, hardDrive);
                if (directoryTo.m_DirectoriesCount == MAX_DIRECTORIES)
                {
                    std::cout << "The directory \"" << target << "\" can't have more then 255 directories!.\n";
                    return false;
                }

                if (directoryTo.ExistName(entityName))
                {
                    std::cout << ErrorMessages::NameAlreadyExist << "\n";
                    return false;
                }

                // Add to Directory
                directoryTo.AddEntity(type, chankIndexEntity, entityName);
            }
        }

        return true;
    }

    static void CommandMove_Delete(Drive*& drive, unsigned int& chankIndexFrom, unsigned int& chankIndexEntity,
        EntityType& type, std::optional<unsigned char>& entityIndexOptional)
    {
        if (chankIndexFrom == 0)
        {
            drive->DeleteEntity(type, entityIndexOptional, chankIndexEntity);
        }
        else // In Directory
        {
            if (drive == Drive::s_DriveCurrent && chankIndexFrom == User::s_CurrentDirectory.directory->m_ChankIndex)
            {
                User::s_CurrentDirectory.directory->DeleteEntity(type, entityIndexOptional, chankIndexEntity);
            }
            else
            {
                Directory directory = Directory(chankIndexFrom, drive->m_Drive);
                directory.DeleteEntity(type, entityIndexOptional, chankIndexEntity);
            }
        }
    }

    static void CommandMove(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 3)
        {
            std::string& path = commandParts[1];
            std::string& target = commandParts[2];

            Drive* driveFrom;
            unsigned int chankIndexFrom; // directory of the entity
            unsigned int chankIndexEntity; // the entity
            std::vector<unsigned int> pathInChanks;
            std::optional<unsigned char> entityIndexOptional;

            EntityType type = GetEntity(path, driveFrom, chankIndexFrom, pathInChanks, entityIndexOptional);
            if (type != EntityType::NotExists)
            {
                if (pathInChanks.empty())
                {
                    std::cout << "Can't move drive.\n";
                    return;
                }

                if (pathInChanks.size() == 1)
                {
                    chankIndexFrom = 0;
                }
                else
                {
                    chankIndexFrom = pathInChanks[pathInChanks.size() - 2];
                }

                chankIndexEntity = pathInChanks[pathInChanks.size() - 1];

                if (driveFrom == Drive::s_DriveCurrent && type == EntityType::Directory)
                {
                    for (const auto& directory : User::s_CurrentDirectory.path)
                    {
                        if (chankIndexEntity == directory.m_chankIndex)
                        {
                            std::cout << "Can't move directory in current path.\n";
                            return;
                        }
                    }
                }

                // md target
                Drive* driveTo;
                unsigned int chankIndexTo;

                bool found;
                if (type == EntityType::Directory)
                {
                    found = CreateDirectory(target, false, driveFrom, chankIndexEntity, HelpMove, driveTo, chankIndexTo);
                }
                else
                {
                    found = CreateDirectory(target, false, HelpMove, driveTo, chankIndexTo);
                }

                if (found)
                {
                    if (driveFrom != driveTo) // from Drive To Other Drive
                    {
                        if (CopyEntity(driveFrom, chankIndexEntity, type, driveTo, chankIndexTo))
                        {
                            CommandMove_Delete(driveFrom, chankIndexFrom, chankIndexEntity, type, entityIndexOptional);
                        }
                    }
                    else
                    {
                        if (chankIndexEntity != chankIndexTo)
                        {
                            if (chankIndexFrom != chankIndexTo)
                            {
                                //Add
                                if (!CommandMove_Add(driveFrom, chankIndexEntity, type, chankIndexTo, target))
                                {
                                    return;
                                }

                                // Remove
                                if (chankIndexFrom == 0)
                                {
                                    // Remove from Drive
                                    driveFrom->RemoveEntity(type, entityIndexOptional, chankIndexEntity);
                                }
                                else
                                {
                                    if (chankIndexFrom == User::s_CurrentDirectory.directory->m_ChankIndex)
                                    {
                                        // Remove from Current Directory
                                        User::s_CurrentDirectory.directory->RemoveEntity(type, entityIndexOptional, chankIndexEntity);
                                    }
                                    else
                                    {
                                        // Remove from Directory
                                        Directory directoryFrom(chankIndexFrom, driveFrom->m_Drive);
                                        directoryFrom.RemoveEntity(type, entityIndexOptional, chankIndexEntity);
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::cout << "The path can't be same as the target.\n";
                            return;
                        }
                    }
                }
            }
            else
            {
                std::cout << "The directory or file \"" << path << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpMove();
        }
    }

    static void CommandCopy(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 3)
        {
            std::string& path = commandParts[1];
            std::string& target = commandParts[2];

            Drive* driveFrom;
            unsigned int chankIndexFrom; // directory of the entity
            unsigned int chankIndexEntity; // the entity
            std::vector<unsigned int> pathInChanks;
            std::optional<unsigned char> entityIndexOptional;

            EntityType type = GetEntity(path, driveFrom, chankIndexFrom, pathInChanks, entityIndexOptional);
            if (type != EntityType::NotExists)
            {
                if (pathInChanks.empty())
                {
                    std::cout << "Can't move drive.\n";
                    return;
                }

                if (pathInChanks.size() == 1)
                {
                    chankIndexFrom = 0;
                }
                else
                {
                    chankIndexFrom = pathInChanks[pathInChanks.size() - 2];
                }

                chankIndexEntity = pathInChanks[pathInChanks.size() - 1];

                // md target
                Drive* driveTo;
                unsigned int chankIndexTo;

                bool found;
                if (type == EntityType::Directory)
                {
                    found = CreateDirectory(target, false, driveFrom, chankIndexEntity, HelpMove, driveTo, chankIndexTo);
                }
                else
                {
                    found = CreateDirectory(target, false, HelpMove, driveTo, chankIndexTo);
                }

                if (found)
                {
                    if (driveFrom == driveTo && (chankIndexFrom == chankIndexTo || chankIndexEntity == chankIndexTo))
                    {
                        std::cout << "The path can't be same as the target.\n";
                        return;
                    }

                    CopyEntity(driveFrom, chankIndexEntity, type, driveTo, chankIndexTo);
                }
            }
            else
            {
                std::cout << "The directory or file \"" << path << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpCopy();
        }
   }

    static void CommandRename_Rename(Drive*& drive, std::vector<unsigned int>& pathInChanks,
        EntityType& type, std::optional<unsigned char>& entityIndex, unsigned int chankIndex, EntityName& newName)
    {
        const char* error;

        if (type == EntityType::Directory) // Directory
        {
            if (pathInChanks.size() == 1) // In drive
            {
                drive->RenameDirectory(entityIndex, chankIndex, newName, error);
            }
            else // In directory
            {
                unsigned int chankIndexDir = pathInChanks[pathInChanks.size() - 2];
                if (drive == Drive::s_DriveCurrent && chankIndexDir == User::s_CurrentDirectory.directory->m_ChankIndex)
                {
                    User::s_CurrentDirectory.directory->RenameDirectory(entityIndex, chankIndex, newName, error);
                }
                else
                {
                    Directory* directory = new Directory(chankIndexDir, drive->m_Drive);
                    directory->RenameDirectory(entityIndex, chankIndex, newName, error);
                }
            }
        }
        else // File
        {
            if (pathInChanks.size() == 1) // In drive
            {
                drive->RenameFile(entityIndex.value(), newName, error);
            }
            else // In directory
            {
                unsigned int chankIndex = pathInChanks[pathInChanks.size() - 2];
                if (drive == Drive::s_DriveCurrent && chankIndex == User::s_CurrentDirectory.directory->m_ChankIndex)
                {
                    User::s_CurrentDirectory.directory->RenameFile(entityIndex.value(), newName, error);
                }
                else
                {
                    Directory* directory = new Directory(chankIndex, drive->m_Drive);
                    directory->RenameFile(entityIndex.value(), newName, error);
                }
            }
        }

        if (error != nullptr)
        {
            std::cout << error << "\n";
        }
    }

    static void CommandRename(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 3)
        {
            if ((commandParts[2]).size() > MAX_ENTITY_NAME)
            {
                std::cout << "The name can't be more the " << MAX_ENTITY_NAME << " characters.\n";
                return;
            }

            EntityName newName;
            memcpy(&newName, &(commandParts[2])[0], (commandParts[2]).size());
            newName[(commandParts[2]).size()] = 0;

            if (!Drive::CheakEntityName(newName))
            {
                HelpRename();
                return;
            }

            DirectoryBody* directory;
            Drive* drive;
            unsigned int chankIndex;
            std::vector<unsigned int> pathInChanks;
            std::optional<unsigned char> entityIndex;
            EntityType type = GetEntity(commandParts[1], drive, chankIndex, pathInChanks, entityIndex);
            if (type != EntityType::NotExists)
            {
                if (pathInChanks.empty())
                {
                    std::cout << "Can't rename drive.\n";
                    return;
                }

                // Rename Entity
                CommandRename_Rename(drive, pathInChanks, type, entityIndex, chankIndex, newName);
                
                // Update Start line
                if (drive == Drive::s_DriveCurrent)
                {
                    int i = 0;
                    while (i < User::s_CurrentDirectory.path.size())
                    {
                        if (chankIndex == User::s_CurrentDirectory.path[i].m_chankIndex)
                        {
                            User::s_CurrentDirectory.Update(i, newName);
                            break;
                        }
                        else
                        {
                            i++;
                        }
                    }
                }
            }
            else
            {
                std::cout << "The directory or file \"" << commandParts[1] << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpRename();
        }
    }

    static void CommandPrint(std::string& command, std::vector<std::string>& commandParts)
    {
        if (commandParts.size() == 2)
        {
            DirectoryBody* directory;
            Drive* drive;
            unsigned int chankIndex;
            std::optional<unsigned char> fileIndex;
            std::vector<unsigned int> pathInChanks;
            if (GetEntity(commandParts[1], drive, chankIndex, pathInChanks, fileIndex) == EntityType::File)
            {
                File file = File(chankIndex, drive->m_Drive);
                file.Print();
            }
            else
            {
                std::cout << "The file \"" << commandParts[1] << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpPrint();
        }
    }

    static void CommandEdit(std::string& command, std::vector<std::string>& commandParts)
    {
        // Nead code
        if (3 == commandParts.size() || commandParts.size() == 4)
        {
            DirectoryBody* directory;
            Drive* drive;
            unsigned int chankIndex;
            std::optional<unsigned char> fileIndex;
            std::vector<unsigned int> pathInChanks;
            if (GetEntity(commandParts[1], drive, chankIndex, pathInChanks, fileIndex) == EntityType::File)
            {
                File file = File(chankIndex, drive->m_Drive);
                
                // Do Edit
                if (commandParts.size() == 3)
                {
                    if (!commandParts[2].compare("/c") || !commandParts[2].compare("/clean")) // rd [path] [/c | /clean]
                    {
                        file.Resize(0);
                    }
                    else // rd [path] [text]
                    {
                        file.Write(0, commandParts[2]);
                    }
                }
                else
                {
                    if (!commandParts[2].compare("/r") || !commandParts[2].compare("/resize")) // rd [path] [/r | /resize] [size]
                    {
                        unsigned int size = 0;
                        if (TextToNumber(commandParts[3], size))
                        {
                            file.Resize(size);
                        }
                        else
                        {
                            std::cout << "\"" << commandParts[3] << "\" is not number.\n";
                        }
                    }
                    else if (!commandParts[2].compare("/a") || !commandParts[2].compare("/append")) // rd [path] [/a | /append] [text]
                    {
                        file.Write(file.m_Size, commandParts[3]);
                    }
                    else // rd [path] [start] [text]
                    {
                        unsigned int start = 0;
                        if (TextToNumber(commandParts[2], start))
                        {
                            if (start <= file.m_Size)
                            {
                                file.Write(start, commandParts[3]);
                            }
                            else
                            {
                                std::cout << start << " bigger then the file size!\n";
                            }
                        }
                        else
                        {
                            std::cout << "\"" << commandParts[2] << "\" is not number.\n";
                        }
                    }
                }
            }
            else
            {
                std::cout << "The file \"" << commandParts[1] << "\" not found.\n";
                return;
            }
        }
        else
        {
            HelpEdit();
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
            if (helpMode)
            {
                HelpMf();
            }
            else
            {
                CommandMf(command, commandParts);
            }
        }
        else if (!action.compare("rf"))
        {
            if (helpMode)
            {
                HelpRf();
            }
            else
            {
                CommandRf(command, commandParts);
            }
        }
        else if (!action.compare("move"))
        {
            if (helpMode)
            {
                HelpMove();
            }
            else
            {
                CommandMove(command, commandParts);
            }
        }
        else if (!action.compare("copy"))
        {
            if (helpMode)
            {
                HelpCopy();
            }
            else
            {
                CommandCopy(command, commandParts);
            }
        }
        else if (!action.compare("rename"))
        {
            if (helpMode)
            {
                HelpRename();
            }
            else
            {
                CommandRename(command, commandParts);
            }
        }
        else if (!action.compare("print"))
        {
            if (helpMode)
            {
                HelpPrint();
            }
            else
            {
                CommandPrint(command, commandParts);
            }
        }
        else if (!action.compare("edit"))
        {
        if (helpMode)
        {
            HelpEdit();
        }
        else
        {
            CommandEdit(command, commandParts);
        }
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

    static bool DoCommand(std::string& command)
    {
        if (command.size() > MAX_COMMAND_SIZE)
        {
            std::cout << "The cammand can't be bigger then " << MAX_COMMAND_SIZE << "!\n";
            return true;
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
            std::cout << "Command syntex error!\n";
            return true;
        }

        if (partIndex != 0)
        {
            commandPart[partIndex] = 0;
            commandParts.emplace_back(commandPart);
        }

        if (commandParts.empty())
        {
            return true;
        }

        // make command lower ("ECHO" to "echo")
        for (auto& tv : commandParts[0])
        {
            tv = std::tolower(tv);
        }

        return Commands::DoCommand(command, commandParts);
    }

    static bool DoCommand(std::string&& command)
    {
        return Commands::DoCommand(command);
    }

#ifdef _DEBUG
    static void PrintFile(int tabs, unsigned int chankIndex)
    {
        File* file = new File(chankIndex, Drive::s_DriveCurrent->m_Drive);

        for (size_t i = 0; i < tabs; i++)
        {
            std::cout << " ";
        }

        std::cout << file->m_Name.GetName() << " (" << file->m_Size << " Bytes)\n";

        delete file;
    }

    static void PrintDirectory(int tabs, unsigned int chankIndex, bool displayFiles)
    {
        Directory* directory = new Directory(chankIndex, Drive::s_DriveCurrent->m_Drive);

        for (size_t i = 0; i < tabs; i++)
        {
            std::cout << "  ";
        }

        std::cout << directory->m_Name.GetName() << "\n";

        for (int i = 0; i < directory->m_DirectoriesCount; i++)
        {
            PrintDirectory(tabs + 1, directory->m_DirectoriesLocations[i], displayFiles);
        }

        if (displayFiles)
        {
            for (int i = 0; i < directory->m_FilesCount; i++)
            {
                PrintFile(tabs + 1, directory->m_FilesLocations[i]);
            }
        }

        delete directory;
    }

    static void PrintDrive(bool displayFiles)
    {
        std::cout << Drive::s_DriveCurrent->m_DriveName << ":\n";
        
        for (int i = 0; i < Drive::s_DriveCurrent->m_DirectoriesCount; i++)
        {
            PrintDirectory(1, Drive::s_DriveCurrent->m_DirectoriesLocations[i], displayFiles);
        }

        if (displayFiles)
        {
            for (int i = 0; i < Drive::s_DriveCurrent->m_FilesCount; i++)
            {
                PrintFile(1, Drive::s_DriveCurrent->m_FilesLocations[i]);
            }
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
        User::s_CurrentDirectory.directory = Drive::s_DriveCurrent;
        User::s_CurrentDirectory.Change();

        DoCommand("md a/b/c");
        DoCommand("md A/B/C");
        DoCommand("md dir");
        DoCommand("mf file");
        DoCommand("clear");
    }

    void Loop()
    {
        bool running = true;
        while (running)
        {
            PrintDrive(true);
            
            // Get Command
            std::string command;
            User::GetCommand(command);
            
            // Do command
            running = Commands::DoCommand(command);
        }
    }

    void Close()
    {
        if (User::s_CurrentDirectory.IsDirectory())
        {
            delete User::s_CurrentDirectory.directory;
        }
    }
};