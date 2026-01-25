#include "Chip_emulator.h"
#include <vector>
#include <iostream>

ChipEmulator::ChipEmulator(std::string filePath)  : file(filePath)
{
if (!file.is_open()) {
    throw std::invalid_argument("File not open");
}

}

void ChipEmulator::ReadFile()
{
uint8_t byte;

for (int i = 0; i < memory.size(); i++)
{
    if (!file.get(reinterpret_cast<char&>(byte))) { break; }
    memory.at(i) = byte;
}

}


uint16_t ChipEmulator::Fetch()
{
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
    case InstructionType::CLS: break;
    case InstructionType::RET: programcounter = stack[--stackpointer];
    break;

    case InstructionType::JP: programcounter = instruction.nnn; break;
    case InstructionType::CALL: stack[stackpointer] = programcounter+2;
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
        registers[instruction.x] <<= 1;
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


    case InstructionType::SNE_VX_VY: {}
}
}


ChipEmulator::~ChipEmulator() {
    file.close();
}
