#include "pch.h"
#include "File.h"

namespace VirtualComputer
{
    // Static
    unsigned int File::Create(Drive* drive, const EntityName& name, char* content, size_t size)
    {
        unsigned int chankIndex = drive->GenerateChank();

        Logger::Info("Generate Chank ", chankIndex, " for File \"", &name[0], "\"");

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
                Logger::Info("Delete Chank ", chankIndex);

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

        m_Size = m_Drive->m_FileStream.Read<unsigned int>();

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

                chank->ChankIndex = m_Drive->m_FileStream.Read<unsigned int>();

                m_Drive->GoToChank(chank->ChankIndex);
                auto a = CHANK_SIZE - 4;
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

    void File::Print()
    {
        if (m_Size <= FIRST_FILE_BODY_SIZE)
        {
            std::cout.write(m_FirstBodyChank, m_Size);
        }
        else
        {
            std::cout.write(m_FirstBodyChank, FIRST_FILE_BODY_SIZE);

            size_t size = m_Size - FIRST_FILE_BODY_SIZE;
            for (const auto& chank : m_BodyChanks)
            {
                if (size > CHANK_SIZE - 4)
                {
                    std::cout.write(chank->Body, CHANK_SIZE - 4);
                }
                else
                {
                    std::cout.write(chank->Body, size);
                }
                size -= (CHANK_SIZE - 4);
            }
        }
        std::cout << "\n";
    }

    void File::Resize(unsigned int newSize)
    {
        if (newSize != m_Size)
        {
            m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME);
            m_Drive->m_FileStream.Write(newSize);

            if (newSize < m_Size) // Remove Bytes
            {
                if (newSize <= FIRST_FILE_BODY_SIZE)
                {
                    m_Drive->m_FileStream += FIRST_FILE_BODY_SIZE;
                    m_Drive->m_FileStream.Write<unsigned int>(0);

                    while (!m_BodyChanks.empty())
                    {
                        auto chank = m_BodyChanks.back();
                        m_Drive->DeleteChank(chank->ChankIndex);
                        m_BodyChanks.pop_back();
                        delete chank;
                    }
                }
                else
                {
                    unsigned int chanksNeeded = (newSize - FIRST_FILE_BODY_SIZE);
                    chanksNeeded = chanksNeeded / (CHANK_SIZE - 4) + (chanksNeeded % (CHANK_SIZE - 4) > 0 ? 1 : 0);

                    while (m_BodyChanks.size() > chanksNeeded)
                    {
                        auto chank = m_BodyChanks.back();
                        m_Drive->DeleteChank(chank->ChankIndex);
                        m_BodyChanks.pop_back();
                        delete chank;
                    }

                    m_Drive->GoToChank(m_BodyChanks[chanksNeeded - 1]->ChankIndex, CHANK_SIZE - 4);
                    m_Drive->m_FileStream.Write<unsigned int>(0);
                }
            }
            else // Add Bytes
            {
                std::array<char, CHANK_SIZE - 4> data;
                data.fill(0);
                unsigned int sizeFull = m_Size;
                if (newSize <= FIRST_FILE_BODY_SIZE)
                {
                    m_Drive->m_FileStream += sizeFull;
                    m_Drive->m_FileStream.Write(&data[0], FIRST_FILE_BODY_SIZE - sizeFull);
                }
                else
                {
                    unsigned int chanksNeeded = (newSize - FIRST_FILE_BODY_SIZE);
                    chanksNeeded = chanksNeeded / (CHANK_SIZE - 4) + (chanksNeeded % (CHANK_SIZE - 4) > 0 ? 1 : 0);
                    chanksNeeded -= m_BodyChanks.size();

                    if (sizeFull <= FIRST_FILE_BODY_SIZE)
                    {
                        m_Drive->m_FileStream += sizeFull;
                        if (sizeFull != FIRST_FILE_BODY_SIZE)
                        {
                            m_Drive->m_FileStream.Write(&data[0], FIRST_FILE_BODY_SIZE - sizeFull);
                            sizeFull = FIRST_FILE_BODY_SIZE;
                        }

                        while (chanksNeeded > 0)
                        {
                            unsigned int chankIndex = m_Drive->GenerateChank();
                            Logger::Info("Generate Chank ", chankIndex, " for File \"", m_Name.GetName(), "\"");
                            m_Drive->m_FileStream.Write(chankIndex);
                            
                            m_Drive->GoToChank(chankIndex);
                            FileBodyChank* chank = new FileBodyChank();
                            chank->ChankIndex = chankIndex;
                            memcpy(chank->Body, &data[0], data.size());
                            m_Drive->m_FileStream.Write(&data[0], data.size());

                            m_BodyChanks.push_back(chank);
                            chanksNeeded--;
                        }
                        m_Drive->m_FileStream.Write<unsigned int>(0);
                    }
                    else
                    {
                        unsigned int chankIndex = m_BodyChanks[m_BodyChanks.size() - 1]->ChankIndex;
                        m_Drive->GoToChank(chankIndex);
                        unsigned int i = (sizeFull - FIRST_FILE_BODY_SIZE) % (CHANK_SIZE - 4);
                        if (i != 0)
                        {
                            m_Drive->m_FileStream += i;

                            auto chank = m_BodyChanks[m_BodyChanks.size() - 1];
                            memcpy(&chank->Body[i], &data[0], CHANK_SIZE - 4 - i);
                            m_Drive->m_FileStream.Write(&data[0], CHANK_SIZE - 4 - i);
                        }
                        else
                        {
                            m_Drive->m_FileStream += CHANK_SIZE - 4;
                        }

                        while (chanksNeeded > 0)
                        {
                            chankIndex = m_Drive->GenerateChank();
                            Logger::Info("Generate Chank ", chankIndex, " for File \"", m_Name.GetName(), "\"");
                            m_Drive->m_FileStream.Write(chankIndex);
                            
                            m_Drive->GoToChank(chankIndex);

                            FileBodyChank* chank = new FileBodyChank();
                            chank->ChankIndex = chankIndex;
                            memcpy(chank->Body, &data[0], data.size());
                            m_Drive->m_FileStream.Write(&data[0], data.size());

                            m_BodyChanks.push_back(chank);
                            chanksNeeded--;
                        }
                        m_Drive->m_FileStream.Write<unsigned int>(0);
                    }
                }
            }

            m_Size = newSize;
        }
    }

