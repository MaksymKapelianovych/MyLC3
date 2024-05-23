#pragma once
#include <string>
#include <vector>

class Utilities
{
public:
	static std::string ToUpperCase( const std::string &inputString );

	static uint16_t SwitchEndianness( const uint16_t &inputValue );

	static std::string ConcatenateStrings( const std::vector<std::string> &lineToConcat, char delimitingCharacter = ' ' );
};