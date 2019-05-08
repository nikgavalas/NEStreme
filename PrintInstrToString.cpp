//------------------------------------------------------------------------------
// Name: PrintInstrToString.cpp
// Desc: Holds the functions involved in printing an instruction
//       from the 6502 into a string.
//------------------------------------------------------------------------------


// Includes
#include <windows.h>
#include <stdio.h>

#include "PrintInstrToString.h"


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Absolute()
// Desc: Prints the the instruction with the absolute addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Absolute(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X %02X", GetMemoryByte(wPC+1), GetMemoryByte(wPC+2));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X%02X", GetMemoryByte(wPC+2), GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 2;
} // end PrintAddrMode_Absolute()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_AbsoluteX()
// Desc: Prints the the instruction with the absolute X addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_AbsoluteX(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X %02X", GetMemoryByte(wPC+1), GetMemoryByte(wPC+2));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X%02X, X", GetMemoryByte(wPC+2), GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 2;
} // end PrintAddrMode_AbsoluteX()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_AbsoluteY()
// Desc: Prints the the instruction with the absolute Y addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_AbsoluteY(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X %02X", GetMemoryByte(wPC+1), GetMemoryByte(wPC+2));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X%02X, Y", GetMemoryByte(wPC+2), GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 2;
} // end PrintAddrMode_AbsoluteY()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Accumulator()
// Desc: Prints the the instruction with the accumulator addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Accumulator(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " A");

	// Return the number of bytes for this addressing mode.
	return 0;
} // end PrintAddrMode_Accumulator()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Immediate()
// Desc: Prints the the instruction with the immediate addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Immediate(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " #$%02X", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_Immediate()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Implied()
// Desc: Prints the the instruction with the implied addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Implied(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, "   ");

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " ");

	// Return the number of bytes for this addressing mode.
	return 0;
} // end PrintAddrMode_Implied()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_PostIndexedIndirect()
// Desc: Prints the the instruction with the post indexed indirect addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_PostIndexedIndirect(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " ($%02X), Y", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_PostIndexedIndirect()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_PreIndexedIndirect()
// Desc: Prints the the instruction with the pre indexed indirect addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_PreIndexedIndirect(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " ($%02X, X)", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_PreIndexedIndirect()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Indirect()
// Desc: Prints the the instruction with the indirect addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Indirect(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X %02X", GetMemoryByte(wPC+1), GetMemoryByte(wPC+2));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " ($%02X%02X)", GetMemoryByte(wPC+2), GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 2;
} // end PrintAddrMode_Indirect()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_Relative()
// Desc: Prints the the instruction with the relative addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_Relative(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_Relative()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_ZeroPage()
// Desc: Prints the the instruction with the zero page addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_ZeroPage(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_ZeroPage()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_ZeroPageX()
// Desc: Prints the the instruction with the zero page X addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_ZeroPageX(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X, X", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_ZeroPageX()


//------------------------------------------------------------------------------
// Name: PrintAddrMode_ZeroPageY()
// Desc: Prints the the instruction with the zero page Y addressing mode
//       and returns the number of bytes this addressing mode uses.
//------------------------------------------------------------------------------
BYTE PrintAddrMode_ZeroPageY(LPSTR strDest, LPSTR strInstrName, WORD wPC)
{
	// Print the memory byte.
	sprintf(strDest, " %02X", GetMemoryByte(wPC+1));

	// Print the instruction name to the instruction string.
	memcpy(&strDest[7], strInstrName, 3);

	// Print the operand for the instruction.
	sprintf(&strDest[10], " $%02X, Y", GetMemoryByte(wPC+1));

	// Return the number of bytes for this addressing mode.
	return 1;
} // end PrintAddrMode_ZeroPageY()


//------------------------------------------------------------------------------
// Name: PrintADC()
// Desc: Prints the ADC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintADC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "ADC"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x61:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0x65:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x69:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0x6D:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x71:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0x75:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x79:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0x7D:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintADC()


