#include "pch.h"
#include "File.h"

namespace VirtualComputer
{
    // Static
    unsigned int File::Create(Drive* drive, const EntityName& name, char* content, size_t size)
    {
        unsigned int chankIndex = drive->GenerateChank();

        drive->GoToChank(chankIndex);
        drive->m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
        drive->m_FileStream.Write(size);

        if (size <= FIRST_FILE_BODY_SIZE)
        {
            drive->m_FileStream.Write(&content[0], size);
            if (size != FIRST_FILE_BODY_SIZE)
            {
                std::array<char, FIRST_FILE_BODY_SIZE> data;
                memset(&data[0], 0, FIRST_FILE_BODY_SIZE - size);
                drive->m_FileStream.Write(&data[0], FIRST_FILE_BODY_SIZE - size);
            }
        }
        else
        {
            drive->m_FileStream.Write(&content[0], FIRST_FILE_BODY_SIZE);

            size -= FIRST_FILE_BODY_SIZE;
            content += FIRST_FILE_BODY_SIZE;

            unsigned chankIndexNow = chankIndex;
            while (size > 0)
            {
                unsigned int nextChankIndex = drive->GenerateChank();

                drive->GoToChank(chankIndexNow, CHANK_SIZE - 4);
                drive->m_FileStream.Write<unsigned int>(nextChankIndex);

                chankIndexNow = nextChankIndex;

                drive->GoToChank(chankIndexNow);
                if (size > CHANK_SIZE - 4)
                {
                    drive->m_FileStream.Write(&content[0], CHANK_SIZE - 4);
                    size -= CHANK_SIZE - 4;
                }
                else
                {
                    drive->m_FileStream.Write(&content[0], size);
                    if (size != CHANK_SIZE - 4)
                    {
                        std::array<char, CHANK_SIZE - 4> data;
                        memset(&data[0], 0, CHANK_SIZE - 4 - size);
                        drive->m_FileStream.Write(&data[0], CHANK_SIZE - 4 - size);
                    }

                    size = 0;
                }
            }
        }
        drive->m_FileStream.Write<unsigned int>(0);

        return chankIndex;
    }

    void File::DeleteFile(Drive* drive, unsigned int chankIndex)
    {
        drive->GoToChank(chankIndex, MAX_ENTITY_NAME);

        unsigned int size = drive->m_FileStream.Read<unsigned int>();

        drive->DeleteChank(chankIndex);

        if (size > FIRST_FILE_BODY_SIZE)
        {
            drive->m_FileStream += FIRST_FILE_BODY_SIZE;

            chankIndex = drive->m_FileStream.Read<unsigned int>();
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
                    chankIndex = drive->m_FileStream.Read<unsigned int>();
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
}