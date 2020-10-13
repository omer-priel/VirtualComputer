#include "pch.h"
#include "Directory.h"

// None-Static
void Directory::LoadBody()
{
    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);

    // Laod Directories and Files
    m_DirectoriesCount = m_Drive->m_FileStream.Read<unsigned char>();
    m_Drive->m_FileStream.Read((char*)m_DirectoriesLocations, MAX_DIRECTORIES);

    for (unsigned char i = 0; i < m_DirectoriesCount; i++)
    {
        m_Drive->GoToChank(m_DirectoriesLocations[i]);
        m_DirectoriesNames[i].LoadName(m_Drive->m_FileStream);
    }

    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME + 1 + MAX_DIRECTORIES * 4);
    m_FilesCount = m_Drive->m_FileStream.Read<unsigned char>();
    m_Drive->m_FileStream.Read((char*)m_FilesLocations, MAX_FILES);

    for (unsigned char i = 0; i < m_FilesCount; i++)
    {
        m_Drive->GoToChank(m_FilesLocations[i]);
        m_FilesNames[i].LoadName(m_Drive->m_FileStream);
    }
}

unsigned int Directory::CreateDirectory(const EntityName& name)
{
    if (m_DirectoriesCount == MAX_DIRECTORIES)
    {
        Logger::Error("Can't create Directorie");
        return 0;
    }

    for (unsigned char i = 0; i < m_DirectoriesCount; i++)
    {
        if (m_DirectoriesNames[i].IsEqual(name))
        {
            Logger::Error("This Name already exist!");
            return 0;
        }
    }

    unsigned char index = m_DirectoriesCount;
    unsigned int chankIndex = m_Drive->GenerateChank();
    Logger::Info("Directory \"", m_Name.GetName(), "\" Generate Chank ", chankIndex, " for Directory \"", &name[0], "\"");

    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);

    m_DirectoriesCount++;
    m_Drive->m_FileStream.Write(m_DirectoriesCount);

    m_DirectoriesLocations[index] = chankIndex;
    m_DirectoriesNames[index].Change(name);
    m_Drive->m_FileStream += index * 4;
    m_Drive->m_FileStream.Write(m_DirectoriesLocations[index]);

    m_Drive->GoToChank(chankIndex);

    m_Drive->m_FileStream.Write(&name[0], MAX_ENTITY_NAME);

    std::array<char, CHANK_SIZE - MAX_ENTITY_NAME> data;
    data.fill(0);
    m_Drive->m_FileStream.Write(&data[0], data.size());

    return chankIndex;
}

void Directory::Delete()
{
    // delete directories
    while (m_DirectoriesCount > 0)
    {
        DeleteDirectory(m_DirectoriesCount - 1, false);
    }

    // delete files
    while (m_FilesCount > 0)
    {
        DeleteFile(m_FilesCount - 1, false);
    }
}


void Directory::DeleteDirectory(unsigned char directoryIndex, const bool& first)
{
    unsigned int chankIndex = m_DirectoriesLocations[directoryIndex];

    Directory directory(chankIndex, m_Drive);
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

    m_DirectoriesCount--;

    if (first)
    {
        unsigned char lastIndex = m_DirectoriesCount;
        if (directoryIndex != lastIndex)
        {
            m_DirectoriesLocations[directoryIndex] = m_DirectoriesLocations[lastIndex];
            m_DirectoriesNames[directoryIndex] = m_DirectoriesNames[lastIndex];
        }

        m_DirectoriesLocations[lastIndex] = 0;
        m_DirectoriesNames[lastIndex].Clear();

        m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);
        m_Drive->m_FileStream.Write(m_DirectoriesCount);

        m_Drive->m_FileStream += lastIndex * 4;
        m_Drive->m_FileStream.Write<unsigned int>(0);
    }
}

void Directory::DeleteDirectory(const EntityName& name)
{

}

void Directory::DeleteFile(unsigned char fileIndex, const bool& first)
{
    unsigned int chankIndex = m_FilesLocations[fileIndex];

    // Need Code
    m_Drive->DeleteChank(chankIndex);

    m_FilesCount--;
    if (first)
    {
        unsigned char lastIndex = m_FilesCount;
        if (fileIndex != lastIndex)
        {
            m_FilesLocations[fileIndex] = m_FilesLocations[lastIndex];
            m_FilesNames[fileIndex] = m_FilesNames[lastIndex];
        }

        m_FilesLocations[lastIndex] = 0;
        m_FilesNames[lastIndex].Clear();

        m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME + (1 + MAX_DIRECTORIES * 4));
        m_Drive->m_FileStream.Write(m_FilesCount);

        m_Drive->m_FileStream += lastIndex * 4;
        m_Drive->m_FileStream.Write<unsigned int>(0);
    }
}

void Directory::DeleteFile(const EntityName& name)
{

}