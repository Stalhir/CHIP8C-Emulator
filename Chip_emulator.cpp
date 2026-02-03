#include "Chip_emulator.h"
#include <vector>
#include <iostream>

ChipEmulator::ChipEmulator(std::string filePath)  : file(filePath, std::ios::binary), FrameBuffer(32, std::vector<uint8_t>(64, 0)),
window(sf::VideoMode(sf::Vector2u(640, 320)), "CHIP-8 Emulator")
, texture()
, sprite(texture)
{
    std::fill(memory.begin(), memory.end(), 0);
    std::fill(registers.begin(), registers.end(), 0);
    std::fill(stack.begin(), stack.end(), 0);

    uint8_t chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, 
        0x20, 0x60, 0x20, 0x20, 0x70, 
        0xF0, 0x10, 0xF0, 0x80, 0xF0, 
        0xF0, 0x10, 0xF0, 0x10, 0xF0, 
        0x90, 0x90, 0xF0, 0x10, 0x10, 
        0xF0, 0x80, 0xF0, 0x10, 0xF0, 
        0xF0, 0x80, 0xF0, 0x90, 0xF0, 
        0xF0, 0x10, 0x20, 0x40, 0x40, 
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 
        0xF0, 0x90, 0xF0, 0x10, 0xF0, 
        0xF0, 0x90, 0xF0, 0x90, 0x90, 
        0xE0, 0x90, 0xE0, 0x90, 0xE0, 
        0xF0, 0x80, 0x80, 0x80, 0xF0, 
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0, 
        0xF0, 0x80, 0xF0, 0x80, 0x80  
    };
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    ReadFile();
  

    window.setFramerateLimit(60);

  
    sprite.setScale(sf::Vector2f(10, 10));


    srand(static_cast<unsigned>(time(nullptr)));

    std::cout << "CHIP-8 Emulator initialized. PC: " << programcounter << std::endl;
}

void ChipEmulator::ReadFile()
{
    int address = 0x200;

    char byte;
    while (file.get(byte) && address < 4096) {
        memory[address++] = static_cast<uint8_t>(byte);
    }

    std::cout << "Loaded " << (address - 0x200) << " bytes from ROM." << std::endl;
}


uint16_t ChipEmulator::Fetch()
{
    if (programcounter >= 4096 - 1) {
        throw std::runtime_error("PC out of bounds");
    }
    uint16_t opcode = (memory.at(programcounter) << 8) | memory.at(programcounter+1);
    programcounter += 2;
    return opcode;
}


DecodedInstruction ChipEmulator::Decode(uint16_t opcode)
{
DecodedInstruction inst;
    inst.x   = (opcode & 0x0F00) >> 8;
    inst.y   = (opcode & 0x00F0) >> 4;
    inst.nnn = (opcode & 0x0FFF);
    inst.kk = (opcode & 0x00FF);
    uint8_t n     = (opcode & 0x000F);

    uint8_t nibble = (opcode & 0xF000) >> 12;

    switch (nibble) {
        case 0x0:
            if (opcode == 0x00E0) inst.type = InstructionType::CLS;
            else if (opcode == 0x00EE) inst.type = InstructionType::RET;
            break;

        case 0x1: inst.type = InstructionType::JP; break;
        case 0x2: inst.type = InstructionType::CALL; break;
        case 0x3: inst.type = InstructionType::SE_VX_BYTE; break;
        case 0x4: inst.type = InstructionType::SNE_VX_BYTE; break;
        case 0x5: inst.type = InstructionType::SE_VX_VY; break;
        case 0x6: inst.type = InstructionType::LD_VX_BYTE; break;
        case 0x7: inst.type = InstructionType::ADD_VX_BYTE; break;

        case 0x8: // ГРУППА АРИФМЕТИКИ (смотрим на последнюю цифру N)
            switch (n) {
                case 0x0: inst.type = InstructionType::LD_VX_VY;   break;
                case 0x1: inst.type = InstructionType::OR_VX_VY;   break;
                case 0x2: inst.type = InstructionType::AND_VX_VY;  break;
                case 0x3: inst.type = InstructionType::XOR_VX_VY;  break;
                case 0x4: inst.type = InstructionType::ADD_VX_VY;  break;
                case 0x5: inst.type = InstructionType::SUB_VX_VY;  break;
                case 0x6: inst.type = InstructionType::SHR_VX;      break;
                case 0x7: inst.type = InstructionType::SUBN_VX_VY; break;
                case 0xE: inst.type = InstructionType::SHL_VX;      break;
            }
            break;

        case 0x9: inst.type = InstructionType::SNE_VX_VY; break;
        case 0xA: inst.type = InstructionType::LD_I_ADDR; break;
        case 0xB: inst.type = InstructionType::JP_V0;    break;
        case 0xC: inst.type = InstructionType::RND_VX_BYTE; break;
        case 0xD: inst.type = InstructionType::DRW;       break;

        case 0xE: // ГРУППА ВВОДА (смотрим на весь последний байт KK)
            if (inst.kk == 0x9E) inst.type = InstructionType::SKP_VX;
            else if (inst.kk == 0xA1) inst.type = InstructionType::SKNP_VX;
            break;

        case 0xF: // ГРУППА ТАЙМЕРОВ И ПАМЯТИ (смотрим на байт KK)
            switch (inst.kk) {
                case 0x07: inst.type = InstructionType::LD_VX_DT; break;
                case 0x0A: inst.type = InstructionType::LD_VX_K;  break;
                case 0x15: inst.type = InstructionType::LD_DT_VX; break;
                case 0x18: inst.type = InstructionType::LD_ST_VX; break;
                case 0x1E: inst.type = InstructionType::ADD_I_VX; break;
                case 0x29: inst.type = InstructionType::LD_F_VX;  break;
                case 0x33: inst.type = InstructionType::LD_B_VX;  break; 
                case 0x55: inst.type = InstructionType::LD_I_VX;  break;
                case 0x65: inst.type = InstructionType::LD_VX_I;  break;
            }
            break;
        default:
            inst.type = InstructionType::UKNOWN; break;
    }

    return inst;
}



