#include "pch.h"
#include "File.h"

// None-Static
void File::LoadBody()
{
    m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);

    // Laod Directories and Files
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

void File::DeleteBody(Drive* drive)
{
    for (FileBodyChank* chank :m_BodyChanks)
    {
        drive->DeleteChank(chank->ChankIndex);
    }
}