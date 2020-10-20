#pragma once
namespace VirtualComputer::Commands
{
	void Load();
	void Loop();

	bool DoCommand(std::string& command, std::vector<std::string>& commandParts);

	void Help();
};

