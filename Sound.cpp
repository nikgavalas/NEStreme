//------------------------------------------------------------------------------
// Name: Sound.cpp
// Desc: Holds all the functions that have to do with sound.
//------------------------------------------------------------------------------


#include "Sound.h"


// Length table lookup table...sounds wierd.
static const BYTE abyLengthTable[32] =
{
	0x05, 0x7F, 0x0A, 0x01, 0x13, 0x02, 0x28, 0x03,
	0x50, 0x04, 0x1E, 0x05, 0x07, 0x06, 0x0D, 0x07,
	0x06, 0x08, 0x0C, 0x09, 0x18, 0x0A, 0x30, 0x0B,
	0x60, 0x0C, 0x24, 0x0D, 0x08, 0x0E, 0x10, 0x0F
};

static const BYTE abyDutyCycleTablePos[4] = { 2,  4,  8, 12};
static const BYTE abyDutyCycleTableNeg[4] = {14, 12,  8,  4};


//------------------------------------------------------------------------------
// Global variables for this file.
//------------------------------------------------------------------------------
//CSound*        pcsndSquare1 = NULL; // Buffer for square channel 1.
//CSoundManager* pcsndmanMain = NULL; // Main sound manager for our program.
//LPSTR          strSquareWaveFile = "square1000hz44100.wav";


NESAPU         APU;                 // Apu strucure.


BOOL bCalculateSound = FALSE; // Tells the sound functions to use new or old data.

DWORD dwNumCyclesPerSample = CPU_FREQUENCY / OPTIONS_NUM_SAMPLESPERSEC;

// DirectSound stuff.
LPDIRECTSOUND       lpDS           = NULL;
LPDIRECTSOUNDBUFFER lpdsbSoundBuf = NULL;
DWORD               dwMidSoundBuf = 0;


