#include "pch.h"

#include "Drive.h"
#include "Commands.h"

int main()
{
    using namespace VirtualComputer;

    Utils::Debug::DebugTrace::BeginSession();
    Logger::ChangeLevel(Logger::Level::Warning);

    // Load Compuer

    // Load Drives
    bool haveError = false;
    Drive::LoadDrives(haveError);

    if (haveError)
    {
        std::cin.get();
        return 1;
    }

    //load StartLine
    Commands::Load();

    // Start Runing
    Commands::Loop();

    Utils::Debug::DebugTrace::EndSession();
}