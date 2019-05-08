// Joystick.hpp

// If you are confused about how this works please read Yoshi's NESTECH.TXT
// It will explain this very well.  I will try to add as many comments as
// possible but they wont make sense if you don't understand how the NES
// joysticks work.

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#define JOYA			0
#define JOYB			1
#define JOYSELECT    2
#define JOYSTART		3
#define JOYUP			4
#define JOYDOWN		5
#define JOYLEFT		6
#define JOYRIGHT		7

class Joystick
{

public:

	Joystick( unsigned int );

   void write( unsigned char );
   unsigned char read();

   void Strobe();
	char GetButton( int );
   void SetButton( int );
   void ClearButton( int );

private:

	// Using an array for the bit stream takes more memory but it is faster
   // and much easier to implement.  Btw this is the actual joystick data
   char BitStream[ 24 ];

   // The current item to be read from the joystick
   int BitIndex;

   // The toggle switch that dictates whether to Srobe or not on the Next
   // write to the port
   bool StrobeOnNext;

   // The port associated with eack joystick
   unsigned int Port;
};

#endif