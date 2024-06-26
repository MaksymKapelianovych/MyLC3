//********************************************
// Code in this file taken from https://www.jmeiners.com/lc3-vm/ with their blessing
// 
//********************************************
#include "ExternalUtilities.h"
#include <iostream>
#include <bitset>
#include <fstream>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering()
{
    hStdin = GetStdHandle( STD_INPUT_HANDLE );
    GetConsoleMode( hStdin, &fdwOldMode ); /* save old mode */
    fdwMode = fdwOldMode
        ^ ENABLE_ECHO_INPUT  /* no input echo */
        ^ ENABLE_LINE_INPUT; /* return when one or
                                more characters are available */
    SetConsoleMode( hStdin, fdwMode ); /* set new mode */
    FlushConsoleInputBuffer( hStdin ); /* clear buffer */
}

void restore_input_buffering()
{
    SetConsoleMode( hStdin, fdwOldMode );
}

void handle_interrupt( int signal )
{
    restore_input_buffering();
    printf( "\n" );
    exit( -2 );
}

void ExternalUtilities::Init()
{
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
}

void ExternalUtilities::CleanUp()
{
    restore_input_buffering();
}

uint16_t ExternalUtilities::check_key()
{
    return WaitForSingleObject( hStdin, 1000 ) == WAIT_OBJECT_0 && _kbhit();
}





