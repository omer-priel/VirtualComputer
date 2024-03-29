#pragma once

// Settings
#define MAX_DRIVES 26
#define MAX_DIRECTORIES 255
#define MAX_FILES 255
#define MAX_ENTITY_NAME 22
#define POINTER 0xffffffff // MAX_VALUE of "unsigned int"
#define MAX_FILE_SIZE 0xffffffff // MAX_VALUE of "unsigned int"
#define CHANK_SIZE (MAX_ENTITY_NAME + 2 + ((MAX_DIRECTORIES + MAX_FILES) * 4))
#define DELETED_MEMORY_LIST_SIZE ((CHANK_SIZE - 1) / 4)
#define FIRST_FILE_BODY_SIZE (CHANK_SIZE - MAX_ENTITY_NAME - 4 - 4)
#define MAX_COMMAND_SIZE 114

// Librarys
#include <iostream>

// Data Structures
#include <string>
#include <array>
#include <vector>

// Utils
#include "Utils/Debug/Logger.h"
#include "Utils/Debug/DebugTimer.h"

// using
namespace VirtualComputer
{
	using Logger = Utils::Debug::Logger;
	using DebugTimer = Utils::Debug::DebugTimer;
	using Chank = std::array<char, CHANK_SIZE>;
	using EntityName = std::array<char, MAX_ENTITY_NAME + 1>;
}