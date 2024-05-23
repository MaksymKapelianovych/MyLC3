#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include "Assembler.h"
#include "Logger.h"
#include "Utilities.h"

int main( int argc, char *argv[] )
{
	if ( argc < 3 )
	{
		std::cout << "Usage: " << argv[0] << " path swap_endianness\n"
			<< "  path:             relative or abolute path to input assembly code using forward slashes.\n"
			<< "  swap_endianness:  whether to swap byte order during assembly. Acceptable values are TRUE or FALSE. Should typically be TRUE."
			<< '\n';

		return 1;
	}

	bool switchEndianness;

	if ( Utilities::ToUpperCase( argv[2] ) == "TRUE" )
		switchEndianness = true;
	else if ( Utilities::ToUpperCase( argv[2] ) == "FALSE" )
		switchEndianness = false;
	else
	{
		std::cout << "Usage: " << argv[0] << " path swap_endianness\n"
			<< "  path:             relative or abolute path to input assembly code using forward slashes.\n"
			<< "  swap_endianness:  whether to swap byte order during assembly. Acceptable values are TRUE or FALSE. Should typically be TRUE."
			<< '\n';

		return 1;
	}

	std::string inputFilePath = argv[1];

	std::ifstream input( inputFilePath, std::ios::in );
	if ( !input.is_open() )
	{
		std::cout << "File failed to load at " + inputFilePath + ". Exiting..." << '\n';
		return -1;
	}

	std::vector<std::string> fileAsLines;
	std::string currentLine;
	std::regex nonBlankLinePattern( "(\\w+)", std::regex_constants::ECMAScript );
	while ( std::getline( input, currentLine ) )
	{
		Assembler::RemoveCommentsFromLine( currentLine );

		std::smatch sm;
		if ( std::regex_search( currentLine, sm, nonBlankLinePattern ) )
		{
			fileAsLines.push_back( currentLine );
		}
	}

	input.close();

	std::cout << "Recieved the following raw input: " << '\n';

	for ( std::string const &line : fileAsLines )
		std::cout << line << '\n';


	Assembler::HandleENDMacro( fileAsLines );
	Assembler::HandleORIGMacro( fileAsLines );
	
	std::vector<std::vector<std::string>> tokenizedInput = Assembler::GetTokenizedInputStrings( fileAsLines );

	std::cout << "\n------------------------------\nParsed the following tokenized output: \n" << '\n';

	for ( std::vector<std::string> const &lineOfTokens : tokenizedInput )
	{
		for ( std::string const &token : lineOfTokens )
			std::cout << token << " ";

		std::cout << '\n';
	}

	Assembler::HandleFILLMacros( tokenizedInput );
	Assembler::HandleTRAPCodeMacroReplacement( tokenizedInput );
	Assembler::HandleSTRINGZMacros( tokenizedInput );

	uint16_t startLocation = Assembler::ConvertStringIfNumber( tokenizedInput[0][1] );
	Assembler::ResolveAndReplaceLabels( tokenizedInput, startLocation );

	std::cout << "\n------------------------------\nPseudo op-codes filled tokenized output: \n" << '\n';

	for ( const std::vector<std::string> &lineOfTokens : tokenizedInput )
	{
		for ( const std::string &token : lineOfTokens )
		{
			std::cout << token << " ";
		}

		std::cout << '\n';
	}

	std::cout << "\n------------------------------\nInterpreted the following parsed output: \n" << '\n';


	for ( size_t lineIndex = 0; lineIndex < tokenizedInput.size(); ++lineIndex )
	{
		std::cout << "[" << lineIndex << "]" << Utilities::ConcatenateStrings( tokenizedInput[lineIndex], ' ' );
		if ( tokenizedInput[lineIndex][0] == "LIT" && Assembler::IsANumberString( tokenizedInput[lineIndex][1] ) )
			if ( uint16_t argAsInt = Assembler::ConvertStringIfNumber( tokenizedInput[lineIndex][1] ); argAsInt < 256 )
				std::cout << " ;" << static_cast<char>( argAsInt );

		std::cout << '\n';
	}


	std::cout << "Beginning conversion to binary..." << '\n';

	std::vector<uint16_t> outputOfAssembler = Assembler::AssembleIntoBinary( tokenizedInput );

	if ( Assembler::HasErrors() )
	{
		std::cout << "Errors occurred during assembly. Details below:" << '\n';
		Logger logger = Logger();

		Assembler::LogErrors( logger );

		std::cout << "\n\nAssembly aborted." << '\n';
		return 1;
	}
	else
	{
		std::cout << "No errors were encountered during assembly.";
	}

	if ( switchEndianness )
		for ( uint16_t &value : outputOfAssembler )
			value = Utilities::SwitchEndianness( value );


	std::ofstream output( "ASSEMBLY.obj", std::ios::binary | std::ios::trunc );

	if ( output.is_open() )
	{
		output.write( reinterpret_cast<char *>( outputOfAssembler.data() ), outputOfAssembler.size() * sizeof( uint16_t ) );

		output.close();

		std::cout << "Assembly complete. Output saved as ASSEMBLY.obj" << '\n';
	}

	return 0;
}