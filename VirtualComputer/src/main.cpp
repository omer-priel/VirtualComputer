#include "pch.h"

int main()
{
    Utils::Debug::DebugTrace::BeginSession();
    std::cout << "Hello World\n";

    std::cout << "Press any key to continue . . .";
    std::cin.get();
    Utils::Debug::DebugTrace::EndSession();
}