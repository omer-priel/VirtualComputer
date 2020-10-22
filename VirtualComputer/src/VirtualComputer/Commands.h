#pragma once
namespace VirtualComputer::Commands
{
	void Load();
	void Loop();
	void Close();

	bool DoCommand(std::string& command, std::vector<std::string>& commandParts);

	void Help();
};

