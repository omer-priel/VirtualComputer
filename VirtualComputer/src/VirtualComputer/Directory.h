#pragma once

#include "DirectoryBody.h"
#include "SmartEntityName.h"
#include "Drive.h"

namespace VirtualComputer
{
	class Directory : public DirectoryBody
	{
		// None-Static
	private:
		Drive* m_Drive;

	public:
		SmartEntityName m_Name;

	public:
		Directory(const unsigned int& chankIndex, Drive* drive)
			: m_Drive(drive)
		{
			m_ChankIndex = chankIndex;

			// Load Name
			drive->GoToChank(m_ChankIndex);
			m_Name.LoadName(drive->m_FileStream);

			LoadBody();
		}

		void LoadBody();

		bool ExistName(const EntityName& name);

		unsigned int CreateDirectory(const EntityName& name, const char*& error);

		void Delete();

		void DeleteDirectory(unsigned char directoryIndex, const bool& first = true);
		void DeleteDirectory(const EntityName& name);

		void RenameDirectory(std::optional<unsigned char>& directoryIndex, unsigned int chankIndex, const EntityName& name, const char*& error);

		unsigned int CreateFile(const EntityName& name, char* content, size_t size, const char*& error);

		void DeleteFile(unsigned char directoryIndex, const bool& first = true);

		void RenameFile(unsigned char fileIndex, const EntityName& name, const char*& error);

		void AddEntity(const EntityType& type, const unsigned int& chankIndex, const EntityName& name);

		void RemoveEntity(const EntityType& type, const unsigned char& entityIndex);

		void RemoveEntity(const EntityType& type, const std::optional<unsigned char> entityIndexOptional, const unsigned int& chankIndex);
	};
}