#include "pch.h"
#include "DirectoryBody.h"

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
}