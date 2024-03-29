#pragma once

namespace VirtualComputer
{
	const char* s_HelpBody = R"V0G0N(
	exit    - Exit the program
	help    - Show all the commands
	drives  - Show drives list
	dir     - Display directory
	md      - Create directory
	rd      - Remove directory
	cd      - Change directory
	mf      - Create file
	rf      - Remove file
	move    - Move file or directory
	copy    - File or directory
	rename  - Rename file or directory
	print   - Display file
	edit    - Edit file
	echo    - Print text
	clear   - Clear the window.

)V0G0N";

	char* HelpBody()
	{
		return (char*)(s_HelpBody + 1);
	}
}