#include "Register.h"
#include "ExternalUtilities.h"
#include <string>
#include <iostream>
#include <exception>

Register::Register()
{
}

Register::~Register()
{
}

void Register::WriteMemoryAt(uint16_t address, uint16_t value)
{
    if (address < MEM_MAX)
        memory[address] = value;
}

uint16_t Register::reg[R_COUNT];
uint16_t Register::memory[MEM_MAX] = {0};
bool Register::shouldBeRunning = false;

void Register::UpdateFlags(REGISTER regIndex)
{
    uint16_t valueAtRegister = reg[regIndex];

    if (valueAtRegister == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if ((valueAtRegister >> 15) & 1) // negative bit is set
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

uint16_t Register::ReadMemoryAt(uint16_t address) 
{
    if (address == Register::MR_KBSR)
    {
        if (ExternalUtilities::check_key())
        {
            Register::memory[Register::MR_KBSR] = (1 << 15);
            Register::memory[Register::MR_KBDR] = getchar();
        }
        else 
        {
            Register::memory[Register::MR_KBSR] = 0;
        }
    }
    else if (address > (MEM_MAX))
    {
        std::cout << "Attempted to read data outside of MEM_MAX. Attempted Address was: " << address << std::endl;
    }
    
    return Register::memory[address];
}

void Register::ProcessProgram()
{
    reg[R_COND] = FL_ZRO;

    while (Register::shouldBeRunning)
    {
        ProcessWord();
    }
}

void Register::Add(const uint16_t& instruction)
{
    // xxxx xxx xxx x xx xxx
    // inst  dr sr1 m xx sr2
    // inst  dr sr1 m imm5
    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t firstRegister = (instruction >> 6) & 0x7;
    if ((instruction >> 5) & 1) // alt-add mode
    {
        uint16_t valueToAdd = (instruction) & 0b11111;
        valueToAdd = ExtendSign(valueToAdd, 5);
    
        reg[destinationRegister] = reg[firstRegister] + valueToAdd;
    }
    else // first add mode 
    {
        uint16_t secondRegister = (instruction) & 0x7;
      
        reg[destinationRegister] = reg[firstRegister] + reg[secondRegister];
    }
    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::And(const uint16_t& instruction)
{
    // xxxx xxx xxx x xx xxx
    // inst  dr sr1 m xx sr2
    // inst  dr sr1 m imm5
    uint16_t firstRegister = (instruction >> 6) & 0x7;
    uint16_t destinationRegister = (instruction >> 9) & 0x7;

    if ((instruction >> 5) & 1) // alt-and mode
    {
        uint16_t valueToAnd = (instruction) & 0b11111;
        valueToAnd = ExtendSign(valueToAnd, 5);

        reg[destinationRegister] = reg[firstRegister] & valueToAnd;
    }
    else // normal mode
    {
        uint16_t secondRegister = (instruction) & 0x7;
        reg[destinationRegister] = reg[firstRegister] & reg[secondRegister];
    }

    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::Not(const uint16_t& instruction) 
{
    // xxxx xxx xxx x xxxxx
    // inst DR  SR  x xxxxx
    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t firstRegister = (instruction >> 6) & 0x7;

    reg[destinationRegister] = ~reg[firstRegister];

    Register::UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::Jmp(const uint16_t& instruction) 
{
    // xxxx xxx xxx xxxxxx
    // inst xxx reg xxxxxx

    uint16_t regIndex = (instruction >> 6) & 0x7;
    reg[R_PC] = reg[regIndex];
}

void Register::Jsr(const uint16_t& instruction) 
{
    // xxxx x xxxxxxxxxxx JSR
    // inst m1 PCOffset11
    // xxxx x xx xxx xxxxxx
    // inst m0   reg xxxxxx

    bool flagSet = (instruction >> 11) & 1;
    reg[R_R7] = reg[R_PC];
    
    if (flagSet) 
    {
        reg[R_PC] += ExtendSign(instruction & 0b11111111111, 11);
    }
    else 
    {
        uint16_t registerIndex = (instruction >> 6) & 0x7;

        reg[R_PC] = reg[registerIndex];
    }
}

void Register::Ld(const uint16_t & instruction)
{
    // xxxx xxx xxxxxxxxx
    // inst DR  PCOffset9
    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111111, 9);
    
    reg[destinationRegister] = Register::ReadMemoryAt(reg[R_PC] + signExtendedOffset);

    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::Ldi(const uint16_t& instruction)
{
    // xxxx xxx xxxxxxxxx
    // inst DR  9PCOffset

    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111111, 9);

    reg[destinationRegister] = ReadMemoryAt(ReadMemoryAt(reg[R_PC] + signExtendedOffset));

    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::Ldr(const uint16_t& instruction)
{
    // xxxx xxx xxx xxxxxx
    // inst DR  Reg  Off6

    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t baseRegister = (instruction >> 6) & 0x7;

    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111, 6);

    reg[destinationRegister] = ReadMemoryAt(reg[baseRegister] + signExtendedOffset);

    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::Lea(const uint16_t& instruction) 
{
    // xxxx xxx xxxxxxxxx
    // inst DR  PCOffset9

    uint16_t destinationRegister = (instruction >> 9) & 0x7;
    uint16_t signExtendedOffset = ExtendSign(instruction & 0b111111111, 9);

    reg[destinationRegister] = reg[R_PC] + signExtendedOffset;

    UpdateFlags(static_cast<REGISTER>(destinationRegister));
}

void Register::St(const uint16_t& instruction) 
{
    // xxxx xxx xxxxxxxxx
    // inst SR  PCOffset9

    uint16_t sourceRegister = (instruction >> 9) & 0x7;
    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111111, 9);

    WriteMemoryAt(reg[R_PC] + signExtendedOffset, reg[sourceRegister]);
}

void Register::Sti(const uint16_t& instruction)
{
    // xxxx xxx xxxxxxxxx
    // inst SR  PCOffset9

    uint16_t sourceRegister = (instruction >> 9) & 0x7;
    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111111, 9);

    WriteMemoryAt(ReadMemoryAt(reg[R_PC] + signExtendedOffset), reg[sourceRegister]);
}

void Register::Str(const uint16_t& instruction)
{
    // xxxx xxx xxx xxxxxx
    // inst SR  BSR Off6

    uint16_t sourceRegister = (instruction >> 9) & 0x7;
    uint16_t baseRegister = (instruction >> 6) & 0x7;
    uint16_t signExtendedOffset = ExtendSign((instruction) & 0b111111, 6);

    WriteMemoryAt(reg[baseRegister] + signExtendedOffset, reg[sourceRegister]);
}

void Register::Trap(const uint16_t& instruction) 
{
    reg[R_R7] = reg[R_PC];

    switch (instruction & 0xFF)
    {
    case TRAP_GETC:
    {
        char letter = std::getchar();
        SetValueInRegister(R_R0, static_cast<uint16_t>(letter));
        UpdateFlags(R_R0);
        break;
    }
    case TRAP_HALT:
    {
        std::cout << "HALT" << std::endl;
        shouldBeRunning = false;
        break;
    }
    case TRAP_IN:
    {
        std::cout << "Input a character: ";
        char letter = std::getchar();
        SetValueInRegister(R_R0, static_cast<uint16_t>(letter) & 0xFF);
        UpdateFlags(R_R0);
        std::cout << std::flush;
        break;
    }
    case TRAP_OUT:
    {
        char letter = GetValueInReg(R_R0) & 0xFF;
        std::cout << letter << std::flush;
        break;
    }
    case TRAP_PUTS:
    {
        uint16_t index = 0;
        while (char letter = ReadMemoryAt(GetValueInReg(R_R0) + index) & 0xFF)
        {
            std::cout << letter;
            ++index;
        }

        std::cout << std::flush;
        break;
    }
    case TRAP_PUTSP:
    {
        uint16_t index = 0;
        
        while (uint16_t letter = ReadMemoryAt(GetValueInReg(R_R0) + index))
        {
            char c1, c2;

            c1 = letter >> 8; //High side
            c2 = letter & 0xFF; // Low side
            
            std::cout << c2;

            if (c1 == 0)
                break;

            std::cout << c1;

            ++index;
        }
        std::cout << std::flush;

        break;
    }
    default:
        std::cout << "Error in execution of trap code: " << std::to_string(instruction & 0xFF) << " at line: " << std::to_string(reg[R_PC]) << ". Full instruction: " << std::to_string(instruction) << std::endl;
        break;
    }
}

void Register::Br(const uint16_t& instruction)
{
    // xxxx x x x xxxxxxxxx
    // inst n z p PCOffset9
    bool nInstructionFlagSet = (instruction >> 11) & 0x1;
    bool zInstructionFlagSet = (instruction >> 10) & 0x1;
    bool pInstructionFlagSet = (instruction >> 9) & 0x1;

    bool nRegisterSet = reg[R_COND] & FL_NEG;
    bool zRegisterSet = reg[R_COND] & FL_ZRO;
    bool pRegisterSet = reg[R_COND] & FL_POS;

    if ((nInstructionFlagSet && nRegisterSet) || (zInstructionFlagSet && zRegisterSet) || (pInstructionFlagSet && pRegisterSet)) 
    {
        reg[R_PC] += ExtendSign(instruction & 0b111111111, 9);
    }
}

void Register::SetValueInRegister(REGISTER regIndex, uint16_t value)
{
    Register::reg[regIndex] = value;
}

void Register::HandleBadOpCode(const uint16_t& instruction) 
{
    std::cout << "Bad Op Code: " << instruction << std::endl;
    //Do something!
}

void Register::ProcessWord()
{
    uint16_t instr = ReadMemoryAt(reg[R_PC]++);
    uint16_t op = instr >> 12;

    switch (op)
    {
    case OP_ADD:
        Register::Add(instr);
        break;
    case OP_AND:
        Register::And(instr);
        break;
    case OP_NOT:
        Register::Not(instr);
        break;
    case OP_BR:
        Register::Br(instr);
        break;
    case OP_JMP:
        Register::Jmp(instr);
        break;
    case OP_JSR:
        Register::Jsr(instr);
        break;
    case OP_LD:
        Register::Ld(instr);
        break;
    case OP_LDI:
        Register::Ldi(instr);
        break;
    case OP_LDR:
        Register::Ldr(instr);
        break;
    case OP_LEA:
        Register::Lea(instr);
        break;
    case OP_ST:
        Register::St(instr);
        break;
    case OP_STI:
        Register::Sti(instr);
        break;
    case OP_STR:
        Register::Str(instr);
        break;
    case OP_TRAP:
        Register::Trap(instr);
        break;
    case OP_RES:
        Register::HandleBadOpCode(instr);
        break;
    case OP_RTI:
        Register::HandleBadOpCode(instr);
        break;
    default:
        Register::HandleBadOpCode(instr);
        break;
    }
}
