#include "pch.h"
#include "File.h"

#include <filesystem>
#include <sys/stat.h>

namespace Utils
{
	// Static
	bool File::Exists(const char* path)
	{
		// stat - from sys/stat.h
		struct stat buffer;
		return stat(path, &buffer) == 0;
	}

	inline bool File::Exists(const std::string& path)
	{
		return File::Exists(path.c_str());
	}

	void File::Create(const char* path)
	{
		std::fstream stream(path, std::ios::out);
	}

	inline void File::Create(const std::string& path)
	{
		File::Create(path.c_str());
	}

	bool File::Delete(const char* path)
	{
		return remove(path) == 0;
	}

	bool File::Delete(const std::string& path)
	{
		return remove(path.c_str()) == 0;
	}

	void File::Resize(const char* path, const size_t& size)
	{
		std::filesystem::resize_file(path, size);
	}

	void File::Resize(const std::string& path, const size_t& size)
	{
		std::filesystem::resize_file(path, size);
	}

	// None Static
	void File::Open(const char* path, const bool createIfNeed)
	{
		if (createIfNeed && !Exists(path))
		{
			Create(path);
		}

		m_Stream.open(path, std::ios::in | std::ios::out | std::ios::binary);
		m_IndexChanged = true;
	}

	void File::Open(const char* path, bool const createIfNeed, bool& isCreatedNow)
	{
		if (createIfNeed && !Exists(path))
		{
			isCreatedNow = true;
			Create(path);
		}
		else
		{
			isCreatedNow = false;
		}

		m_Stream.open(path, std::ios::in | std::ios::out | std::ios::binary);
		m_IndexChanged = true;
	}

	void File::Open(const std::string& path, const bool createIfNeed)
	{
		Open(path.c_str(), createIfNeed);
	}

	inline void File::Open(const std::string& path, const bool createIfNeed, bool& isCreatedNow)
	{
		Open(path.c_str(), createIfNeed, isCreatedNow);
	}

	// Actions
	inline void File::Clear()
	{
		m_Stream.clear();
	}

	inline bool File::Good()
	{
		return m_Stream.good();
	}

	size_t File::Size()
	{
		Clear();

		m_Stream.seekg(0, std::ios::end);
		size_t size = m_Stream.tellg();

		m_Stream.seekg(m_Index, std::ios::beg);

		return size;
	}

	// Index Action
	size_t File::GetIndex()
	{
		return m_Index;
	}

	void File::ChangeIndex(const size_t& value)
	{
		m_Index = value;
		m_IndexChanged = true;
	}

	void File::operator++()
	{
		UpdateIndex();
		m_Index++;
		m_Stream.seekg(1, std::ios::cur);
	}

	void File::operator++(int)
	{
		UpdateIndex();
		m_Index++;
		m_Stream.seekg(1, std::ios::cur);
	}

	void File::operator+=(const size_t& value)
	{
		UpdateIndex();
		m_Index += value;
		m_Stream.seekg(value, std::ios::cur);
	}

	void File::operator--()
	{
		UpdateIndex();
		m_Index--;
		m_Stream.seekg(-1, std::ios::cur);
	}

	void File::operator--(int)
	{
		UpdateIndex();
		m_Index--;
		m_Stream.seekg(1, std::ios::cur);
	}

	void File::operator-=(const size_t& value)
	{
		UpdateIndex();
		m_Index -= value;
		m_Stream.seekg(0 - value, std::ios::cur);
	}

	inline void File::UpdateIndex()
	{
		if (m_IndexChanged)
		{
			m_Stream.seekg(m_Index, std::ios::beg);
			m_IndexChanged = false;
		}
	}

	// Read
	Byte File::Read()
	{
		if (m_Mode != FileMode::Read)
		{
			Clear();
			m_Mode = FileMode::Read;
		}

		UpdateIndex();

		Byte byte;
		m_Stream.read((char*)&byte, 1);

		m_Index++;

		return byte;
	}

	void File::Read(char* output, const size_t& size)
	{
		if (m_Mode != FileMode::Read)
		{
			Clear();
			m_Mode = FileMode::Read;
		}

		UpdateIndex();

		m_Stream.read(output, size);

		m_Index += size;
	}

	char* File::ReadArray(const size_t& size)
	{
		char* output = new char[size];
		Read(output, size);

		return output;
	}

	// Read from Index
	void File::Read(const size_t& index, char* output, const size_t& size)
	{
		ChangeIndex(index);
		return Read(output, size);
	}

	char* File::ReadArray(const size_t& index, const size_t& size)
	{
		ChangeIndex(index);
		return ReadArray(size);
	}

	// Write
	void File::Write(const Byte& value)
	{
		if (m_Mode != FileMode::Write)
		{
			Clear();
			m_Mode = FileMode::Write;
		}

		UpdateIndex();

		m_Stream.write((char*)&value, 1);

		m_Index++;
	}

	void File::Write(const char* value, const size_t& size)
	{
		if (m_Mode != FileMode::Write)
		{
			Clear();
			m_Mode = FileMode::Write;
		}

		UpdateIndex();

		m_Stream.write(value, size);

		m_Index += size;
	}

	void File::Write(const char* value)
	{
		Write(value, strlen(value));
	}

	void File::Write(const std::string& value)
	{
		Write(value.c_str(), value.size());
	}

	// Write to Index
	void File::Write(const size_t& index, char* value, const size_t& size)
	{
		ChangeIndex(index);
		Write(value, size);
	}

	void File::Write(const size_t& index, const char* value)
	{
		ChangeIndex(index);
		Write(value, strlen(value));
	}
}