void ChipEmulator::Execute(DecodedInstruction instruction)
{
switch (instruction.type)
{
case InstructionType::CLS: for (int i = 0; i < 32; i++) { FrameBuffer.at(i).assign(64, 0); };   NeedDraw = true;
        break;
    case InstructionType::RET:if (stackpointer == 0) throw std::runtime_error("Stack underflow");
        programcounter = stack[--stackpointer];
        break;

    case InstructionType::JP: programcounter = instruction.nnn; break;
    case InstructionType::CALL: if (stackpointer >= stack.size()) throw std::runtime_error("Stack overflow"); stack[stackpointer] = programcounter;
        stackpointer++; programcounter = instruction.nnn ;
    break;
    case InstructionType::SE_VX_BYTE: if (registers[instruction.x] == instruction.kk) programcounter+=2; break;
    case InstructionType::SNE_VX_BYTE: if (registers[instruction.x] != instruction.kk) programcounter+=2; break;
    case InstructionType::SE_VX_VY: if (registers[instruction.x] == registers[instruction.y]) programcounter+=2; break;
    case InstructionType::LD_VX_BYTE: registers[instruction.x] = instruction.kk; break;
    case InstructionType::ADD_VX_BYTE: registers[instruction.x] += instruction.kk; break;

    case InstructionType::LD_VX_VY: registers[instruction.x] = registers[instruction.y];
        break;
    case InstructionType::OR_VX_VY: registers[instruction.x] = (registers[instruction.x] | registers[instruction.y]);
        break;
    case InstructionType::AND_VX_VY: registers[instruction.x] = (registers[instruction.x] & registers[instruction.y]);
        break;
    case InstructionType::XOR_VX_VY: registers[instruction.x] = (registers[instruction.x] ^ registers[instruction.y]);
        break;
    case InstructionType::ADD_VX_VY: {
        uint16_t sum = registers[instruction.x] + registers[instruction.y];
        uint8_t carry = (sum > 0xFF) ? 1 : 0;
        registers[instruction.x] = sum & 0xFF;
        registers[0xF] = carry;
        break;
    }
    case InstructionType::SUB_VX_VY: {
        uint8_t not_borrow = (registers[instruction.x] >= registers[instruction.y]) ? 1 : 0;

        registers[instruction.x] -= registers[instruction.y];

        registers[0xF] = not_borrow;
        break;
    }
    case InstructionType::SHR_VX: {
        uint8_t lsb = registers[instruction.x] & 0x01;
        registers[instruction.x] >>= 1;
        registers[0xF] = lsb;
        break;
    }
    case InstructionType::SUBN_VX_VY: {
        uint8_t not_borrow = (registers[instruction.y] >= registers[instruction.x]) ? 1 : 0;

        registers[instruction.x] = registers[instruction.y] - registers[instruction.x];

        registers[0xF] = not_borrow;
        break;
    }
    case InstructionType::SHL_VX: {
        uint8_t msb = (registers[instruction.x] & 0x80) >> 7;
        registers[instruction.x] <<= 1;
        registers[0xF] = msb;
        break;
    }


    case InstructionType::SNE_VX_VY: if (registers[instruction.x] != registers[instruction.y]) programcounter+=2;
        break;

    case InstructionType::LD_I_ADDR: indexRegister = instruction.nnn;
        break;

    case InstructionType::JP_V0: programcounter = instruction.nnn + registers[0];
        break;

    case InstructionType::RND_VX_BYTE: registers[instruction.x] = (rand() & instruction.kk);
        break;

    case InstructionType::DRW: {
        uint8_t x = registers[instruction.x] % 64;  
        uint8_t y = registers[instruction.y] % 32;
        uint8_t height = instruction.kk;

        registers[0xF] = 0;

        for (uint8_t row = 0; row < height; ++row) {
            if (y + row >= 32) break;

            uint8_t byte = memory[indexRegister + row];
       
            for (int bit = 0; bit < 8; ++bit) {
                if (x + bit >= 64) break;

                uint8_t color = (byte >> (7 - bit)) & 1;

                uint8_t oldPixel = FrameBuffer[y + row][x + bit];

            
                if (color == 1) {
                    FrameBuffer[y + row][x + bit] ^= 1; 
                    if (FrameBuffer[y + row][x + bit] == 0) {
                        registers[0xF] = 1;  
                    }
                }
            }
        }

        NeedDraw = true;
        break;
    }


    case InstructionType::SKP_VX: if (CheckKeyPressed(registers[instruction.x])) programcounter+=2;
        break;
    case InstructionType::SKNP_VX: if (!CheckKeyPressed(registers[instruction.x])) programcounter+=2;
        break;

    case InstructionType::LD_VX_DT: registers[instruction.x] = DelayTimer;
        break;
    case InstructionType::LD_VX_K: {
        uint8_t key = ReadKey();
        if (key != 0xFF) {
            registers[instruction.x] = key;
        } else {
            programcounter -= 2;
        }
        break;
    }
    case InstructionType::LD_DT_VX: DelayTimer = registers[instruction.x];
        break;
    case InstructionType::LD_ST_VX:  SoundTimer = registers[instruction.x];
        break;
    case InstructionType::ADD_I_VX:  indexRegister += registers[instruction.x];
        break;
    case InstructionType::LD_F_VX: indexRegister = registers[instruction.x] * 5;
        break;

    case InstructionType::LD_B_VX: {
        uint8_t val = registers.at(instruction.x);
        memory.at(indexRegister) = val / 100;
        memory.at(indexRegister + 1) = (val / 10) % 10;
        memory.at(indexRegister + 2) = val % 10;
        break;
    }

    case InstructionType::LD_I_VX: {
        for (int i = 0; i <= instruction.x; i++) {
            if (indexRegister + i < memory.size()) {
                memory[indexRegister + i] = registers[i];
            }
        }
        break;
    }
    case InstructionType::LD_VX_I: {
        for (int i = 0; i <= instruction.x; i++) {
            if (indexRegister + i < memory.size()) {
                registers[i] = memory[indexRegister + i];
            }
        }
        break;
    }

    case InstructionType::UKNOWN: break;
}
}


