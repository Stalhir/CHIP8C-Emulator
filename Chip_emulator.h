#pragma once
#include <fstream>
#include <array>
#include <stack>
#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>


enum class InstructionType {
    CLS,
    RET,
    JP,
    CALL,
    JP_V0,

    SE_VX_BYTE,
    SNE_VX_BYTE,
    SE_VX_VY,
    SNE_VX_VY,

    LD_VX_BYTE,
    ADD_VX_BYTE,
    LD_VX_VY,
    OR_VX_VY,
    AND_VX_VY,
    XOR_VX_VY,
    ADD_VX_VY,
    SUB_VX_VY,
    SHR_VX,
    SUBN_VX_VY,
    SHL_VX,

    LD_I_ADDR,
    LD_B_VX,
    ADD_I_VX,

    DRW,
    SKP_VX,
    SKNP_VX,
    LD_VX_DT,
    LD_VX_K,
    LD_DT_VX,
    LD_ST_VX,
    RND_VX_BYTE,

    LD_F_VX,
    LD_I_VX,
    LD_VX_I,

    UKNOWN
};

struct DecodedInstruction
{
    InstructionType type;
    uint16_t nnn;
    uint8_t x;
    uint8_t y;
    uint8_t kk;
};





class ChipEmulator
{
    private:
    std::array<uint8_t, 4096> memory;
    std::array<uint16_t, 16> stack;
    std::array<uint8_t, 16> registers;
    std::vector<std::vector<uint8_t>> FrameBuffer;  

    uint16_t programcounter = 0x200;
    uint16_t indexRegister = 0; 
    uint8_t stackpointer = 0;

    uint8_t DelayTimer  = 0;
    uint8_t SoundTimer  = 0;
     
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite; 
    bool NeedDraw;

    std::ifstream file;

    bool CheckKeyPressed(uint8_t keynumber);

    uint8_t ReadKey();

    public:
   

    void ReadFile();

    uint16_t Fetch();

    DecodedInstruction Decode(uint16_t opcode);

    void Execute(DecodedInstruction instruction);

    void Draw();

    void GameCycle();

    ChipEmulator(std::string filePath);

    ~ChipEmulator();


};