    void File::Write(unsigned int index, const std::string& text)
    {
        Logger::Debug("Write(", index, ", \"", text, "\")");

        if (index + text.size() > m_Size) // Need to resize
        {
            Resize(index + text.size());
        }

        unsigned int writedSize = 0;

        if (index <= FIRST_FILE_BODY_SIZE)
        {
            m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME + 4 + index);

            if (text.size() < FIRST_FILE_BODY_SIZE - index)
            {
                writedSize = text.size();
            }
            else
            {
                writedSize = FIRST_FILE_BODY_SIZE - index;
            }
            memcpy(m_FirstBodyChank + index, text.c_str(), writedSize);
            m_Drive->m_FileStream.Write(text.c_str(), writedSize);

            index = 0;
        }
        else
        {
            index -= FIRST_FILE_BODY_SIZE;
        }

        unsigned int i = index / (CHANK_SIZE - 4);
        while (writedSize < text.size())
        {
            auto& chank = m_BodyChanks[index / (CHANK_SIZE - 4)];
            unsigned int start = index % (CHANK_SIZE - 4);

            m_Drive->GoToChank(chank->ChankIndex, start);

            if (text.size() - writedSize < (CHANK_SIZE - 4) - start)
            {
                writedSize += text.size();
                memcpy(chank->Body + start, text.c_str(), text.size());
                m_Drive->m_FileStream.Write(text.c_str(), text.size());
            }
            else
            {
                writedSize += (CHANK_SIZE - 4) - start;
                memcpy(chank->Body + start, text.c_str(), (CHANK_SIZE - 4) - start);
                m_Drive->m_FileStream.Write(text.c_str(), (CHANK_SIZE - 4) - start);
            }
            index += (CHANK_SIZE - 4);
        }
    }
}