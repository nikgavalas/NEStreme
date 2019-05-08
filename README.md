# NEStreme
NES Emulator project I wrote in college. This source code hasn't been touched in years and is meant more as a 
reference for anyone interested. Below is the paper I wrote about the project.

## How to write an emulator by Nikolas Gavalas

### Introduction
Emulating your favorite system to play games is easier than you think depending on the system you’re trying to emulate. 
This document is intended to give you the general idea of building an emulator. To help you understand the concepts
more I will give specific examples to the NES emulator that I have been working on this semester at California State University
of Long Beach.

This document assumes that the reader is familiar with C++ and Assembly.

## Stages

There are really only two major stages that are involved in emulating. They are loading the game or application (ROM) 
and running the ROM. Most of your work will be spend in the running stage. The loading stage is pretty straight forward.

### Loading

The first thing you want to do when loading the ROM is load the whole file into memory. How you load it into memory 
depends on what system you’re emulating. I’ll explain how it works on the NES for a good example.

The NES has two types of memory on the cartridge, program and character. The program memory is the actual instructions
to be executed by the CPU. The character memory is the 8x8 pattern tiles used for displaying the graphics. For the NES,
I load all the program memory into an array and all the pattern tiles into another array.

The NES only has 32k of program memory and 8k of character memory. This limits the types and quality of game you can 
make with such a limited amount of memory. To overcome this, some cartridges have what’s called memory mappers. These 
are used to swap memory from the cartridge to the memory on the NES. A mapper swap occurs when data is written to one 
of the registers on the cartridge. These registers tell the cartridge which memory bank to use.

Since you have already loaded the entire cartridge into memory, you can make swapping really fast. Rather than 
really copying memory to and from the different arrays, you can just change a pointer. For example, if you were required
to swap the first bank of program memory, you would just change the variable that points to the bank in use on the cartridge.

```cpp
pPRGROMBank1 = &abyPRGROM[PRGROM_BANKSIZE*wBankNumber];
```

Now that we have some background information about how to load the ROM into memory and deal with memory mappers, 
I’ll explain what needs to be done when the ROM is loaded. 

You must first set up the PRG-ROM memory bank(s) in a similar way to dealing with memory swapping via memory mappers. 
The NES has to program banks, each 16k. So depending on which memory mapper is being used, you have to assign the pointers 
appropriately. In the code example below, wBankNumber1 and wBankNumber2 are the bank numbers to use for each PRG-ROM area.

```cpp
pPRGROMBank1 = &abyPRGROM[PRGROM_BANKSIZE*wBankNumber1];
pPRGROMBank1 = &abyPRGROM[PRGROM_BANKSIZE*wBankNumber2];
```

The next thing you need to do is set the Instruction pointer to the starting address of the program. Since I do not have 
much experience with other systems, I’ll explain how the NES works. At a specified address in the PRG-ROM (0FFFEh), there 
is a word that holds the starting value of the Instruction Pointer. You copy that word into the Instruction Pointer and 
start execution to wherever the Instruction Pointer points to.

Other stuff can be done during loading as well. You can initialize the graphics and sound. Another option is starting a 
debugger. There choice is yours really. Now your ready to run the ROM.

### Running

Running the ROM can be broken down into a few more sections. In order to run the ROM you must:

1. Fetch the opcode from the ROM.
2. Execute the opcode.
3. Execute Interrupts.
4. Read/Write to memory.
5. Do the cyclic tasks.

#### Fetching the opcode

The first step to running the ROM is fetching the opcode. You get wherever byte the CPU’s Instruction Pointer (IP) is 
pointing to. This byte is your opcode. Depending on the opcode you may want to fetch the operands or you may want to 
save this part for the executing part. For an 8-bit processor, the code would be like the following.

```cpp
byOpcode = pPRGROMBank1[wInstructionPointer];
```

Where CPU.pPRGROMBank1 points to the PRG-ROM bank that was loaded in the beginning and wInstructionPointer is the 
Instruction Pointer. After this statement is executed, byOpcode will hold the numeric value of your opcode.
Executing the opcode

Now that you have your opcode byte, you need to decode and execute it. Decoding is much simpler if the processor you’re 
emulating is 8-bit. If it’s a 16-bit processor, most of this is relatively the same I imagine. But, to make this process 
more simple and to the point, lets just stick with an 8-bit processor. 

Your opcode byte is an instruction. For example, on the 6502, the value 29h is a form of an AND instruction. In this case, 
it is an AND Immediate, which means that it ands the Accumulator register with the immediate value following the opcode byte. 
So in this case, if you were writing an emulator for the 6502, you would perform an AND on the Accumulator register with 
the byte following the AND opcode byte. Then you would increment the IP and add or subtract the number of clock cycles it 
took to execute that particular instruction.

