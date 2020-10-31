#include "pch.h"
#include "DirectoryBody.h"

#include "File.h"
#include "ErrorMessages.h"
#include "EntityType.h";

namespace VirtualComputer
{
	// Static
    bool DirectoryBody::CheakEntityName(const EntityName& name)
    {
        if (name.empty())
        {
            Logger::Error("The entity name can't be empty!");
            return false;
        }

        for (char tv : name)
        {
            if (tv == 0)
            {
                break;
            }
            if (tv == ':' || tv == '\\' || tv == '/' || tv == '\'' || tv == '\"' || tv == '\n')
            {
                Logger::Error("The tv \'", tv, "\' can't be in entity name!");
                return false;
            }
        }

        return true;
    }

	// None-Static
    void DirectoryBody::GoToThisChankBody(size_t indexInTheChank)
    {
        m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME + indexInTheChank);
    }

    // Load
    void DirectoryBody::LoadBody()
    {
        // Laod Directories
        GoToThisChankBody();

        m_DirectoriesCount = m_Drive->m_FileStream.Read<unsigned char>();
        m_Drive->m_FileStream.Read((char*)m_DirectoriesLocations, MAX_DIRECTORIES * 4);

        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            m_Drive->GoToChank(m_DirectoriesLocations[i]);
            m_DirectoriesNames[i].LoadName(m_Drive->m_FileStream);
        }

        // Laod Files
        GoToThisChankBody(1 + MAX_DIRECTORIES * 4);        
        
        m_FilesCount = m_Drive->m_FileStream.Read<unsigned char>();
        m_Drive->m_FileStream.Read((char*)m_FilesLocations, MAX_FILES * 4);

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            m_Drive->GoToChank(m_FilesLocations[i]);
            m_FilesNames[i].LoadName(m_Drive->m_FileStream);
        }
    }

    // Info
    bool DirectoryBody::ExistName(const EntityName& name)
    {
        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            if (m_DirectoriesNames[i].IsEqual(name))
            {
                return true;
            }
        }

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            if (m_FilesNames[i].IsEqual(name))
            {
                return true;
            }
        }

        return false;
    }

    // Entity Actions
    void DirectoryBody::AddEntity(const EntityType& type, const unsigned int& chankIndex, const EntityName& name)
    {
        if (type == EntityType::Directory)
        {
            unsigned char index = m_DirectoriesCount;
            GoToThisChankBody();

            m_DirectoriesCount++;
            m_Drive->m_FileStream.Write(m_DirectoriesCount);

            m_DirectoriesLocations[index] = chankIndex;
            m_DirectoriesNames[index].Change(name);
            m_Drive->m_FileStream += index * 4;
            m_Drive->m_FileStream.Write(m_DirectoriesLocations[index]);
        }
        else // File
        {
            m_FilesLocations[m_FilesCount] = chankIndex;
            m_FilesNames[m_FilesCount] = name;

            m_FilesCount++;

            GoToThisChankBody(1 + MAX_DIRECTORIES * 4);
            m_Drive->m_FileStream.Write(m_FilesCount);
            m_Drive->m_FileStream += (m_FilesCount - 1) * 4;
            m_Drive->m_FileStream.Write(chankIndex);
        }
    }

    void DirectoryBody::RemoveEntity(const EntityType& type, const unsigned char& entityIndex)
    {
        if (type == EntityType::Directory)
        {
            m_DirectoriesCount--;

            GoToThisChankBody();
            m_Drive->m_FileStream.Write(m_DirectoriesCount);

            unsigned char lastIndex = m_DirectoriesCount;
            if (entityIndex != lastIndex)
            {
                m_DirectoriesLocations[entityIndex] = m_DirectoriesLocations[lastIndex];
                m_DirectoriesNames[entityIndex] = m_DirectoriesNames[lastIndex];

                m_Drive->m_FileStream += entityIndex * 4;
                m_Drive->m_FileStream.Write<unsigned int>(m_DirectoriesLocations[entityIndex]);
                m_Drive->m_FileStream -= entityIndex * 4;
            }

            m_DirectoriesLocations[lastIndex] = 0;
            m_DirectoriesNames[lastIndex].Clear();

            m_Drive->m_FileStream += lastIndex * 4;
            m_Drive->m_FileStream.Write<unsigned int>(0);
        }
        else // File
        {
            GoToThisChankBody(1 + MAX_DIRECTORIES * 4);
            m_Drive->m_FileStream.Write(m_FilesCount);

            unsigned char lastIndex = m_FilesCount;
            if (entityIndex != lastIndex)
            {
                m_FilesLocations[entityIndex] = m_FilesLocations[lastIndex];
                m_FilesNames[entityIndex] = m_FilesNames[lastIndex];

                m_Drive->m_FileStream += entityIndex * 4;
                m_Drive->m_FileStream.Write<unsigned int>(m_FilesLocations[entityIndex]);
                m_Drive->m_FileStream -= entityIndex * 4;
            }

            m_FilesLocations[lastIndex] = 0;
            m_FilesNames[lastIndex].Clear();

            m_Drive->m_FileStream += lastIndex * 4;
            m_Drive->m_FileStream.Write<unsigned int>(0);
        }
    }

    void DirectoryBody::RemoveEntity(const EntityType& type, const std::optional<unsigned char> entityIndexOptional, const unsigned int& chankIndex)
    {
        if (entityIndexOptional.has_value())
        {
            RemoveEntity(type, entityIndexOptional.value());
        }
        else
        {
            unsigned char entityIndex;
            if (type == EntityType::Directory)
            {
                for (unsigned char i = 0; i < m_DirectoriesCount; i++)
                {
                    if (m_DirectoriesLocations[i] == chankIndex)
                    {
                        RemoveEntity(type, i);
                        return;
                    }
                }
            }
            else
            {
                for (unsigned char i = 0; i < m_FilesCount; i++)
                {
                    if (m_FilesLocations[i] == chankIndex)
                    {
                        RemoveEntity(type, i);
                        return;
                    }
                }
            }
        }
    }

    // Directories Actions
    unsigned int DirectoryBody::CreateDirectory(const EntityName& name, const char*& error)
    {
        error = nullptr;
        if (m_DirectoriesCount == MAX_DIRECTORIES)
        {
            error = ErrorMessages::MaxDirectories;
            return 0;
        }

        if (!DirectoryBody::CheakEntityName(name))
        {
            return 0;
        }

        if (ExistName(name))
        {
            error = ErrorMessages::NameAlreadyExist;
            return 0;
        }

        unsigned int chankIndex = m_Drive->GenerateChank();
        Logger::Info("Generate Chank ", chankIndex, " for Directory \"", &name[0], "\"");

        AddEntity(EntityType::Directory, chankIndex, name);

        m_Drive->GoToChank(chankIndex);

        m_Drive->m_FileStream.Write(&name[0], MAX_ENTITY_NAME);

        std::array<char, CHANK_SIZE - MAX_ENTITY_NAME> data;
        data.fill(0);
        m_Drive->m_FileStream.Write(&data[0], data.size());

        return chankIndex;
    }

    void DirectoryBody::DeleteDirectory(unsigned char directoryIndex, const bool& first)
    {
        unsigned int chankIndex = m_DirectoriesLocations[directoryIndex];

        DirectoryBody directory(chankIndex, m_Drive);
        directory.LoadBody();

        // delete directories
        while (directory.m_DirectoriesCount > 0)
        {
            directory.DeleteDirectory(directory.m_DirectoriesCount - 1, false);
        }

        // delete files
        while (directory.m_FilesCount > 0)
        {
            directory.DeleteFile(directory.m_FilesCount - 1, false);
        }

        m_Drive->DeleteChank(chankIndex);

        if (first)
        {
            RemoveEntity(EntityType::Directory, directoryIndex);
        }
        else
        {
            m_DirectoriesCount--;
        }
    }

    void DirectoryBody::RenameDirectory(std::optional<unsigned char>& directoryIndex, unsigned int chankIndex, const EntityName& name, const char*& error)
    {
        error = nullptr;
        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            if (m_FilesNames[i].IsEqual(name))
            {
                error = ErrorMessages::NameAlreadyExist;
                return;
            }
        }

        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            if (m_DirectoriesNames[i].IsEqual(name))
            {
                if (i != directoryIndex)
                {
                    error = ErrorMessages::NameAlreadyExist;
                }
                return;
            }

            if (!directoryIndex.has_value() && chankIndex == m_DirectoriesLocations[i])
            {
                directoryIndex.emplace(i);
            }
        }

        m_DirectoriesNames[directoryIndex.value()].Change(name);

        m_Drive->GoToChank(chankIndex);
        m_Drive->m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
    }

    // Files Actions
    unsigned int DirectoryBody::CreateFile(const EntityName& name, char* content, size_t size, const char*& error)
    {
        error = nullptr;
        if (m_FilesCount == MAX_FILES)
        {
            error = ErrorMessages::MaxFiles;
            return 0;
        }

        if (!DirectoryBody::CheakEntityName(name))
        {
            return 0;
        }

        if (ExistName(name))
        {
            error = ErrorMessages::NameAlreadyExist;
            return 0;
        }

        unsigned int chankIndex = File::Create(m_Drive, name, content, size);

        AddEntity(EntityType::File, chankIndex, name);

        return chankIndex;
    }

    void DirectoryBody::DeleteFile(unsigned char fileIndex, const bool& first)
    {
        unsigned int chankIndex = m_FilesLocations[fileIndex];

        File::DeleteFile(m_Drive, chankIndex);

        if (first)
        {
            RemoveEntity(EntityType::File, fileIndex);
        }
        else
        {
            m_FilesCount--;
        }
    }

    void DirectoryBody::RenameFile(unsigned char fileIndex, const EntityName& name, const char*& error)
    {
        error = nullptr;
        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            if (m_DirectoriesNames[i].IsEqual(name))
            {
                error = ErrorMessages::NameAlreadyExist;
                return;
            }
        }

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            if (m_FilesNames[i].IsEqual(name))
            {
                if (i != fileIndex)
                {
                    error = ErrorMessages::NameAlreadyExist;
                }
                return;
            }
        }

        unsigned int chankIndex = m_FilesLocations[fileIndex];
        m_FilesNames[fileIndex].Change(name);

        m_Drive->GoToChank(chankIndex);
        m_Drive->m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
    }
}