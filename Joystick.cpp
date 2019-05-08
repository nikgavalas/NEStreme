// Joystick.cpp

#include "Joystick.h"

Joystick::Joystick(unsigned int PortValue)
{
	if ((PortValue != 0x4016) || (PortValue != 0x4017))
		Port = 0x4016;
	else
		Port = PortValue;

	Strobe();
}


void Joystick::Strobe()
{
	BitIndex = 0;
}

void Joystick::write(unsigned char value)
{
	if (((value & 1) == 1) && (StrobeOnNext == false))
		StrobeOnNext = true;
	else if (((value & 1) == 0) && (StrobeOnNext == true))
	{
		Strobe();
		StrobeOnNext = false;
	}
}

unsigned char Joystick::read()
{
	char BitReturn = BitStream[BitIndex];
	BitIndex++;

	// Check to see if read past the joysticks data.  Not likely that this
	// will happen but its possible.
	if (BitIndex >= 24)
		Strobe();

	return BitReturn;
}

char Joystick::GetButton(int button)
{
	return BitStream[button];
}

void Joystick::SetButton(int button)
{
	BitStream[button] = 1;
}

void Joystick::ClearButton(int button)
{
	BitStream[button] = 0;
}