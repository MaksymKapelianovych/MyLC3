#include <iostream>
#include <bitset>
#include <fstream>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit

enum ERegister 
{
	R_R0 = 0b000, // in/out data
	R_R1 = 0b001,
	R_R2 = 0b010,
	R_R3 = 0b011,
	R_R4 = 0b100,
	R_R5 = 0b101,
	R_R6 = 0b110, // stack pointer
	R_R7 = 0b111, // store return address 
	R_PC,         // program counter
	R_PSR,
	R_NUM
};

enum ECondition
{
	FL_P = 1 << 0,
	FL_Z = 1 << 1,
	FL_N = 1 << 2
};

enum EOpcodes
{
	OP_BR  = 0, /* branch */
	OP_ADD = 0b0001,    /* add  */
	OP_LD  = 0b0010,    /* load */
	OP_ST  = 0b0011,    /* store */
	OP_JSR = 0b0100,    /* jump register */
	OP_AND = 0b0101,    /* bitwise and */
	OP_LDR = 0b0110,    /* load register */
	OP_STR = 0b0111,    /* store register */
	OP_RTI = 0b1000,    /* unused */
	OP_NOT = 0b1001,    /* bitwise not */
	OP_LDI = 0b1010,    /* load indirect */
	OP_STI = 0b1011,    /* store indirect */
	OP_JMP = 0b1100,    /* jump */
	OP_RES = 0b1101,    /* reserved (unused) */
	OP_LEA = 0b1110,    /* load effective address */
	OP_TRAP= 0b1111     /* execute trap */
};

enum EKeyboard
{
	MR_KBSR = 0xFE00, /* keyboard status */
	MR_KBDR = 0xFE02  /* keyboard data */
};
enum ETrap
{
	TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
	TRAP_OUT = 0x21,   /* output a character */
	TRAP_PUTS = 0x22,  /* output a word string */
	TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
	TRAP_PUTSP = 0x24, /* output a byte string */
	TRAP_HALT = 0x25   /* halt the program */
};


#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];  /* 65536 locations */
uint16_t regs[ERegister::R_NUM];

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

uint16_t check_key()
{
	return WaitForSingleObject( hStdin, 1000 ) == WAIT_OBJECT_0 && _kbhit();
}

void handle_interrupt( int signal )
{
	restore_input_buffering();
	printf( "\n" );
	exit( -2 );
}

uint16_t sign_extend( uint16_t x, int bit_count )
{
	if ( ( x >> ( bit_count - 1 ) ) & 1 ) // check if desired number is negative
	{
		x |= ( 0xFFFF << bit_count );     // fill leftmost bits with 1s
	}
	return x;
}

uint16_t swap16( uint16_t x )
{
	return ( x << 8 ) | ( x >> 8 );
}

void update_flags( uint16_t r )
{
	if ( regs[r] == 0 )
	{
		regs[R_PSR] = FL_Z;
	}
	else if ( regs[r] >> 15 ) /* a 1 in the left-most bit indicates negative */
	{
		regs[R_PSR] = FL_N;
	}
	else
	{
		regs[R_PSR] = FL_P;
	}
}


constexpr std::size_t BITS_PER_BYTE = std::numeric_limits<uint16_t>::digits;

using bits_in_byte = std::bitset<BITS_PER_BYTE>;

int read_image( const char *path_to_file )
{
	std::ifstream file( path_to_file, std::ios::binary ); // open in binary mode
	if ( !file )
	{
		return 0;
	};

	/* the origin tells us where in memory to place the image */
	uint16_t origin;
	file.read( reinterpret_cast<char *>( &origin ), 2 );
	origin = swap16( origin );

	/* we know the maximum file size so we only need one fread */
	//uint16_t max_read = MEMORY_MAX - origin;
	uint16_t *word = memory + origin;


	while ( file.read( reinterpret_cast<char *>( word ), 2 ) ) // read byte by byte
	{
		*word = swap16( *word );
		++word;
	}

	return origin;
}

void mem_write( uint16_t address, uint16_t val )
{
	memory[address] = val;
}

uint16_t mem_read( uint16_t address )
{
	if ( address == MR_KBSR )
	{
		if ( check_key() )
		{
			memory[MR_KBSR] = ( 1 << 15 );
			memory[MR_KBDR] = getchar();
		}
		else
		{
			memory[MR_KBSR] = 0;
		}
	}
	return memory[address];
}

