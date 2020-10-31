#include "pch.h"
#include "Directory.h"

namespace VirtualComputer
{
    // None-Static
    void Directory::GoToThisChankBody(size_t indexInTheChank)
    {
        m_Drive->GoToChank(m_ChankIndex, MAX_ENTITY_NAME + indexInTheChank);
    }
}