//********************************************
// Code in this file taken from https://www.jmeiners.com/lc3-vm/ with their blessing
// 
//********************************************

#pragma once
#include <stdint.h>

class ExternalUtilities 
{
public:

	void Init();
	void CleanUp();
	static uint16_t check_key();
};