The code for the AND operation is below. The other operations are just like this with different cases.

```cpp
switch (byOpcode)
{
	case 0x29:
		byOperand1 = pPRGROMBank1[wInstructionPointer+1];
		A &= byOperand1;
		break;

	.
	.
	.
}

wInstructionPointer += wNumBytesForOpcode;
dwCPUCycles -= dwNumCyclesForOpcode;
```

#### Execute Interrupts

An interrupt can be either a hardware or software interrupt. Hardware interrupts happen when some hardware condition occurs. 
For example, when VBlank occurs, an interrupt can happen. A software interrupt happens when the program executes a certain 
instruction. On the NES, this would happen when a BRK instruction is executed.

When an interrupt occurs, the NES saves the Instruction Pointer and Flags registers and then looks in the particular area of
memory for a new value to put in the Instruction Pointer. Each interrupt has its own assigned address, so it gets the value at 
that location and puts it in the Instruction Pointer. Executing then continues from this point.

The NES has three interrupts. The new value of the Instruction Pointer is at these addresses in memory depending on the 
interrupt that has occurred.

FFFAh = NMI (VBlank)
FFFCh = RESET
FFFEh = IRQ/BRK (software)

So when VBlank occurs the CPU would:

1. Push the current value of the Instruction Pointer onto the stack.
2. Push the current value of the Flags register onto the stack.
3. Get the word value at FFFAh and copy that into the Instruction Pointer.

#### Reading And Writing To Memory

There are two different ways to read and write to memory. Getting the opcode and the opcode’s operands would be considered 
the simpler form of reading from memory and not too much emulation has to take place during this step. The other way is when 
an instruction has to go out and read/write from memory. This is the one you’ll be most concerned with.

To emulate this method, you will need to figure out which instructions actually require a read from or a write to memory.
On the NES’s cpu (the 6502), the instructions are LDA (read) and STA (write). 

Depending on what the address of the read or write is, certain events must occur. The particular address of memory may be a
register, mirrored memory, or whatever else your system may have. I’ll provide an example that has addresses specific to the
NES but will illustrate the point. Writing to memory uses the same idea as reading from memory.

```cpp
BYTE ReadMemory(WORD wAddress)
{
	switch (wAddress)
	{
		case 0x2002:
		// Emulate the PPU status register.
		break;

		case 0x2004:
		// Return a byte from sprite memory.
		break;

		.
		.
		.
		
		default:
		// Return a byte at the address passed in.
	}
}
```

#### Doing the Cyclic Tasks

Choosing where and when you want to execute all the cyclic tasks is a choice that you will have to make depending on the system 
you are emulating. It can be at the end of drawing a scanline, after drawing a full screen, or even after every instruction. 
The more you do the cyclic tasks, the more performance loss you will have, but you may get more accuracy. It goes the same for 
the other way as well. If you do the cyclic tasks less you may get faster emulation speeds but loose some accuracy. It’s up to 
you to design your emulator, so I leave the choice up to you.

Once you have decided where to put your cyclic tasks, you must decide what they are. Some examples can be executing interrupts, 
incrementing the scanline that is being drawn, or anything else you can think of that must be done at certain intervals.

For my NES emulator I chose to do the cyclic tasks after every scanline is drawn. I keep track of which scanline the screen is 
drawing by the number of cpu cycles that have passed. I calculate the number of cpu cycles per scanline as follows:

```cpp
NumCyclesPerScanline = (CPUFrequency / RefreshRate) / NumScanlinePerFrame
```

### Video Emulation

Emulating the graphics is a very system dependent subject. In this section, I will explain some general ideas towards emulating
graphics, but most of the material covered here will be specific to the NES. If you’re emulating another system besides the NES, 
I hope that this information will be useful; otherwise, just ignore it.

On television sets images are displayed via an electron beam. If you look really closely at a television set, you can see the
individual pixels. The pixels change color by the electron beam starting from the left side of the screen and moving to the right 
side of the screen. It does this line by line doing some mojo along the way to make the pixels change color. This is called 
drawing a scanline. When the beam has finished with the scanline, it moves back to the left side of the screen and moves down 
to the next scanline. The time that the beam takes to move back to the left side of the screen is called the HBlank period. 
Once the beam has drawn all the scanlines on the screen, which is 240 on a NTSC television set, it moves back up to the top
left corner of the screen. The time that is takes for the beam to move back to the top left corner of the screen is called the
VBlank period.

With all that information behind us, we can talk about some general guidelines to displaying the graphics. The way I see it, you 
can either draw graphics every scanline, or you can wait and draw everything during the VBlank period. I’ll discuss the 
differences between the two methods.