int main( int argc, const char *argv[] )
{
	if ( argc < 2 )
	{
		/* show usage string */
		printf( "lc3 [image-file1] ...\n" );
		exit( 2 );
	}

	uint16_t origin = read_image( argv[1] );

	if ( !origin )
	{
		printf( "failed to load image: %s\n", argv[1] );
		exit( 1 );
	}

	signal( SIGINT, handle_interrupt );
	disable_input_buffering();

	/* since exactly one condition flag should be set at any given time, set the Z flag */
	regs[ERegister::R_PSR] = ECondition::FL_Z;

	/* set the PC to starting position */
	/* 0x3000 is the default */
	regs[ERegister::R_PC] = origin;

	int running = 1;
	while ( running )
	{
		/* FETCH */
		uint16_t instr = mem_read( regs[ERegister::R_PC]++ );
		uint16_t op = instr >> 12;

		switch ( op )
		{
			case OP_BR:
			{
				uint16_t pc_offset = sign_extend( instr & 0x1FF, 9 );
				uint16_t cond_flag = ( instr >> 9 ) & 0x7;
				if ( cond_flag & regs[ERegister::R_PSR] || cond_flag == 0 )
				{
					regs[ERegister::R_PC] += pc_offset;
				}
			}
			break;
			case OP_ADD:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t sr1 = ( instr >> 6 ) & 0x7;
				uint16_t imm_mode = ( instr >> 5 ) & 0x1;

				if ( imm_mode )
				{
					uint16_t imm5 = sign_extend( instr & 0x1F, 5 );
					regs[dr] = regs[sr1] + imm5;
				}
				else
				{
					uint16_t sr2 = instr & 0x7;
					regs[dr] = regs[sr1] + regs[sr2];
				}

				update_flags( dr );
			}
			break;
			case OP_LD:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t pc_offset9 = sign_extend( instr & 0x1FF, 9 );

				regs[dr] = mem_read( regs[ERegister::R_PC] + pc_offset9 );

				update_flags( dr );
			}
			break;
			case OP_ST:
			{
				uint16_t sr = ( instr >> 9 ) & 0x7;
				uint16_t pc_offset9 = sign_extend( instr & 0x1FF, 9 );
				
				mem_write( regs[ERegister::R_PC] + pc_offset9, regs[sr] );
			}
			break;
			case OP_JSR:
			{
				uint16_t flag = ( instr >> 11 ) & 0x1;
				regs[ERegister::R_R7] = regs[ERegister::R_PC];
				if ( flag ) // JSR Label
				{
					uint16_t pc_offset = sign_extend( instr & 0x7FF, 11 );
					regs[ERegister::R_PC] = regs[ERegister::R_PC] + pc_offset;
				}
				else // JSRR BaseR
				{
					uint16_t r = ( instr >> 6 ) & 0x7;
					regs[ERegister::R_PC] = regs[r];
				}
			}
			break;
			case OP_AND:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t sr1 = ( instr >> 6 ) & 0x7;
				uint16_t imm_mode = ( instr >> 5 ) & 0x1;

				if ( imm_mode )
				{
					uint16_t imm5 = sign_extend( instr & 0x1F, 5 );
					regs[dr] = regs[sr1] & imm5;
				}
				else
				{
					uint16_t sr2 = instr & 0x7;
					regs[dr] = regs[sr1] & regs[sr2];
				}

				update_flags( dr );
			}
			break;
			case OP_LDR:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t base_r = ( instr >> 6 ) & 0x7;
				uint16_t offset6 = sign_extend( instr & 0x3F, 6 );

				regs[dr] = mem_read( regs[base_r] + offset6 );

				update_flags( dr );
			}
			break;
			case OP_STR:
			{
				uint16_t sr = ( instr >> 9 ) & 0x7;
				uint16_t base_r = ( instr >> 6 ) & 0x7;
				uint16_t offset6 = sign_extend( instr & 0x3F, 6 );

				mem_write( regs[base_r] + offset6, regs[sr] );
			}
			break;
			case OP_RTI:
			{
				// return from interrupt, currently unused
			}
			case OP_NOT:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t sr = ( instr >> 6 ) & 0x7;

				regs[dr] = ~regs[sr];

				update_flags( dr );
			}
			break;
			case OP_LDI:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t pc_offset9 = sign_extend( instr & 0x1FF, 9 );

				regs[dr] = mem_read( mem_read( regs[ERegister::R_PC] + pc_offset9 ) );

				update_flags( dr );
			}
			break;
			case OP_STI:
			{
				uint16_t sr = ( instr >> 9 ) & 0x7;
				uint16_t pc_offset9 = sign_extend( instr & 0x1FF, 9 );

				mem_write( mem_read( regs[ERegister::R_PC] + pc_offset9 ), regs[sr] );
			}
			break;
			case OP_JMP:
			{
				uint16_t r = ( instr >> 6 ) & 0x7;
				regs[ERegister::R_PC] = regs[r]; // for RET register is 0b111 already R7
			}
			break;
			case OP_RES:
			{
				// reserved
			}
			break;
			case OP_LEA:
			{
				uint16_t dr = ( instr >> 9 ) & 0x7;
				uint16_t pc_offset9 = sign_extend( instr & 0x1FF, 9 );

				regs[dr] = regs[ERegister::R_PC] + pc_offset9;

				update_flags( dr );
			}
			break;
			case OP_TRAP:
			{
				regs[ERegister::R_R7] = regs[ERegister::R_PC];
				uint16_t trapvect8 = instr & 0xFF;

				switch ( trapvect8 )
				{
					case TRAP_GETC:
					{
						/* read a single ASCII char */
						regs[ERegister::R_R0] = (uint16_t)getchar();
						update_flags( ERegister::R_R0 );
					}
					break;
					case TRAP_OUT:
					{
						putc( (char)regs[ERegister::R_R0], stdout );
						fflush( stdout );
					}
					break;
					case TRAP_PUTS:
					{
						/* one char per word */
						uint16_t *c = memory + regs[ERegister::R_R0];
						while ( *c )
						{
							putc( (char)*c, stdout );
							++c;
						}
						fflush( stdout );
					}
					break;
					case TRAP_IN:
					{
						printf( "Enter a character: " );
						char c = getchar();
						putc( c, stdout );
						fflush( stdout );
						regs[ERegister::R_R0] = (uint16_t)c;
						update_flags( ERegister::R_R0 );
					}
					break;
					case TRAP_PUTSP:
					{
						/* one char per byte (two bytes per word)
						   here we need to swap back to
						   big endian format */
						uint16_t *c = memory + regs[ERegister::R_R0];
						while ( *c )
						{
							char char1 = ( *c ) & 0xFF;
							putc( char1, stdout );
							char char2 = ( *c ) >> 8;
							if ( char2 ) putc( char2, stdout );
							++c;
						}
						fflush( stdout );
					}
					break;
					case TRAP_HALT:
					{
						puts( "HALT" );
						fflush( stdout );
						running = 0;
					}
					break;
					default:
						break;
				}
			}
			break;
			default:
				abort();
				break;
		}
	}

	restore_input_buffering();
	return 0;
}