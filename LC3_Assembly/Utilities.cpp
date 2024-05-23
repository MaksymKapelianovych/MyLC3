#include "Utilities.h"


std::string Utilities::ToUpperCase( const std::string &inputString )
{
	std::string upperCaseCommand = "";

	for (char currentLetter : inputString)
	{
		if ( currentLetter > 96 && currentLetter < 123 ) //lower case ascii
		{
			upperCaseCommand += currentLetter - 32;
		}
		else
		{
			upperCaseCommand += currentLetter;
		}
	}

	return upperCaseCommand;
}

uint16_t Utilities::SwitchEndianness( const uint16_t &inputValue )
{
	return inputValue << 8 | inputValue >> 8;
}

std::string Utilities::ConcatenateStrings( const std::vector<std::string> &lineToConcat, char delimitingCharacter )
{
	std::string result;

	size_t lineToConcatSize = lineToConcat.size();
	for ( size_t i = 0; i < lineToConcatSize; ++i )
	{
		if ( i != lineToConcatSize - 1 ) // Its not the last one
		{
			result += ( lineToConcat[i] + delimitingCharacter );
		}
		else
		{
			result += lineToConcat[i];
		}
	}

	return result;
}

