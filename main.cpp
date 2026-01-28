#include <bitset>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Chip_emulator.h"



int main()
{
    try
    {
        sf::RenderWindow window(sf::VideoMode({640, 320}), "SFML 3.0 Test");

        //sf::Texture texture; для хранения (типо кадра) для отрисовки экрана
        //sf::Sprite sprite(texture); для отрисовки либо sf::RenderTexture
        sf::Texture texture;

        auto emu = std::make_unique<ChipEmulator>("C:/Users/askoy/Documents/PROJECT/MyEmulator/test.ch8");
        emu->ReadFile();

        for (int i = 0; i < 300; i++) emu->Execute(emu->Decode(emu->Fetch()));
        //ChipEmulator test("C:/Users/askoy/Documents/PROJECT/MyEmulator/test.ch8");

        //test.ReadFile();

        //for (int i = 0; i < 300; i++) test.Execute(test.Decode(test.Fetch()));


        window.setFramerateLimit(60);

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

}