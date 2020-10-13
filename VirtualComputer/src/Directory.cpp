#include "pch.h"
#include "Directory.h"

// None-Static
void Directory::LoadBody()
{
    // Laod Directories and Files
    m_DirectoriesCount = m_Drive->m_FileStream.Read<unsigned char>();
    m_Drive->m_FileStream.Read((char*)m_DirectoriesLocations, MAX_DIRECTORIES);

    for (unsigned char i = 0; i < m_DirectoriesCount; i++)
    {
        m_Drive->GoToChank(m_DirectoriesLocations[i]);
        m_DirectoriesNames[i].LoadName(m_Drive->m_FileStream);
    }

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

    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);

    m_DirectoriesCount++;
    unsigned char a = m_Drive->m_FileStream.Read<unsigned char >();
    m_Drive->m_FileStream--;
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

void Directory::DeleteMe()
{

}


void Directory::DeleteDirectory(unsigned char directoryIndex)
{

}

void Directory::DeleteDirectory(const EntityName& name)
{

}