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
        //for (int i = 0; i < 300; i++) emu->Execute(emu->Decode(emu->Fetch()));
        //ChipEmulator test("C:/Users/askoy/Documents/PROJECT/MyEmulator/test.ch8");

        //test.ReadFile();

        //for (int i = 0; i < 300; i++) test.Execute(test.Decode(test.Fetch()));

        
            
        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

}