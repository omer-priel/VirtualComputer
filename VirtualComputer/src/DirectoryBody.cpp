#include "pch.h"
#include "DirectoryBody.h"

#include "Utils/File.h"

void DirectoryBody::LoadBody(Utils::File& fileStream)
{
	m_DirectoriesCount = fileStream.Read<unsigned char>();
	fileStream.Read(m_DirectoriesLocations);

	m_FilesCount = fileStream.Read<unsigned char>();
	fileStream.Read(m_FilesLocations);
}