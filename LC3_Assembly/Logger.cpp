#include "Logger.h"

void Logger::Log( const std::vector<std::string> &logs )
{
	for ( const std::string& line : logs )
	{
		std::cout << line << '\n';
	}
}