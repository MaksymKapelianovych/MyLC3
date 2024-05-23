#pragma once
#include <map>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "Logger.h"

class Assembler
{
public:
	static void RunAssemblyProcess( std::vector<std::string>& fileAsLines );

	static std::vector<uint16_t> AssembleIntoBinary( const std::vector<std::vector<std::string>> &inputTokens );

	static void ResolveAndReplaceLabels( std::vector<std::vector<std::string>> &inputTokens, uint16_t pcStart );

	static std::vector<std::vector<std::string>> GetTokenizedInputStrings( const std::vector<std::string> &inputString );

	static void RemoveCommentsFromLine( std::string &inputLine );

	static void HandleENDMacro( std::vector<std::string> &linifiedFile );
	static void HandleORIGMacro( std::vector<std::string> &linifiedFile );

	static void HandleFILLMacros( std::vector<std::vector<std::string>> &tokeninzedInput );
	static void HandleSTRINGZMacros( std::vector<std::vector<std::string>> &tokeninzedInput );
	static void HandleTRAPCodeMacroReplacement( std::vector<std::vector<std::string>> &tokeninzedInput );

	static void LogErrors( Logger &logger );
	static bool HasErrors();

	static bool IsANumberString( const std::string &token );
	static uint16_t ConvertStringIfNumber( const std::string &token );

private:
	static std::vector<std::string> _errors;

	static std::map<std::string, uint16_t> BuildLabelAddressMap( std::vector<std::vector<std::string>> &inputTokens, std::vector<std::string> &errors );
	static void ReplaceLabelsWithOffsets( std::vector<std::vector<std::string>> &inputTokens, const std::vector<std::string> &opCodesToCheck, const std::map<std::string, uint16_t> &labelIndexPairs, uint16_t pcStart );

	static bool IsANumberLiteral( const std::string &token );
	static bool IsADecimalNumber( const std::string &token );
	static bool IsAHexNumber( const std::string &token );

	static uint16_t Get5BitImm5( const std::string &token );
	static uint16_t Get9BitOffset( const std::string &token );
	static uint16_t Get11BitOffset( const std::string &token );
	static uint16_t Get6BitOffset( const std::string &token );

	static uint16_t ConvertToDecimal( const std::string &token );
	static uint16_t ConvertRegisterStringsTo3BitAddress( const std::string &registerName, std::vector<std::string> &errors );
	static uint16_t ConvertFromHexToDec( const std::string &token );

	static uint16_t HandleLITConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleADDConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleANDConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleNOTConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleBRConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleJMPConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleJSRConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleLDConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleLDIConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleLDRConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleLEAConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleSTConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleSTIConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleRTIConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleSTRConversion( const std::vector<std::string> &instruction );
	static uint16_t HandleTRAPConversion( const std::vector<std::string> &instruction );
};