Drawing the graphics every scanline is more accurate but also not as fast. To emulate this method you would run the cpu for the 
number of cycles that it takes to draw a scanline (as calculated above). For the NES, this is 114 (rounded up). Then you would 
draw that scanline. For the VBlank period you would continue to emulate the cpu, but not draw the scanline until VBlank is over.

Draw the entire screen at once can be much faster, but may loose some accuracy if not emulated correctly. To emulate this method, 
you would run the cpu for 240 scanlines then draw all the scanlines at once. You can loose accuracy if the system your emulating 
changes values (scroll values, palette entries) while the screen is drawning.

For my NES emulator, I chose to use the scanline method for ultimate accuracy. For more information about the NES and the graphics
system, check out Yoshi’s NES document.

http://www.zophar.net/tech/files/ndox200.zip

### Sound Emulation

For me, sound was the hardest thing to emulate. It was mostly due to programming DirectSound Buffers. I’ll explain the approach 
I used and leave the rest up to you in hopes that you can find where I went wrong and correct my mistakes.

First off, you will need to study streaming DirectSound Buffers. Microsoft has plenty of information about them in their MSDN. 
It would be a waste of time for me to try and explain them when they have much more information and better writers, hehe.

My algorithm is at the end of every frame, I update the DirectSound Buffer with the sound data. These are the steps that I 
use for emulating a channel that outputs a square wave at a desired frequency, volume, etc.:

1. Calculate the wavelength of the wave. This is done in CPU cycles and is read from some sound registers.
2. Calculate the duty cycle for the wave. This is how many time to repeat the wave length until the wave must flip over and be negative. Again this is read from sound registers.
3. Calculate the output volume. This is read from sound registers. You can think of this as the amplitude of the wave.

I only do this at the beginning of the frame. Then I move along the wave and write the data out to the DirectSound Buffer. The 
sample rate determines how many bytes to write to the buffer. I only write as many bytes as there are samples in one cpu frame. 
On the next frame I continue where I left off writing and write the rest of the data. When I am done writing all the data for
the wave I calculated for, I recalculate steps 1-3 and then repeat the whole process. For example:

```cpp
NumBytesToLock=(((NumBitsPerSample/8)*NumChannels*NumSamplesPerSec)/30)
```

Below is my code to emulate one square wave channel on the NES to help clarify what I mean.

```cpp
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
```

### Optimization

For this section, I will list some general optimization methods and some methods that are specific to the NES.

1. Unroll loops. If you have a loop that you know you are going to be doing a certain amount of times, just cut and paste the same code. For example, if you have a for loop that executes some code three times, then just execute that code 3 times rather than using looping. Looping introduces compares and jumps in assembly language which take longer to execute than just the straight code.

```cpp
for (int i=0; i < 3; i++)
{
	// Some Code
}
```

This could be written as:

```cpp
// Some Code
// Some Code
// Some Code
```

2. Rewrite sections of code that get used a lot in assembly language. Loops are one example of code that gets executed a lot. Another example is instead of using switch and case statements in C++, you could use what’s called a jump table in assembly language. A jump table is an array that holds addresses. Whatever the case value is, you go to that position in the array and move that value into a 32-bit register. Then you jump to the value that is in that register. This idea is very useful when emulating the Read and Write memory functions and the opcode switch statement.

```
; Get the opcode byte.
GET_MEMORY_BYTE CPU.P

; Save the opcode for later use and then jump to the
; correct opcode using the jump table defined above.
and eax, 000000FFh
mov dwOpcode, eax
jmp [adwOpcodeJumpTable+eax*4]
```

3.	For the graphics on the NES, use a tile caching algorithm. This will make sense when you read how NES graphics work. They are explained in Yoshi’s NES document. http://www.zophar.net/tech/files/ndox200.zip

### Lessons Learned

1.	Don’t do this project during a semester at school ;-)
2.	When emulating the NES, read different documents on the 6502. They have different information. For example, one document I read said that PLA did not set the flags on the CPU when another document I read said that it does in fact set the flags.

### My Emulator

You can use the source code to my emulator in anyway to help you in your quest. The project was done using Visual C++ .NET and 
the DirectX 8 SDK.

To run a NES ROM on my emulator, you start the program, select Open Rom from the file menu, and then select Run from the file 
menu. While running the ROM, use the arrow keys for up, down, left, and right, S for Start, A for Select, Z for B, X for A. 
Pressing ESC will stop running the ROM.

To debug a ROM in my emulator you can double click on any line to add or remove a breakpoint. You can use the Debug menu 
to step through the code or run until an NMI interrupt. Use the View menu to view memory.

Reference

[Zophar’s Domain](http://www.zophar.net/index.phtml)
Holds tons of information on emulators as well as the emulators themselves.

[NES info, programs, and demos](http://nesdev.parodius.com/)
Holds over 2 thousand pounds of programming information on the NES as well as some free demos.
