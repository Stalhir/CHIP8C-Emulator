#include <bitset>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Chip_emulator.h"



int main()
{
    try
    {
       


        auto emu = std::make_unique<ChipEmulator>("C:/Users/askoy/Desktop/CHIP8/CHIP8C-Emulator/test.ch8");
  
        emu->GameCycle();

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

}