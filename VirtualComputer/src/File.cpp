#include "pch.h"
#include "File.h"

// Static
unsigned int File::Create(Drive* drive, const EntityName& name, unsigned int size)
{
    
}

void File::DeleteFileBody(Drive* drive, unsigned int chankIndex)
{
    drive->GoToChank(chankIndex, MAX_ENTITY_NAME);
    
    unsigned int size = drive->m_FileStream.Read<unsigned char>();

    drive->DeleteChank(chankIndex);

    if (size > FIRST_FILE_BODY_SIZE)
    {
        drive->m_FileStream += FIRST_FILE_BODY_SIZE;
        chankIndex = drive->m_FileStream.Read<unsigned char>();
        size -= FIRST_FILE_BODY_SIZE;

        while (size > 0)
        {
            drive->GoToChank(chankIndex, CHANK_SIZE - 4);
            drive->DeleteChank(chankIndex);

            if (CHANK_SIZE - 4 > size)
            {
                break;
            }
            else
            {
                size -= CHANK_SIZE - 4;
                chankIndex = drive->m_FileStream.Read<unsigned char>();
            }
        }
    }
}

// None-Static
void File::LoadBody()
{
    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);

    m_Size = m_Drive->m_FileStream.Read<unsigned char>();

    if (m_Size < FIRST_FILE_BODY_SIZE)
    {
        m_Drive->m_FileStream.Read(m_FirstBodyChank, m_Size);
    }
    else
    {
        m_Drive->m_FileStream.Read(m_FirstBodyChank, FIRST_FILE_BODY_SIZE);
        
        unsigned int neadToLoad = m_Size - FIRST_FILE_BODY_SIZE;
        while (neadToLoad > 0)
        {
            FileBodyChank* chank = new FileBodyChank;
            
            chank->ChankIndex = m_Drive->m_FileStream.Read<unsigned char>();
            
            m_Drive->GoToChank(chank->ChankIndex);
            if (neadToLoad <= CHANK_SIZE - 4)
            {
                m_Drive->m_FileStream.Read(chank->Body, neadToLoad);
                neadToLoad = 0;
            }
            else
            {
                m_Drive->m_FileStream.Read(chank->Body, CHANK_SIZE - 4);
                neadToLoad -= (CHANK_SIZE - 4);
            }

            m_BodyChanks.push_back(chank);
        }
    }
}