#include <cstdint>
#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>
#include "Register.h"
#include "ExternalUtilities.h"
#include "Utilities.h"
#include <stdio.h>
#include <stdint.h>


int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: "<< argv[0] << " path swap_endianness\n"
		          << "	path:             relative or abolute path to assembly using forward slashes.\n" 
				  << "  swap_endianness:  whether to swap byte order for VM. Acceptable values are TRUE or FALSE. Should typically be TRUE."
				  << std::endl;
		return 1;
	}

	bool swapEndianness;
	if (Utilities::ToUpperCase(argv[2]) == "TRUE")
		swapEndianness = true;
	else if (Utilities::ToUpperCase(argv[2]) == "FALSE")
		swapEndianness = false;
	else 
	{
		std::cout << "Unrecognized argument: " << argv[2] << '\n' << std::endl;

		std::cout << "Usage: "<< argv[0] << " path swap_endianness\n"
			<< "  path:             relative or abolute path to assembly using forward slashes.\n" 
			<< "  swap_endianness:  whether to swap byte order for VM. Acceptable values are TRUE or FALSE. Should typically be TRUE."
			<< std::endl;

		return 1;
	}

	uint16_t executableOrigin = Utilities::LoadFileInto(argv[1], Register::memory, MEM_MAX, swapEndianness);

	ExternalUtilities EUtils;

	EUtils.Init();

	Register::SetValueInRegister(Register::R_PC, executableOrigin);

	Register::shouldBeRunning = true;

	std::cout << "Executing Image at " << executableOrigin << "\n-----------------------------" << std::endl;

	Register::ProcessProgram();
	
	std::cout << "\n-----------------------------\n" << "Execution terminated at " 
		<< Register::GetValueInReg(Register::R_PC)<< std::endl;
	
	EUtils.CleanUp();
	
	return 0;
}
