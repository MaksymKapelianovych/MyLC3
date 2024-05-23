// LC3_Assembly.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

uint16_t current_address;

std::map<std::string, uint16_t> symbol_table;

std::map<std::string, uint16_t> regs_table{
	{
		{"R0", 0b000},
		{"R1", 0b001},
		{"R2", 0b010},
		{"R3", 0b011},
		{"R4", 0b100},
		{"R5", 0b101},
		{"R6", 0b110},
		{"R7", 0b111}
	}
};

std::map<std::string, uint16_t> op_codes_table{
	{
		{"ADD",   0b0001},
		{"AND",   0b0101},
		{"NOT",   0b1001},

		{"BR",    0b0000},
		{"BRN",   0b0000},
		{"BRZ",   0b0000},
		{"BRP",   0b0000},
		{"BRZP",  0b0000},
		{"BRNP",  0b0000},
		{"BRNZ",  0b0000},
		{"BRNZP", 0b0000},
		
		{"JMP",   0b1100},
		{"RET",   0b1100},
		
		{"JSR",   0b0100},
		{"JSRR",  0b0100},
		
		{"LD",    0b0010},
		{"LDI",   0b1010},
		{"LDR",   0b0110},
		{"LEA",   0b1110},

		{"ST",    0b0011},
		{"STI",   0b1011},
		{"STR",   0b0111},
		
		{"RTI",   0b1000},

		{"TRAP",  0b1111}
	}
};

std::map<std::string, uint16_t> trap_table{
	{
		{"GETC",  0x20},
		{"OUT",   0x21},
		{"PUTS",  0x22},
		{"IN",    0x23},
		{"PUTSP", 0x24},
		{"HALT",  0x25}
	}
};

/*
	Pseudo-ops:
		.ORIG
		.FILL
		.BLKW
		.STRINGZ
		.END
*/

uint16_t read_register( std::string_view word )
{
	if ( auto iter = regs_table.find( word.data() ); iter != regs_table.end() )
	{
		return ( *iter ).second;
	}

	return -1;
}

uint16_t read_opcode( std::string_view word )
{
	if ( auto iter = op_codes_table.find( word.data() ); iter != op_codes_table.end() )
	{
		return ( *iter ).second;
	}

	return -1;
}

uint16_t read_trap( std::string_view word )
{
	if ( auto iter = trap_table.find( word.data() ); iter != trap_table.end() )
	{
		return ( *iter ).second;
	}

	return -1;
}

uint16_t read_imm_value( std::string_view word )
{
	return 0x3000; // just for testing
}


void fill_symbol_table( std::ifstream &file )
{
	std::string word{};

	while ( file >> word )
	{
		std::transform( word.begin(), word.end(), word.begin(), ::toupper );
		if ( read_opcode( word ) != -1 || read_trap( word ) != -1 )
		{
			if ( word == ".ORIG" )
			{
				std::string value{};
				file >> value;
				current_address = read_imm_value( value );
			}
			symbol_table[word] = current_address;
		}
	}
}

void print_symbol_table()
{
	for ( auto& elem : symbol_table )
	{
		std::cout << std::left << std::setw( 10 ) << std::setfill( ' ' ) << elem.first << elem.second;
	}
}

void fill_image_file( std::ifstream &file )
{

}

void write_symbol_table( std::string_view path_to_file )
{

}

void write_image( std::string_view path_to_file )
{
	std::ofstream file( path_to_file.data(), std::ios::binary );
}

int main_old( int argc, const char *argv[] )
{
	if ( argc < 2 )
	{
		/* show usage string */
		std::cout << ( "LC3_Assembly [image-file1] ...\n" );
		exit( 2 );
	}

	std::ifstream file( argv[1] );
	if ( !file )
	{
		std::cout << "failed to load image: " << argv[1] << "\n";
		exit( 1 );
	};

	fill_symbol_table( file );
	fill_image_file( file );

	return 0;
}