//------------------------------------------------------------------------------
// Name: PrintAND()
// Desc: Prints the AND instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintAND(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "AND"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x21:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0x25:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x29:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0x2D:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x31:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0x35:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x39:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0x3D:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintAND()


//------------------------------------------------------------------------------
// Name: PrintASL()
// Desc: Prints the ASL instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintASL(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "ASL"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x06:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x0A:
			return (1 + PrintAddrMode_Accumulator(strDest, strInstrName, wPC));
		case 0x0E:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x16:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x1E:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintASL()


//------------------------------------------------------------------------------
// Name: PrintBCC()
// Desc: Prints the BCC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBCC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BCC"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBCC()
  

//------------------------------------------------------------------------------
// Name: PrintBCS()
// Desc: Prints the BCS instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBCS(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BCS"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBCS()


//------------------------------------------------------------------------------
// Name: PrintBEQ()
// Desc: Prints the BEQ instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBEQ(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BEQ"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBEQ()


//------------------------------------------------------------------------------
// Name: PrintBIT()
// Desc: Prints the BIT instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBIT(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BIT"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x24:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x2C:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintBIT()


//------------------------------------------------------------------------------
// Name: PrintBMI()
// Desc: Prints the BMI instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBMI(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BMI"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBMI()


//------------------------------------------------------------------------------
// Name: PrintBNE()
// Desc: Prints the BNE instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBNE(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BNE"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBNE()


//------------------------------------------------------------------------------
// Name: PrintBPL()
// Desc: Prints the BPL instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBPL(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BPL"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBPL()


//------------------------------------------------------------------------------
// Name: PrintBRK()
// Desc: Prints the BRK instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBRK(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BRK"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintBRK()

//------------------------------------------------------------------------------
// Name: PrintBVC()
// Desc: Prints the BVC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBVC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BVC"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBVC()


//------------------------------------------------------------------------------
// Name: PrintBVS()
// Desc: Prints the BVS instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintBVS(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "BVS"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Relative(strDest, strInstrName, wPC));
} // end PrintBVS()


//------------------------------------------------------------------------------
// Name: PrintCLC()
// Desc: Prints the CLC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCLC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CLC"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintCLC()


//------------------------------------------------------------------------------
// Name: PrintCLD()
// Desc: Prints the CLD instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCLD(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CLD"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintCLD()


//------------------------------------------------------------------------------
// Name: PrintCLI()
// Desc: Prints the CLI instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCLI(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CLI"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintCLI()


//------------------------------------------------------------------------------
// Name: PrintCLV()
// Desc: Prints the CLV instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCLV(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CLV"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintCLV()


//------------------------------------------------------------------------------
// Name: PrintCMP()
// Desc: Prints the CMP instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCMP(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CMP"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xC1:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0xC5:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xC9:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xCD:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xD1:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0xD5:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xD9:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0xDD:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintCMP()


//------------------------------------------------------------------------------
// Name: PrintCPX()
// Desc: Prints the CPX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCPX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CPX"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xE0:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xE4:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xEC:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintCPX()


//------------------------------------------------------------------------------
// Name: PrintCPY()
// Desc: Prints the CPY instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintCPY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "CPY"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xC0:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xC4:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xCC:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintCPY()


//------------------------------------------------------------------------------
// Name: PrintDEC()
// Desc: Prints the DEC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintDEC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "DEC"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xC6:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xCE:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xD6:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xDE:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintDEC()


//------------------------------------------------------------------------------
// Name: PrintDEX()
// Desc: Prints the DEX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintDEX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "DEX"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintDEX()


//------------------------------------------------------------------------------
// Name: PrintDEY()
// Desc: Prints the DEY instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintDEY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "DEY"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintDEY()


//------------------------------------------------------------------------------
// Name: PrintEOR()
// Desc: Prints the EOR instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintEOR(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "EOR"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x41:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0x45:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x49:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0x4D:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x51:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0x55:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x59:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0x5D:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintEOR()


