#include <bitset>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Chip_emulator.h"



int main()
{
    try
    {
        sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3.0 Test");

        //sf::Texture texture; для хранения (типо кадра) для отрисовки экрана
        //sf::Sprite sprite(texture); для отрисовки либо sf::RenderTexture
        sf::Texture texture;


        ChipEmulator test("C:/Users/askoy/Documents/PROJECT/MyEmulator/test.ch8");

        test.ReadFile();

        window.setFramerateLimit(60);

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

}