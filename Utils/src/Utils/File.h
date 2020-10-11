#pragma once
#include "pch.h"

#include <fstream>

namespace Utils
{
	using Byte = unsigned char;

	class File
	{
		// Subclasses
	private:
		enum class FileMode : bool
		{
			Read = false,
			Write = true,
		};

		// Static
	public:
		static bool Exists(const char* path);

		static inline bool Exists(const std::string& path);

		static void Create(const char* path);

		static inline void Create(const std::string& path);

		static void Resize(const char* path, const size_t& size);

		static void Resize(const std::string& path, const size_t& size);

		// None Static
	private:
		std::fstream m_Stream;
		size_t m_Index = 0;
		bool m_IndexChanged = true;
		FileMode m_Mode;

	public:

		//Open, Close, IsClose
		File() { }

		explicit File(const char* path)
		{
			Open(path);
		}

		explicit File(const char* path, const bool createIfNeed)
		{
			Open(path, createIfNeed);
		}

		explicit File(const char* path, const bool createIfNeed, bool& isCreatedNow)
		{
			Open(path, createIfNeed, isCreatedNow);
		}

		explicit File(const std::string& path)
		{
			Open(path.c_str());
		}

		explicit File(const std::string& path, const bool createIfNeed)
		{
			Open(path.c_str(), createIfNeed);
		}

		explicit File(const std::string& path, const bool createIfNeed, bool& isCreatedNow)
		{
			Open(path.c_str(), createIfNeed, isCreatedNow);
		}

		virtual ~File()
		{
			if (IsOpened())
			{
				Close();
			}
		}

		void Open(const char* path, const bool createIfNeed = false);

		void Open(const char* path, bool const createIfNeed, bool& isCreatedNow);

		inline void Open(const std::string& path, const bool createIfNeed = false);

		inline void Open(const std::string& path, const bool createIfNeed, bool& isCreatedNow);

		inline void Close()
		{
			m_Stream.close();
		}

		inline bool IsOpened()
		{
			return m_Stream.is_open();
		}

		// Actions
		inline void Clear();

		inline bool Good();

		size_t Size();

		// Index Action
		size_t GetIndex();

		void ChangeIndex(const size_t& value);

		void operator++();

		void operator++(int);

		void operator+=(const size_t& value);

		void operator--();

		void operator--(int);

		void operator-=(const size_t& value);

	private:
		inline void UpdateIndex();

	public:
		// Read
		Byte Read();

		void Read(char* output, const size_t& size);

		char* ReadArray(const size_t& size);

		template <typename T>
		inline void Read(T& output)
		{
			Read((char*)&output, sizeof(T));
		}

		template <typename T>
		T Read()
		{
			T output;
			Read<T>(output);
			return output;
		}

		// Read from Index
		void Read(const size_t& index, char* output, const size_t& size);

		char* ReadArray(const size_t& index, const size_t& size);

		template <typename T>
		void Read(const size_t& index, T& output)
		{
			ChangeIndex(index);
			Read<T>(output);
		}

		template <typename T>
		T Read(const size_t& index)
		{
			ChangeIndex(index);
			return Read<T>();
		}

		// Write
		void Write(const Byte& value);

		void Write(const char* value, const size_t& size);

		void Write(const char* value);

		void Write(const std::string& value);

		template <typename T>
		inline void Write(const T& value)
		{
			Write((char*)&value, sizeof(T));
		}

		// Write to Index
		void Write(const size_t& index, char* value, const size_t& size);

		void Write(const size_t& index, const char* value);

		template <typename T>
		void Write(const size_t& index, const T& value)
		{
			ChangeIndex(index);
			Write<T>(value);
		}
	};

	// Read operator
	template <typename T>
	File& operator>>(File& file, T& output)
	{
		if constexpr (std::is_same<T, Byte>::value)
		{
			output = file.Read();
		}
		else if constexpr (std::is_same<T, char>::value)
		{
			output = (char)file.Read();
		}
		else
		{
			file.Read<T>(output);
		}

		return file;
	}

	// Write operator
	template <typename T>
	File& operator<<(File& file, const T& value)
	{
		if constexpr (std::is_same<T, Byte>::value)
		{
			file.Write((const Byte)value);
		}
		else if constexpr (std::is_same<T, const Byte>::value)
		{
			file.Write((const Byte)value);
		}
		else if constexpr (std::is_same<T, char>::value)
		{
			file.Write((const Byte)value);
		}
		else if constexpr (std::is_same<T, const char>::value)
		{
			file.Write((const Byte)value);
		}
		else if constexpr (std::is_same<T, const char*>::value)
		{
			file.Write((const char*)value);
		}
		else if constexpr (std::is_same<T, char*>::value)
		{
			file.Write((const char*)value);
		}
		else if constexpr (std::is_same<T, const std::string>::value)
		{
			file.Write((const std::string)value);
		}
		else if constexpr (std::is_same<T, std::string>::value)
		{
			file.Write((const std::string)value);
		}
		else
		{
			file.Write<T>(value);
		}

		return file;
	}
}