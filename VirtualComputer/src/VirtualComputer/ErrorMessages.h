#pragma once

#ifndef _VIRTUAL_COMPUTER_ERROR_MESSAGES_H
#define _VIRTUAL_COMPUTER_ERROR_MESSAGES_H

namespace VirtualComputer::ErrorMessages
{
	const char* const NameAlreadyExist = "This Name already exist!";

	const char* const MaxDirectories = "Can't create more then 255 directories!";
	const char* const MaxFiles = "Can't create more then 255 files!";
}

#endif // _VIRTUAL_COMPUTER_ERROR_MESSAGES_H_H