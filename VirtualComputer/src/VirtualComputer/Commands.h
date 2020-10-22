#pragma once
namespace VirtualComputer
{
	struct PathItem
	{
		unsigned int m_chankIndex;
		std::string m_Name;

		PathItem(const unsigned int& chankIndex, const std::string& name)
			: m_chankIndex(chankIndex), m_Name(name)
		{

		}
	};
}

namespace VirtualComputer::Commands
{
	void Load();
	void Loop();
	void Close();
};

