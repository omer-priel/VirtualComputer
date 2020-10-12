#include "pch.h"
#include "DirectoryBody.h"

#include "Utils/File.h"

void DirectoryBody::LoadDirectoryBody(Utils::File& fileStream)
{
	if (!m_DirectoryBodyLoaded)
	{
		m_DirectoriesCount = fileStream.Read<unsigned char>();
		fileStream.Read(m_DirectoriesLocations);

		m_FilesCount = fileStream.Read<unsigned char>();
		fileStream.Read(m_FilesLocations);

		m_DirectoryBodyLoaded = true;
	}
}