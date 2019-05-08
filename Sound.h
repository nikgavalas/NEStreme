//------------------------------------------------------------------------------
// Name: Sound.h
// Desc: Header file for Sound.cpp
//------------------------------------------------------------------------------

#ifndef __SOUND_H__
#define __SOUND_H__


//-----------------------------------------------------------------------------
// Defines.
//-----------------------------------------------------------------------------
#define OPTIONS_NUM_CHANNELS         1
#define OPTIONS_NUM_SAMPLESPERSEC    44100
#define OPTIONS_NUM_BITSPERSAMPLE    8
#define OPTIONS_NUM_BYTESTOLOCK      (((OPTIONS_NUM_BITSPERSAMPLE/8)*OPTIONS_NUM_CHANNELS*OPTIONS_NUM_SAMPLESPERSEC)/30)
#define OPTIONS_NUM_SECONDSFORBUFFER 8

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <dsound.h>
#include "Cpu.h"


//-----------------------------------------------------------------------------
// Structures for our program.
//-----------------------------------------------------------------------------

// Structure for the square wave channels of the NES's apu.
typedef struct tagCHANSQUARE
{
	BYTE byLengthCtr;       // Length counter for the channel.
} CHANNEL_SQUARE;


// Structure for the triangle channel of the NES's apu.
typedef struct tagCHANTRIANGLE
{
} CHANNEL_TRIANGLE;

// Structure for the noise channel of the NES's apu.
typedef struct tagCHANNOISE
{
} CHANNEL_NOISE;

// Structure for the dmc channel of the NES's apu.
typedef struct tagCHANDMC
{
} CHANNEL_DMC;


// Structure for the NES's apu.
typedef struct tagNESAPU
{
	// All the sound channels on the NES.
	CHANNEL_SQUARE   sndchanSquare1;
	CHANNEL_SQUARE   sndchanSquare2;
	CHANNEL_TRIANGLE sndchanTriangle;
	CHANNEL_NOISE    sndchanNoise;
	CHANNEL_DMC      sndchanDMC;

	// Which channels are enabled for sound.
	BOOL             bSquare1Enabled;
	BOOL             bSquare2Enabled;
	BOOL             bTriangleEnabled;
	BOOL             bNoiseEnabled;
	BOOL             bDMCEnabled;
} NESAPU;	


//-----------------------------------------------------------------------------
// Function Prototypes.
//-----------------------------------------------------------------------------
BOOL APU_DoFrame();
WORD APU_DoSquare1();
BYTE APU_Read(WORD wReg);
VOID APU_Write(WORD wReg, BYTE byData);

BOOL CreateSound(HWND hwnd);
BOOL CreateStreamingSoundBuffer(LPDIRECTSOUNDBUFFER& lpdsbSound, DWORD* dwMidBuffer);
VOID DestroySound();
BOOL FillBufferWithSilence(LPDIRECTSOUNDBUFFER& lpdsbSound);



//-----------------------------------------------------------------------------
// External variables needed.
//-----------------------------------------------------------------------------
extern NES6502 CPU;


#endif