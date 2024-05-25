#include <cstdint>
#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>
#include "CPU.h"
#include "ExternalUtilities.h"
#include "Utilities.h"
#include <stdio.h>
#include <stdint.h>


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: "<< argv[0] << " path swap_endianness\n"
		          << "	path:             relative or abolute path to assembly using forward slashes.\n" 
				  << "  swap_endianness:  whether to swap byte order for VM. Acceptable values are TRUE or FALSE. Default is TRUE."
				  << '\n';
		return 1;
	}

	bool swapEndianness = true;
	if (argc == 3)
	{
		if (Utilities::ToUpperCase(argv[2]) == "TRUE")
			swapEndianness = true;
		else if (Utilities::ToUpperCase(argv[2]) == "FALSE")
			swapEndianness = false;
		else 
		{
			std::cout << "Unrecognized argument: " << argv[2] << '\n' << '\n';

			std::cout << "Usage: "<< argv[0] << " path swap_endianness\n"
				<< "  path:             relative or abolute path to assembly using forward slashes.\n" 
				<< "  swap_endianness:  whether to swap byte order for VM. Acceptable values are TRUE or FALSE. Default is TRUE."
				<< '\n';

			return 1;
		}
	}

	uint16_t executableOrigin = Utilities::LoadFileInto(argv[1], CPU::memory, MEM_MAX, swapEndianness);

	ExternalUtilities EUtils;

	EUtils.Init();

	CPU::SetValueInRegister(CPU::R_PC, executableOrigin);

	CPU::shouldBeRunning = true;

	std::cout << "Executing Image at " << executableOrigin << "\n-----------------------------" << '\n';

	CPU::ProcessProgram();
	
	std::cout << "\n-----------------------------\n" << "Execution terminated at " 
		<< CPU::GetValueInReg(CPU::R_PC)<< '\n';
	
	EUtils.CleanUp();
	
	return 0;
}
