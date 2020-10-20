#pragma once
namespace VirtualComputer::Commands
{
	bool DoCommand(std::string& command, std::vector<std::string>& commandParts);

	void Help();
};