//------------------------------------------------------------------------------
// Name: CreateSound()
// Desc: Creates DirectSound and sets the format of the primary buffer. Next
//       it creates all the secondary buffers we need for streaming.
//------------------------------------------------------------------------------
BOOL CreateSound(HWND hwnd)
{
	HRESULT hr;
	LPDIRECTSOUNDBUFFER lpdsbPrimary;
	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfm;

	// Create DirectSound.
	if (FAILED(DirectSoundCreate(NULL, &lpDS, NULL)))
		return FALSE;

	// Set the cooperative level.
	if (FAILED(lpDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
		return FALSE;

	// Set up the DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL;

	// Set up strucure for the desired sound format.
	memset(&wfm, 0, sizeof(WAVEFORMATEX));
	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = OPTIONS_NUM_CHANNELS;
	wfm.nSamplesPerSec = OPTIONS_NUM_SAMPLESPERSEC;
	wfm.wBitsPerSample = OPTIONS_NUM_BITSPERSAMPLE;
	wfm.nBlockAlign = (wfm.wBitsPerSample / 8) * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	// Gain access to the primary buffer.
	hr = lpDS->CreateSoundBuffer(&dsbdesc, &lpdsbPrimary, NULL);

	// Set the primary buffer to the desired format. If this fails,
	// we will just ignore it and go with the default format.
	if (SUCCEEDED(hr))
		hr = lpdsbPrimary->SetFormat(&wfm);

	// Create the secondary streaming buffers.
	CreateStreamingSoundBuffer(lpdsbSoundBuf, &dwMidSoundBuf);

	return TRUE;
} // end CreateDirectSound()


//------------------------------------------------------------------------------
// Name: DestroySound()
// Desc: Shuts down DirectSound, this releases the secondary buffers as well.
//------------------------------------------------------------------------------
VOID DestroySound()
{
	// Stop playing the buffer first.
	lpdsbSoundBuf->Stop();

	// Now release DirectSound.
	if (lpDS != NULL)
	{
		lpDS->Release();
		lpDS = NULL;
	}
} // end DestroyDirectSound();


//------------------------------------------------------------------------------
// Name: FillBufferWithSilence()
// Desc: Does exactly what is says. For 8-bit waves 0x80 is silent, for 
//       16-bit wave files 0 is silence.
//------------------------------------------------------------------------------
BOOL FillBufferWithSilence(LPDIRECTSOUNDBUFFER& lpdsbSound)
{
	WAVEFORMATEX    wfx;
	DWORD           dwSizeWritten;

	PBYTE   pb1;
	DWORD   cb1;

	if (FAILED(lpdsbSound->GetFormat(&wfx, sizeof(WAVEFORMATEX), &dwSizeWritten)))
		return FALSE;

	if (SUCCEEDED(lpdsbSound->Lock(0, 0, (LPVOID*)&pb1, &cb1, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
	{
		FillMemory(pb1, cb1, (wfx.wBitsPerSample == 8) ? 128 : 0);

		lpdsbSound->Unlock(pb1, cb1, NULL, 0);
		return TRUE;
	}

	return FALSE;
} // end FillBufferWithSilence()


//------------------------------------------------------------------------------
// Name: CreateStreamingSoundBuffer()
// Desc: Creates a secondary buffer for streaming and fills it with silence. 
//       Also returns the halfway point for later use.
//------------------------------------------------------------------------------
BOOL CreateStreamingSoundBuffer(LPDIRECTSOUNDBUFFER& lpdsbSound, DWORD* dwMidBuffer)
{
	HRESULT hr;
	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfm;

	// Set up strucure for the desired sound format.
	memset(&wfm, 0, sizeof(WAVEFORMATEX));
	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = OPTIONS_NUM_CHANNELS;
	wfm.nSamplesPerSec = OPTIONS_NUM_SAMPLESPERSEC;
	wfm.wBitsPerSample = OPTIONS_NUM_BITSPERSAMPLE;
	wfm.nBlockAlign = (wfm.wBitsPerSample / 8) * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	// Set up the DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	dsbdesc.dwBufferBytes = wfm.nAvgBytesPerSec * OPTIONS_NUM_SECONDSFORBUFFER;
	dsbdesc.lpwfxFormat = &wfm;

	// Create the secondary buffer.
	hr = lpDS->CreateSoundBuffer(&dsbdesc, &lpdsbSound, NULL);
	if (FAILED(hr))
		return FALSE;

	FillBufferWithSilence(lpdsbSound);

	// Start playing the buffer.
	lpdsbSound->Play(0, 0, DSBPLAY_LOOPING);

	// Save the middle of the buffer so we can write to either
	// half depending on which half of the buffer is playing.
	*dwMidBuffer = dsbdesc.dwBufferBytes / 2;

	// Success!!!!!
	return TRUE;
} // end CreateStreamingSoundBuffer()


//------------------------------------------------------------------------------
// Name: APU_DoFrame()
// Desc: Takes care of the sound for a frame.
//------------------------------------------------------------------------------
BOOL APU_DoFrame()
{
	// Stuff necessary for working with streaming buffers.
	HRESULT hr;
	DWORD   dwWritePos;
	DWORD   dwBytesLocked1;
	DWORD   dwBytesLocked2;
	DWORD   dwByteNum;
	VOID*   pvData1;
	VOID*   pvData2;
	VOID*   pvDataSave;

	static DWORD dwLastEndWritePos = 0;

	// Stuff for nintendo sound files.
	WORD wTotalOutputVol; // Sum of all the sound channel's volumes.


	// If the buffer is invalid then return.
	if (lpdsbSoundBuf == NULL)
		return FALSE;

	// Get the current write position within the buffer.
	if (FAILED(lpdsbSoundBuf->GetCurrentPosition(NULL, &dwWritePos)))
		return FALSE;

	// If the play cursor has just reached the first or second half
	// of the buffer, it's time to stream data to the other half.
	LONG lTemp = dwLastEndWritePos - 1000;
	if (lTemp < 0)
		lTemp = 0;

	//if (dwWritePos >= (dwLastEndWritePos))// || ())
	if (dwWritePos >= (DWORD)lTemp)// || ())
	{
		// Lock the buffer so we can write to it.
		hr = lpdsbSoundBuf->Lock(dwLastEndWritePos, OPTIONS_NUM_BYTESTOLOCK, 
			&pvData1, &dwBytesLocked1, &pvData2, &dwBytesLocked2, 0);

		if (SUCCEEDED(hr))
		{
			// For the first part of the loop calculate the sound values.
			bCalculateSound = TRUE;

			// Fill the whole locked portion of the sound buffer with
			// out sound data from each of the Nintendo's sound channels.
			// This involves two for loops since the buffer may wrap around.
			
			// Save a temp pointer to the first portions of the sound buffer.
			pvDataSave = pvData1;

			// First portion of the buffer.
			for (dwByteNum = 0; dwByteNum < dwBytesLocked1; 
				dwByteNum += OPTIONS_NUM_CHANNELS * (OPTIONS_NUM_BITSPERSAMPLE / 8))
			{
				// Clear the last volume outta there.
				wTotalOutputVol = 0;

				// Process the square channel 1.
				wTotalOutputVol += (WORD)APU_DoSquare1();

				// Write the data to the sound buffer and move the pointer 
				// to the buffer to the next data position.
				if (OPTIONS_NUM_BITSPERSAMPLE == 8)
				{
					*((BYTE*)pvDataSave) = (BYTE)wTotalOutputVol;
					pvDataSave = (BYTE*)pvDataSave + (OPTIONS_NUM_CHANNELS * 1);
				}
				else
				{
					*((WORD*)pvDataSave) = wTotalOutputVol;
					pvDataSave = (BYTE*)pvDataSave + (OPTIONS_NUM_CHANNELS * 2);
				}

				// Stop calculating the sound.
				if (bCalculateSound == TRUE)
					bCalculateSound = FALSE;
			}

			// If the locked portion of the buffer wrapped around to the 
			// beginning of the buffer then we need to write to it.
			if (dwBytesLocked2 > 0)
			{
				// Save a temp pointer to the second portions of the sound buffer.
				pvDataSave = pvData2;

				// Second portion of the buffer.
				for (dwByteNum = 0; dwByteNum < dwBytesLocked2; 
					dwByteNum += OPTIONS_NUM_CHANNELS * (OPTIONS_NUM_BITSPERSAMPLE / 8))
				{
					// Clear the last volume outta there.
					wTotalOutputVol = 0;

					// Process the square channel 1.
					wTotalOutputVol += (WORD)APU_DoSquare1();

					// Write the data to the sound buffer and move the pointer 
					// to the buffer to the next data position.
					if (OPTIONS_NUM_BITSPERSAMPLE == 8)
					{
						*((BYTE*)pvDataSave) = (BYTE)wTotalOutputVol;
						pvDataSave = (BYTE*)pvDataSave + (OPTIONS_NUM_CHANNELS * 1);
					}
					else
					{
						*((WORD*)pvDataSave) = wTotalOutputVol;
						pvDataSave = (BYTE*)pvDataSave + (OPTIONS_NUM_CHANNELS * 2);
					}
				}
			}

			// Unlock the buffer now that were done with it.
			lpdsbSoundBuf->Unlock(pvData1, dwBytesLocked1, pvData2, dwBytesLocked2);
		}

		// Save the position of the last place we wrote to so 
		// we can continue the next time this function is called.
		dwLastEndWritePos = (DWORD)(dwLastEndWritePos+dwBytesLocked1+dwBytesLocked2);

		if (dwLastEndWritePos >= ((OPTIONS_NUM_BITSPERSAMPLE/8)*OPTIONS_NUM_CHANNELS*OPTIONS_NUM_SAMPLESPERSEC)*OPTIONS_NUM_SECONDSFORBUFFER)
		{
			dwLastEndWritePos -= ((OPTIONS_NUM_BITSPERSAMPLE/8)*OPTIONS_NUM_CHANNELS*OPTIONS_NUM_SAMPLESPERSEC)*OPTIONS_NUM_SECONDSFORBUFFER;
		}
	}


	// Victory is ours!!!!
	return TRUE;
} // end APU_DoFrame()


//------------------------------------------------------------------------------
// Name: APU_DoSquare1()
// Desc: Process the first square wave sound channel.
//------------------------------------------------------------------------------
WORD APU_DoSquare1()
{


	static DWORD dwNumCyclesElapsed = 0; // Keeps track of where we are in the wave.
	static DWORD dwDutyFlip = 0;         // How many times the programmable timer must 
	                                     // reload untill a duty flip.
	static DWORD dwWaveLength = 0;       // How many cycles till the programmable timer reloads.

	static BYTE  byOutput = 0x80; // The returned output volume.
	static BOOL  bDutyFlip = FALSE; // Has a duty flip happended?

	static BOOL bCalcSquare = FALSE;
	static BOOL bCalcOnNextFlag = FALSE;

	if (bCalculateSound)
		bCalcOnNextFlag = TRUE;
	
	// If the calculate flag is set, then we need to calculate everything
	// that is needed to return an output volume to the calling function.
	// Otherwise we just return the precalculated data that was stored
	// in the static variables.
	if (bCalcSquare)
	{
CalculateIt:
		// First thing we need to do is start over by reseting the elapsed cycles.
		dwNumCyclesElapsed = 0;
		// Reset the duty toggle.
		bDutyFlip = FALSE;

		bCalcSquare = FALSE;
		bCalcOnNextFlag = FALSE;

		//---------------------------------------------------------------------
		// Do the length counter part.
		//---------------------------------------------------------------------
		
		// If the length counter is enabled then we need to process it.
		if (!(CPU.Memory[0x4000] & 0x20))
		{
			// If the length counter is not zero then decrement the value.
			if (APU.sndchanSquare1.byLengthCtr)
				APU.sndchanSquare1.byLengthCtr--;
		}


		//---------------------------------------------------------------------
		// TODO: This is where the sweeping unit needs to be emulated.
		//---------------------------------------------------------------------


		//---------------------------------------------------------------------
		// Emulate the programmable timer to get the wavelength.
		//---------------------------------------------------------------------

		// Take the 3 least significant bits from $4003 and
		// use those as the bits 8-10 for our wavelength. Bits
		// 0-7 come from $4002 to produce our 11-bit wavelength.
		// The we need to add one to it.
		dwWaveLength = ((((WORD)(CPU.Memory[0x4003]&0x7)) << 8) | 
			CPU.Memory[0x4002]) + 1;		

		//---------------------------------------------------------------------
		// Emulate the duty flip part.
		//---------------------------------------------------------------------
		dwDutyFlip = abyDutyCycleTablePos[CPU.Memory[0x4000]>>6];
		
		//---------------------------------------------------------------------
		// Now finally send the signal through the volume/envelope decay unit.
		//---------------------------------------------------------------------

		// If the envelope decay bit is set, then the volume goes staight
		// to the DAC...or in our case DirectSound. This means that
		// the envelope decay is disabled.
		if (!(CPU.Memory[0x4000] & 0x10))
		{
			// There are a few conditions when 0 is sent straight
			// to the DAC for the volume. They are as follows:
			// 1.  If the length counter is 0
			// 2.  Something to do with the sweep unit.
			// 3.  On the negative portion of the output frequency 
			//     signal coming from the duty cycle.
			//
			// Otherwise bits (0-3) of $4000 are sent straight the
			// DAC for the volume.
			if (APU.sndchanSquare1.byLengthCtr == 0) // TODO: implement other conditions.
				byOutput = 0x80;
			else
				byOutput = ((CPU.Memory[0x4000] & 0x0F) << 3);
		}
		else
			byOutput = ((CPU.Memory[0x4000] & 0x0F) << 3);
	}

	// If we are done with the wave we need to start over.
	if (dwNumCyclesElapsed >= dwWaveLength)
	{
		// We need to flip the volume to negative if the amount of
		// times for a duty flip has passed.
		if ((dwDutyFlip--) == 0)
		{
			// If the duty flip is positive then we load the counter with the negative
			// value. If the duty flip is negative, then we load the counter
			// with the positive counter.
			if (bDutyFlip)
			{
				dwDutyFlip = abyDutyCycleTablePos[CPU.Memory[0x4000]>>6];

				if (bCalcOnNextFlag)
				{
					bCalcSquare = FALSE;
					//return (WORD)byOutput;
					goto CalculateIt;
				}
			}
			else
			{
				dwDutyFlip = abyDutyCycleTableNeg[CPU.Memory[0x4000]>>6];
			}

			// Flip the output wave.
			byOutput = -byOutput;

			// Toggle the duty flip indicator.
			bDutyFlip ^= TRUE;
		}

		dwNumCyclesElapsed -= dwWaveLength;
	}
	else
		// Keep moving along the phase of the wave form.
		dwNumCyclesElapsed += dwNumCyclesPerSample;

	// Return the final value.
	return (WORD)byOutput;
} // end APU_DoSquare1()


//------------------------------------------------------------------------------
// Name: APU_Read()
// Desc: Handles all the reads from the APU.
//------------------------------------------------------------------------------
BYTE APU_Read(WORD wReg)
{
	BYTE byRetVal = 0;

	switch (wReg)
	{
		case 0x4015:
			// If the length counter is non-zero set bit 1.
			if (APU.sndchanSquare1.byLengthCtr) byRetVal |= 0x01;

			// Return the byte.
			return byRetVal;

		default:
			return 0;
	}

	return 0;
} // end APU_Read()


//------------------------------------------------------------------------------
// Name: APU_Write()
// Desc: Handles all the writes to the APU.
//------------------------------------------------------------------------------
VOID APU_Write(WORD wReg, BYTE byData)
{
	// The old length counters for the channel since writing 0 into
	// $4015 sets the length counter to 0, but writing 1 sets the
	// length counter back to its origional value.
	static BYTE byOldLengthCounter = 0;

	switch (wReg)
	{
		case 0x4003:
			// Use a lookup table to set the length counter.
			// Bits 3-7 are used for this but they must be
			// right shifted to use the lookup table.
			APU.sndchanSquare1.byLengthCtr = abyLengthTable[byData >> 3];

			return;

		case 0x4015:
			// Deal with writes for the first square channel.
			if (byData & 0x1)
				APU.sndchanSquare1.byLengthCtr = byOldLengthCounter;
			else
			{
				// If the bit is not set the length counter is forced to 0,
				// but make sure to save the value for when a 1 gets written.
				byOldLengthCounter = APU.sndchanSquare1.byLengthCtr;
				APU.sndchanSquare1.byLengthCtr = 0;
			}
			return;

		default:
			return;
	}

	return;
} // end APU_Write()