//------------------------------------------------------------------------------
// Name: PrintINC()
// Desc: Prints the INC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintINC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "INC"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xE6:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xEE:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xF6:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xFE:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintINC()


//------------------------------------------------------------------------------
// Name: PrintINX()
// Desc: Prints the INX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintINX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "INX"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintINX()


//------------------------------------------------------------------------------
// Name: PrintINY()
// Desc: Prints the INY instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintINY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "INY"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintINY()


//------------------------------------------------------------------------------
// Name: PrintJMP()
// Desc: Prints the JMP instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintJMP(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "JMP"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x4C:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x6C:
			return (1 + PrintAddrMode_Indirect(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintJMP()


//------------------------------------------------------------------------------
// Name: PrintJSR()
// Desc: Prints the JSR instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintJSR(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "JSR"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
} // end PrintJSR()


//------------------------------------------------------------------------------
// Name: PrintLDA()
// Desc: Prints the LDA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintLDA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "LDA"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xA1:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0xA5:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xA9:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xAD:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xB1:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0xB5:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xB9:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0xBD:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintLDA()


//------------------------------------------------------------------------------
// Name: PrintLDX()
// Desc: Prints the LDX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintLDX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "LDX"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xA6:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xA2:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xAE:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xB6:
			return (1 + PrintAddrMode_ZeroPageY(strDest, strInstrName, wPC));
		case 0xBE:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintLDX()


//------------------------------------------------------------------------------
// Name: PrintLDY()
// Desc: Prints the LDY instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintLDY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "LDY"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xA4:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xA0:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xAC:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xB4:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xBC:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintLDY()


//------------------------------------------------------------------------------
// Name: PrintLSR()
// Desc: Prints the LSR instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintLSR(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "LSR"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x46:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x4A:
			return (1 + PrintAddrMode_Accumulator(strDest, strInstrName, wPC));
		case 0x4E:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x56:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x5E:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintLSR()


//------------------------------------------------------------------------------
// Name: PrintNOP()
// Desc: Prints the NOP instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintNOP(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "NOP"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintNOP()


//------------------------------------------------------------------------------
// Name: PrintORA()
// Desc: Prints the ORA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintORA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "ORA"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x01:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0x05:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x09:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0x0D:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x11:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0x15:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x19:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0x1D:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintORA()
	

//------------------------------------------------------------------------------
// Name: PrintPHA()
// Desc: Prints the PHA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintPHA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "PHA"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintPHA()


//------------------------------------------------------------------------------
// Name: PrintPHP()
// Desc: Prints the PHP instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintPHP(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "PHP"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintPHP()


//------------------------------------------------------------------------------
// Name: PrintPLA()
// Desc: Prints the PLA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintPLA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "PLA"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintPLA()


//------------------------------------------------------------------------------
// Name: PrintPHP()
// Desc: Prints the PHP instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintPLP(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "PHP"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintPHP()


//------------------------------------------------------------------------------
// Name: PrintROL()
// Desc: Prints the ROL instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintROL(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "ROL"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x26:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x2A:
			return (1 + PrintAddrMode_Accumulator(strDest, strInstrName, wPC));
		case 0x2E:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x36:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x3E:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintROL()


//------------------------------------------------------------------------------
// Name: PrintROR()
// Desc: Prints the ROR instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintROR(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "ROR"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x66:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x6A:
			return (1 + PrintAddrMode_Accumulator(strDest, strInstrName, wPC));
		case 0x6E:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x76:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x7E:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintROR()


//------------------------------------------------------------------------------
// Name: PrintRTI()
// Desc: Prints the RTI instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintRTI(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "RTI"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintRTI()


//------------------------------------------------------------------------------
// Name: PrintRTS()
// Desc: Prints the RTS instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintRTS(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "RTS"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintRTS()


//------------------------------------------------------------------------------
// Name: PrintSBC()
// Desc: Prints the SBC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSBC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "SBC"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0xE1:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0xE5:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0xE9:
			return (1 + PrintAddrMode_Immediate(strDest, strInstrName, wPC));
		case 0xED:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0xF1:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0xF5:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0xF9:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0xFD:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintSBC()


//------------------------------------------------------------------------------
// Name: PrintSEC()
// Desc: Prints the SEC instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSEC(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "SEC"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintSEC()


//------------------------------------------------------------------------------
// Name: PrintSED()
// Desc: Prints the SED instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSED(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "SED"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintSED()


//------------------------------------------------------------------------------
// Name: PrintSEI()
// Desc: Prints the SEI instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSEI(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "SEI"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintSEI()


//------------------------------------------------------------------------------
// Name: PrintSTA()
// Desc: Prints the STA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSTA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "STA"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x81:
			return (1 + PrintAddrMode_PreIndexedIndirect(strDest, strInstrName, wPC));
		case 0x85:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x8D:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x91:
			return (1 + PrintAddrMode_PostIndexedIndirect(strDest, strInstrName, wPC));
		case 0x95:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		case 0x99:
			return (1 + PrintAddrMode_AbsoluteY(strDest, strInstrName, wPC));
		case 0x9D:
			return (1 + PrintAddrMode_AbsoluteX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintSTA()


//------------------------------------------------------------------------------
// Name: PrintSTX()
// Desc: Prints the STX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSTX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "STX"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x86:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x8E:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x96:
			return (1 + PrintAddrMode_ZeroPageY(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintSTX()


//------------------------------------------------------------------------------
// Name: PrintSTY()
// Desc: Prints the STY instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintSTY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "STY"; // Name of the instruction being printed.

	// Depending on the opcode of the instruction, different addressing
	// modes apply. This means that different strings must be printed
	// depending on the addressing mode.
	switch (byOpcode)
	{
		case 0x84:
			return (1 + PrintAddrMode_ZeroPage(strDest, strInstrName, wPC));
		case 0x8C:
			return (1 + PrintAddrMode_Absolute(strDest, strInstrName, wPC));
		case 0x94:
			return (1 + PrintAddrMode_ZeroPageX(strDest, strInstrName, wPC));
		default:
			return 0;
	}
} // end PrintSTY()


//------------------------------------------------------------------------------
// Name: PrintTAX()
// Desc: Prints the TAX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTAX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TAX"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTAX()


//------------------------------------------------------------------------------
// Name: PrintTAY()
// Desc: Prints the SEI instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTAY(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TAY"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTAY()


//------------------------------------------------------------------------------
// Name: PrintTSX()
// Desc: Prints the TSX instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTSX(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TSX"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTSX()


//------------------------------------------------------------------------------
// Name: PrintTXA()
// Desc: Prints the TXA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTXA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TXA"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTXA()


//------------------------------------------------------------------------------
// Name: PrintTXS()
// Desc: Prints the TXS instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTXS(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TXS"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTXS()


//------------------------------------------------------------------------------
// Name: PrintTYA()
// Desc: Prints the TYA instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintTYA(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "TYA"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintTYA()


//------------------------------------------------------------------------------
// Name: PrintUNDEF()
// Desc: Prints the UNDEF instruction to a string and returns the number
//       of bytes for the entire instruction.
//------------------------------------------------------------------------------
BYTE PrintUNDEF(LPSTR strDest, WORD wPC, BYTE byOpcode)
{
	LPSTR strInstrName = "???"; // Name of the instruction being printed.

	return (1 + PrintAddrMode_Implied(strDest, strInstrName, wPC));
} // end PrintUNDEF()

//------------------------------------------------------------------------------
// Name: PrintInstrToString()
// Desc: Gets the opcode of the instruction from memory and prints the
//       appropirate instruction to the string passed to the function.
//       This function returns the number of bytes the entire instruction
//       occupies. The string format is:
//       "PC\0MachineCode\0Instruction\0MemoryContents\0"
//
//       Each individual part can be accessed by their associated constants
//       in PrintInstreToString.h
//
//       This way the strings can be easily printed separately 
//       becuase of the null terminating characters.
//------------------------------------------------------------------------------
BYTE PrintInstrToString(LPSTR strInstr, WORD wPC)
{
	BYTE byOpcode = GetMemoryByte(wPC); // The opcode of our instruction.

	// Print the beginning of the string.
	// This is the same for every instruction.
	sprintf(strInstr, "%04X: %02X", wPC, byOpcode);
	strInstr[5] = '\0';

	// Whatever the instruction is, print it to the string.
	switch (byOpcode)
	{
		case 0x69: 
		case 0x65: 
		case 0x75: 
		case 0x6D: 
		case 0x7D: 
		case 0x79:
		case 0x61: 
		case 0x71:
			return PrintADC(&strInstr[8], wPC, byOpcode);

		case 0x21:
		case 0x25:
		case 0x29:
		case 0x2D:
		case 0x31:
		case 0x35:
		case 0x39:
		case 0x3D:
			return PrintAND(&strInstr[8], wPC, byOpcode);

		case 0x06:
		case 0x0A:
		case 0x0E:
		case 0x16:
		case 0x1E:
			return PrintASL(&strInstr[8], wPC, byOpcode);

		case 0x90:
			return PrintBCC(&strInstr[8], wPC, byOpcode);

		case 0xB0:
			return PrintBCS(&strInstr[8], wPC, byOpcode);

		case 0xF0:
			return PrintBEQ(&strInstr[8], wPC, byOpcode);

		case 0x24:
		case 0x2C:
			return PrintBIT(&strInstr[8], wPC, byOpcode);

		case 0x30:
			return PrintBMI(&strInstr[8], wPC, byOpcode);

		case 0xD0:
			return PrintBNE(&strInstr[8], wPC, byOpcode);

		case 0x10:
			return PrintBPL(&strInstr[8], wPC, byOpcode);

		case 0x00:
			return PrintBRK(&strInstr[8], wPC, byOpcode);

		case 0x50:
			return PrintBVC(&strInstr[8], wPC, byOpcode);

		case 0x70:
			return PrintBVS(&strInstr[8], wPC, byOpcode);

		case 0x18:
			return PrintCLC(&strInstr[8], wPC, byOpcode);

		case 0xD8:
			return PrintCLD(&strInstr[8], wPC, byOpcode);

		case 0x58:
			return PrintCLI(&strInstr[8], wPC, byOpcode);

		case 0xB8:
			return PrintCLV(&strInstr[8], wPC, byOpcode);

		case 0xC9:
		case 0xC5:
		case 0xD5:
		case 0xCD:
		case 0xDD:
		case 0xD9:
		case 0xC1:
		case 0xD1:
			return PrintCMP(&strInstr[8], wPC, byOpcode);

		case 0xE0:
		case 0xE4:
		case 0xEC:
			return PrintCPX(&strInstr[8], wPC, byOpcode);

		case 0xC0:
		case 0xC4:
		case 0xCC:
			return PrintCPY(&strInstr[8], wPC, byOpcode);

		case 0xC6:
		case 0xD6:
		case 0xCE:
		case 0xDE:
			return PrintDEC(&strInstr[8], wPC, byOpcode);

		case 0xCA:
			return PrintDEX(&strInstr[8], wPC, byOpcode);

		case 0x88:
			return PrintDEY(&strInstr[8], wPC, byOpcode);

		case 0x49:
		case 0x45:
		case 0x55:
		case 0x4D:
		case 0x5D:
		case 0x59:
		case 0x41:
		case 0x51:
			return PrintEOR(&strInstr[8], wPC, byOpcode);

		case 0xE6:
		case 0xF6:
		case 0xEE:
		case 0xFE:
			return PrintINC(&strInstr[8], wPC, byOpcode);

		case 0xE8:
			return PrintINX(&strInstr[8], wPC, byOpcode);

		case 0xC8:
			return PrintINY(&strInstr[8], wPC, byOpcode);

		case 0x4C:
		case 0x6C:
			return PrintJMP(&strInstr[8], wPC, byOpcode);

		case 0x20:
			return PrintJSR(&strInstr[8], wPC, byOpcode);

		case 0xA9:
		case 0xA5:
		case 0xB5:
		case 0xAD:
		case 0xBD:
		case 0xB9:
		case 0xA1:
		case 0xB1:
			return PrintLDA(&strInstr[8], wPC, byOpcode);

		case 0xA2:
		case 0xA6:
		case 0xB6:
		case 0xAE:
		case 0xBE:
			return PrintLDX(&strInstr[8], wPC, byOpcode);

		case 0xA0:
		case 0xA4:
		case 0xB4:
		case 0xAC:
		case 0xBC:
			return PrintLDY(&strInstr[8], wPC, byOpcode);

		case 0x4A:
		case 0x46:
		case 0x56:
		case 0x4E:
		case 0x5E:
			return PrintLSR(&strInstr[8], wPC, byOpcode);

		case 0xEA:
			return PrintNOP(&strInstr[8], wPC, byOpcode);

		case 0x09:
		case 0x05:
		case 0x15:
		case 0x0D:
		case 0x1D:
		case 0x19:
		case 0x01:
		case 0x11:
			return PrintORA(&strInstr[8], wPC, byOpcode);

		case 0x48:
			return PrintPHA(&strInstr[8], wPC, byOpcode);

		case 0x08:
			return PrintPHP(&strInstr[8], wPC, byOpcode);

		case 0x68:
			return PrintPLA(&strInstr[8], wPC, byOpcode);

		case 0x28:
			return PrintPLP(&strInstr[8], wPC, byOpcode);

		case 0x2A:
		case 0x26:
		case 0x36:
		case 0x2E:
		case 0x3E:
			return PrintROL(&strInstr[8], wPC, byOpcode);

		case 0x6A:
		case 0x66:
		case 0x76:
		case 0x6E:
		case 0x7E:
			return PrintROR(&strInstr[8], wPC, byOpcode);

		case 0x40:
			return PrintRTI(&strInstr[8], wPC, byOpcode);

		case 0x60:
			return PrintRTS(&strInstr[8], wPC, byOpcode);

		case 0xE9:
		case 0xE5:
		case 0xF5:
		case 0xED:
		case 0xFD:
		case 0xF9:
		case 0xE1:
		case 0xF1:
			return PrintSBC(&strInstr[8], wPC, byOpcode);

		case 0x38:
			return PrintSEC(&strInstr[8], wPC, byOpcode);

		case 0xF8:
			return PrintSED(&strInstr[8], wPC, byOpcode);

		case 0x78:
			return PrintSEI(&strInstr[8], wPC, byOpcode);

		case 0x85:
		case 0x95:
		case 0x8D:
		case 0x9D:
		case 0x99:
		case 0x81:
		case 0x91:
			return PrintSTA(&strInstr[8], wPC, byOpcode);

		case 0x86:
		case 0x96:
		case 0x8E:
			return PrintSTX(&strInstr[8], wPC, byOpcode);

		case 0x84:
		case 0x94:
		case 0x8C:
			return PrintSTY(&strInstr[8], wPC, byOpcode);

		case 0xAA:
			return PrintTAX(&strInstr[8], wPC, byOpcode);

		case 0xA8:
			return PrintTAY(&strInstr[8], wPC, byOpcode);

		case 0xBA:
			return PrintTSX(&strInstr[8], wPC, byOpcode);

		case 0x8A:
			return PrintTXA(&strInstr[8], wPC, byOpcode);

		case 0x9A:
			return PrintTXS(&strInstr[8], wPC, byOpcode);

		case 0x98:
			return PrintTYA(&strInstr[8], wPC, byOpcode);

		default:
			return PrintUNDEF(&strInstr[8], wPC, byOpcode);
	}
} // end PrintInstrToString()