void ChipEmulator::Draw()
    {
            window.clear(sf::Color::Black);

            sf::Image image;
            image.resize(sf::Vector2u(64, 32));

            for (unsigned int y = 0; y < 32; ++y) {
            for (unsigned int x = 0; x < 64; ++x) {
                image.setPixel(sf::Vector2u(x, y), sf::Color::Black);
            }
        }

        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
                if (FrameBuffer[y][x] == 1) {
                    image.setPixel(sf::Vector2u(x, y), sf::Color::White);
                }
            }
        }
        if (!texture.loadFromImage(image)) {
            std::cerr << "Failed to load texture from image" << std::endl;
            return;
        }
        sprite = sf::Sprite(texture);
        sprite.setScale(sf::Vector2f(10, 10));

        window.draw(sprite);
        NeedDraw = false;
}


bool ChipEmulator::CheckKeyPressed(uint8_t keynumber)
{
if (keynumber > 0xF) return false;
switch (keynumber)
    {
    case 0x0:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X); break;
    case 0x1:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1); break;
    case 0x2:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2); break;
    case 0x3:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3); break;
    case 0x4:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q); break;
    case 0x5:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W); break;
    case 0x6:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E); break;
    case 0x7:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A); break;
    case 0x8:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S); break;
    case 0x9:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D); break;
    case 0xA:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z); break;
    case 0xB:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C); break;
    case 0xC:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4); break;
    case 0xD:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R); break;
    case 0xE:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F); break;
    case 0xF:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::V); break;
    }
    return false;
}

uint8_t ChipEmulator::ReadKey() {
    sf::Keyboard::Key chip8Keys[16] = {
        sf::Keyboard::Key::X, sf::Keyboard::Key::Num1, sf::Keyboard::Key::Num2,
        sf::Keyboard::Key::Num3, sf::Keyboard::Key::Q, sf::Keyboard::Key::W, sf::Keyboard::Key::E,
        sf::Keyboard::Key::A, sf::Keyboard::Key::S, sf::Keyboard::Key::D, sf::Keyboard::Key::Z, sf::Keyboard::Key::C,
        sf::Keyboard::Key::Num4, sf::Keyboard::Key::R,sf::Keyboard::Key::F, sf::Keyboard::Key::V
    };
    for (int i = 0; i < 16; i++)
    {
        if (sf::Keyboard::isKeyPressed(chip8Keys[i])) {
            return i;
        }
    }
    return 0xFF;
}


void ChipEmulator::GameCycle()
{
    sf::Clock timerClock;
  

    while (window.isOpen()) {
        
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        uint16_t opcode = Fetch();
        DecodedInstruction inst = Decode(opcode);
        Execute(inst);


        if (timerClock.getElapsedTime().asMilliseconds() >= 2) {
            if (DelayTimer > 0) DelayTimer--;
            if (SoundTimer > 0) {
                SoundTimer--;
            }
            timerClock.restart();
        }

        if (NeedDraw) { Draw();  }

      
        sf::sleep(sf::milliseconds(1));  
        window.display();
    }

}

ChipEmulator::~ChipEmulator() {
    file.close();
}

