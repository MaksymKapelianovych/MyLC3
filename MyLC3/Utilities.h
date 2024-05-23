#include <cstdint>
#include <string>

using std::string;

class Utilities 
{
public:
	// Returns PC start
	static uint16_t LoadFileInto(string filename, uint16_t* test, int numberToRead, bool swapEndianness);

	static string ToUpperCase(const string& inputString);
};