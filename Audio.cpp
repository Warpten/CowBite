/*
    CowBite GBA Emulator/Debugger
    Copyright (C) 2002 Thomas Happ

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    CowBite contact info:
    Thomas Happ
    SorcererXIII@yahoo.com
    http://cowbite.emuunlim.com
*/

#include "Audio.h"
#include "gba.h"
#include "Console.h"
#include <math.h>

Audio audio;

void Audio_init(HWND hWnd) {
	memset(&audio, 0, sizeof(Audio));
	audio.dxSoundBuffer = NULL;
	//audio.updateMask = AUDIO_LENGTH32;
	audio.framesPerUpdate = 16;
	audio.frameCount = 0;
	
	//Set default sound attributes.  Currently I made this pretty arbitrary.
	//Only time will tell the best way to go about it.
	Audio_setSoundAttributes (hWnd, DIRECTSOUNDBUFFERSIZE, 44100, 8, 2);
	memset(audio.directSoundBufferA, 128, sizeof(audio.directSoundBufferA));
	memset(audio.directSoundBufferB, 128, sizeof(audio.directSoundBufferB));
	//remmeber to delete this
	//for (int i = 0; i < DIRECTSOUNDBUFFERSIZE; i++) {
	//	audio.directSoundBufferA[i] = sin(i)*255.0f;
	//}

	REG_FIFO_A[0] = REG_FIFO_A[1] = REG_FIFO_A[2] = REG_FIFO_A[3] = 0;
	REG_FIFO_B[0] = REG_FIFO_B[1] = REG_FIFO_B[2] = REG_FIFO_B[3] = 0;
		
}

void Audio_delete() {
	if (audio.dxSoundBuffer != NULL)
		audio.dxSoundBuffer->Release();
}

void Audio_setSoundAttributes (HWND hWnd, u32 bufSize, u32 frequency, u32 bps, u32 channels) {
	PCMWAVEFORMAT pcm;
	DSBUFFERDESC descriptor;
	HRESULT hResult;
	u32 bytesPerSample;
	//audio.dxBufSize = bufSize;
	audio.dxFrequency = frequency;
	audio.dxBPS = bps;
	audio.dxChannels = channels;
	bytesPerSample = bps >> 3;
	//16777216/280896 = around 60 hz

	double lcdRefresh = (double)16777216/(double)280896;
	u32 sampleFramesPerFrame = ceil((double)frequency/lcdRefresh);
	u32 bytesPerFrame = sampleFramesPerFrame * channels * (bps/8);
	u32 bytesPerUpdate = bytesPerFrame * audio.framesPerUpdate;
	//double sampleFramesPerFrame = (double)frequency/lcdRefresh;
	//double bytesPerFrame = sampleFramesPerFrame * (double)channels * ((double)bps/8);
	//double bytesPerUpdate = ceil(bytesPerFrame) * audio.framesPerUpdate;
	
	
	//I set the mix goal size to be slightly higher than the predicted amount.  This is so that
	//we will always mix slighty more bytes so that the play cursor will not advance on us.
	//Once the play cursor is audio.mixGoalSize bytes behind us... um
	//i'm not sure.
	//audio.bytesPerFrame = ceil(bytesPerFrame);
	audio.bytesPerFrame = bytesPerFrame;
	audio.bytesPerUpdate = bytesPerUpdate;
	//audio.sampleFramesPerFrame = ceil(sampleFramesPerFrame);
	audio.sampleFramesPerFrame = sampleFramesPerFrame;
	audio.mixGoalSize = audio.sampleFramesPerFrame * audio.framesPerUpdate;
	audio.numBlocks = 3;
	audio.dxBufSize = audio.bytesPerUpdate * audio.numBlocks;
	//audio.nextWritePosition = audio.mixGoalSize * 3;

	if (audio.dxSoundBuffer != NULL) {
		audio.dxSoundBuffer->Release();
		audio.dxSoundBuffer = NULL;
	}
	if (audio.dxNotify != NULL) {
        audio.dxNotify->Release();
        audio.dxNotify = NULL;
    }


	//Create a new sound buffer to reflect the new attributes.
	//All microsoft stuff :(
	if ( DirectSoundCreate(NULL, &audio.dxSound, NULL) == DS_OK) {
		//Set the coorperative level to normal
		audio.dxSound->SetCooperativeLevel (hWnd, DSSCL_NORMAL);
		
			
   		
    
		// Set up wave format structure.
		memset( &pcm, 0, sizeof(PCMWAVEFORMAT) );
		pcm.wf.wFormatTag         = WAVE_FORMAT_PCM;      
		pcm.wf.nChannels          = audio.dxChannels;
		pcm.wf.nSamplesPerSec     = audio.dxFrequency;
		
		pcm.wf.nBlockAlign        = (WORD)(audio.dxChannels * audio.dxBPS / 8);
		pcm.wf.nAvgBytesPerSec    = pcm.wf.nSamplesPerSec * pcm.wf.nBlockAlign;
		pcm.wBitsPerSample        = (WORD)audio.dxBPS;

		//Set up DSBUFFERDESC structure.
		memset(&descriptor, 0, sizeof(DSBUFFERDESC));  // Zero it out. 
		descriptor.dwSize              = sizeof(DSBUFFERDESC);
		descriptor.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2   // Always a good idea
							| DSBCAPS_GLOBALFOCUS         // Allows background playing
							| DSBCAPS_CTRLPOSITIONNOTIFY; // Needed for notification

		descriptor.dwBufferBytes       = audio.dxBufSize; 
		descriptor.lpwfxFormat         = (LPWAVEFORMATEX)&pcm;

		hResult = audio.dxSound->CreateSoundBuffer(&descriptor, &audio.dxSoundBuffer, NULL);
		if ( hResult != DS_OK) {
					
			int blah = 0;
			return;
		}
	}
	//See if it really worked...
/*	u32 blah1;
	u32 size;
	WAVEFORMATEX *wf;
	
	if (audio.dxSoundBuffer->GetFormat(NULL, NULL, &size) != DS_OK) {
		blah1= 0;
	}
	wf = (WAVEFORMATEX*)malloc(size);
	memset(wf, 0, size);
	if (audio.dxSoundBuffer->GetFormat(wf, size, &blah1) != DS_OK) {
		blah1= 0;
	}

*/
}

/////////////////////////////////////////////////////
//mix()
//This function gets called to take the audio from our pretend gba buffers,
//and up or downsample it into a mix buffer. It is this mix
//buffer that gets copied to the directsound buffer.
//Otherwise, mixing into the directsound buffer directly is risky business
//since it's circular and you shouldn't write over the play cursor.
//
//Note that I am implementing this in sample frame units which are of size 
//bytesPerSample * nChannels.  This is to make sure
//everything is aligned properly when mixing.
////////////////////////////////////////////////////
void Audio_mix(u32 mixStart, u32 mixEnd) {
//	char text[255];
	//u8* dest = audio.mixBuffer;
	//dest += audio.mixSize;
	s32 mixSize = mixEnd - mixStart;
	if (mixSize <= 0)
		return;
	float fractionA;
	float fractionB;
	s32 dataA;	//These MUST be signed
	s32 dataB;
	s32 leftOutput;
	s32 rightOutput;
	s32 maskARight = 0xFFFFFFFF;
	s32 maskALeft = 0xFFFFFFFF;
	s32 maskBRight = 0xFFFFFFFF;
	s32 maskBLeft = 0xFFFFFFFF;
	u32 shiftALeft = 0;	//Use shifting to halve or deliminate sound altogether
	u32 shiftARight = 0;//for direct sound channels.
	u32 shiftBLeft = 0;
	u32 shiftBRight = 0;

	u32 soundCntH;
	
	//Calculate the ratio between the direct sound buffer size and the mix size.
	fractionA = (float)audio.bufferAOffset / (float) mixSize;	
	fractionB = (float)audio.bufferBOffset / (float) mixSize;	

	//Set bounds.
	if (audio.bufferAOffset >= sizeof(audio.directSoundBufferA))
		audio.bufferAOffset = sizeof(audio.directSoundBufferA) -1;
	if (audio.bufferBOffset >= sizeof(audio.directSoundBufferB))
		audio.bufferBOffset = sizeof(audio.directSoundBufferB) -1;

	if (audio.bufferAOffset <= 0) {
		audio.directSoundBufferA[0] = 0;	//Mix 0 so we hear silence when fraction == 0
	}
	if (audio.bufferBOffset <= 0) {
		audio.directSoundBufferB[0] = 0;	//Mix 0
	}
	soundCntH = *REG_SOUNDCNT_H;

	if ( !(soundCntH & 0x0100))	//If not A to Right
		maskARight = 0;
	if ( !(soundCntH & 0x1000))	//If not B to Right
		maskBRight = 0;
	if ( !(soundCntH & 0x0200))	//If not A to Left
		maskALeft = 0;
	if ( !(soundCntH & 0x2000))	//If not B to Left
		maskBLeft = 0;
	
	if ( !(soundCntH & 0x0004)) {	//If A output is 50%
		shiftARight += 1;			//Halve it.
		shiftALeft += 1;
	}
	if ( !(soundCntH & 0x0008))	{//If B output is 50%
		shiftBRight += 1;
		shiftBLeft += 1;
	}


	
	for (u32 i = mixStart; i < mixEnd; i++) {
		dataA = audio.directSoundBufferA[(int)((float)(i)*fractionA)];	//Could use a lookup for this...
		dataB = audio.directSoundBufferB[(int)((float)(i)*fractionB)];
		//dataB = 127;
		//dataA = audio.directSoundBufferB[(int)((float)(i)*fractionB)];
		leftOutput = ((dataA & maskALeft) >> shiftALeft) + ( (dataB&maskBLeft) >> shiftBLeft);
		rightOutput = ((dataA&maskARight) >> shiftARight) + ((dataB&maskBRight) >> shiftBRight);
		if (leftOutput > 127)	//clip that sucka
			leftOutput = 127;
		else if (leftOutput < -128)
			leftOutput = -128;
		if (rightOutput > 127)	//clip that sucka
			rightOutput = 127;
		else if (rightOutput < -128)
			rightOutput = -128;
						
		audio.mixBuffer[(i*2)] = leftOutput+128;
		audio.mixBuffer[(i*2)+1] = rightOutput+128;
	}
	
	audio.bufferAOffset = 0;
	audio.bufferBOffset = 0;
	
}	

////////////////////////////
//mixUpdate()
//This function should be called whenever audio changes between updates.
//It mixes in the current buffer up to the current place in the mixbuffer
//we should expect to be.
//If the difference between the last mixing position and the new one is too small,
//it doesn't do anything (to fix problems in demos where registers are written
//to faster than would make a difference in the sound)
////////////////////////////
void Audio_mixUpdate() {
//	char text[255];
	float fpf = audio.sampleFramesPerFrame;
	float frameCount = audio.frameCount;
	float tickTally = gba.stats.frameTickTally;
	u32 mixPos = fpf * (frameCount + tickTally/280896.0f);
	//gba.stats.invalidWrites[0]++;
	s32 mixSize = mixPos - audio.lastMixPos;
	if (mixSize > (audio.sampleFramesPerFrame)) {
		//sprintf(text, "\nCalling mixUpdate: %d, %d", audio.lastMixPos, mixPos);
		//Console_print(text);
		//gba.stats.invalidWrites[8]++;
		Audio_mix(audio.lastMixPos, mixPos);
	}
	audio.lastMixPos = mixPos;	//Should this go outside the if?
}

/////////////////////////////////////////////////////////////////
//

//////////////////////////////////////////////////
//processFifoA()
//This function gets called whenever a timer indicates that a sample is
//to be loaded from FIFO.  It basically just takes the next bit out of fifo
//and puts it into a buffer that gets output to microsoft's direct sound each frame
//(at approximately 60 hz)
/////////////////////////////////////////////////////
int Audio_processFifoA() {
	s8* fifoA = (s8*)REG_FIFO_A;
	s8* soundBuffer;
	//If we have finished our fifo, it may be empty.

	if (audio.fifoAPos == 3) {
		//This does it 32 bits at a time, as per uze's demo.
		soundBuffer = (s8*)(&audio.directSoundBufferA[audio.bufferAOffset]);
	
		//Because the GBA represents it's PCM values as 2's complement size numbers,
		//while windows represents them as 0 - 127 for negative and 128 - 255 for positive,
		//we must convert.
		soundBuffer[3] = fifoA[3];
		soundBuffer[2] = fifoA[2];	
		soundBuffer[1] = fifoA[1];
		soundBuffer[0] = fifoA[0];

		//*soundBuffer = *fifoA;
		fifoA[3] = 0;	//Clear the fifo after reading it
		fifoA[2] = 0;	//Clear the fifo after reading it
		fifoA[1] = 0;	//Clear the fifo after reading it
		fifoA[0] = 0;	//Clear the fifo after reading it
		audio.bufferAOffset+=4;
		if (audio.bufferAOffset >= sizeof(audio.directSoundBufferA))
			audio.bufferAOffset = sizeof(audio.directSoundBufferA) - 4;
	
	
		audio.fifoAPos = 0;
		return 1;
	} 
	audio.fifoAPos++;
	return 0;
}
		
int Audio_processFifoB() {
	s8* fifoB = (s8*)REG_FIFO_B;
	s8* soundBuffer;
	//If we have finished our fifo, it may be empty.

	if (audio.fifoBPos == 3) {
		//This does it 32 bits at a time, as per uze's demo.
		soundBuffer = (s8*)(&audio.directSoundBufferB[audio.bufferBOffset]);
	
		//Because the GBA represents it's PCM values as 2's complement size numbers,
		//while windows represents them as 0 - 127 for negative and 128 - 255 for positive,
		//we must convert.
		soundBuffer[3] = fifoB[3];
		soundBuffer[2] = fifoB[2];	
		soundBuffer[1] = fifoB[1];
		soundBuffer[0] = fifoB[0];

		//*soundBuffer = *fifoB;
		fifoB[3] = 0;	//Clear the fifo after reading it
		fifoB[2] = 0;	//Clear the fifo after reading it
		fifoB[1] = 0;	//Clear the fifo after reading it
		fifoB[0] = 0;	//Clear the fifo after reading it
		audio.bufferBOffset+=4;
		if (audio.bufferBOffset >= sizeof(audio.directSoundBufferB))
			audio.bufferBOffset = sizeof(audio.directSoundBufferB) - 4;
	
	
		audio.fifoBPos = 0;
		return 1;
	} 
	audio.fifoBPos++;
	return 0;
}







void Audio_resample(u8* source, u8* dest, u32 sourceSize, u32 destSize) {
//	u32 fixedPoint;
//	char text[256];
	//I originally tried to use fixed point here, but it was not accurate
	//enough and resulted in a kind of popping sound at the end of each block.
	float fraction = (float)sourceSize / (float) (destSize/2);
	//fixedPoint = (sourceSize << 12)/ destSize;
	
	if (sourceSize > 0) {
		//First do the left.
		for (u32 i = 0; i < destSize; i+=2) {
			dest[i] = source[(int)((float)(i/2)*fraction)];
			//dest[i] = source[((i*fixedPoint)>>12)];
		}
	}
//	u32 lastIndex = (((destSize-1)*fixedPoint)>>12);
	//sprintf(text, "\nsourceSize: %d, (destSize-1 * fixedPoint)>>12: %d", sourceSize, lastIndex);
	//Console_print(text);
}

//////////////////////////////////////////
//playSound()
//This function should be called periodically (60hz) to do the following:
// -- Take the mix buffer, created by Audio_mix(), and blit it to
//the directx buffer.
//////////////////////////////////////////
void Audio_playSound() {
	u32 playCursor, writeCursor, writePosition;
	void * write1;
	u32 length1;
	void * write2;
	u32 length2;
	HRESULT hResult;
	u32 status;
	write1 = write2 = NULL;	//INitialize them to null
//	char text[256];
	//u32 mixSize;
	//static u32 writeCursorWrapped = 0;
	//static u32 writePositionWrapped = 0;
	
	u32 currentBlock;
	//static int blah = 0;
	//Audio_mix(); //FIrst mix.

	//If we have a sound buffer
	if (audio.dxSoundBuffer != NULL) {
		if (! SUPPORT_GETBIT7 (*REG_SOUNDCNT_X) ) {	//If sound circuits are off,
			audio.bufferAOffset = audio.bufferBOffset = 0;
			audio.dxSoundBuffer->Stop();			//stop playing.
			return;									//Leave this function.
		}
		audio.dxSoundBuffer->GetStatus(&status);	//Find if it's playing
		//If it's not already playing, play it (will I regret this?)
		if ((status & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) {
			audio.dxSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);	//Loop the sound
		}

		//Get the current position so we can figure out where to start writing
		audio.dxSoundBuffer->GetCurrentPosition(&playCursor, &writeCursor);

		//Find out what the next block it.
		currentBlock = ((writeCursor / audio.bytesPerUpdate) + 1) % audio.numBlocks;
		while (currentBlock == audio.lastBlock) {

			//if for some reason we're still on the same block,
			//sync it on up.
			audio.dxSoundBuffer->GetCurrentPosition(&playCursor, &writeCursor);

			currentBlock = ((writeCursor / audio.bytesPerUpdate) + 1) % audio.numBlocks;

		}
		audio.lastBlock = currentBlock;

		writePosition = currentBlock * audio.bytesPerUpdate;

		//Mix the audio streams together into one buffer, which we blit to directsound.
		//mixSize = audio.mixGoalSize;
		//Console_print("\nMixing from play()");
		Audio_mix(audio.lastMixPos, audio.mixGoalSize);
		audio.lastMixPos = 0;	

		
		u32 distance;
		if (writePosition < writeCursor ) {	//if we looped
			distance = audio.dxBufSize - writeCursor + writePosition;
		} else {
			distance = writePosition - writeCursor;
		}
		audio.lastPlayCursor = playCursor;
		//sprintf(text, "\ndistance: %d, currentBlock: %d, samples: %d", distance, currentBlock,
		//				audio.bufferAOffset);
		//Console_print(text);

		
		//audio.lastWriteCursor = writeCursor;				
		//audio.lastWritePosition = writePosition;

		//Lock from the write cursor position up to the offset in our buffer
		//to which we have written
		if (audio.mixGoalSize > 0 ) {
			hResult = audio.dxSoundBuffer->Lock(writePosition, audio.bytesPerUpdate, &write1, &length1, &write2, &length2, 0);
			//If the buffer was lost
			if (hResult == DSERR_BUFFERLOST) {
				audio.dxSoundBuffer->Restore();
				//I wonder what happens if it gets lost again?  Are you screwed?
				hResult = audio.dxSoundBuffer->Lock(writePosition, audio.bytesPerUpdate, &write1, &length1, &write2, &length2, 0);
			}
			//If everything went smoothly, do our thing.
			if (hResult == DS_OK) {
				memcpy (write1, audio.mixBuffer, length1);
				//If we looped past the end of the sound buffer, write the second
				//part.  Since we are using blocks evenly divisible into the buffer size,
				//this should never happen.
				if (write2 != NULL) {
					memcpy (write2, audio.mixBuffer+length1, length2);
				}
							
				hResult = audio.dxSoundBuffer->Unlock(write1, length1, write2, length2);

			} else {
					switch (hResult) {
					case DSERR_BUFFERLOST: hResult = 0;
					case DSERR_INVALIDCALL: hResult = 1;
					case DSERR_INVALIDPARAM: hResult = 2;
					case DSERR_PRIOLEVELNEEDED: hResult = 3;
					}
			}
	
		} else {	//Otherwise, we have no audio, so write dead air.up to the play cursor
			hResult = audio.dxSoundBuffer->Lock(writePosition, audio.dxBufSize-writePosition+playCursor, &write1, &length1, &write2, &length2, 0);
			//If the buffer was lost
			if (hResult == DSERR_BUFFERLOST) {
				audio.dxSoundBuffer->Restore();
				hResult = audio.dxSoundBuffer->Lock(writePosition, audio.dxBufSize-writePosition+playCursor, &write1, &length1, &write2, &length2, 0);
			}
			//If everything went smoothly, set the buffer to 0.
			if (hResult == DS_OK) {
				memset (write1, 128, length1);	//Write silence
				if (write2 != NULL) {
					memset (write2, 128, length2);	//Write silence
				}
				hResult = audio.dxSoundBuffer->Unlock(write1, length1, write2, length2);

			}


		}
		

		
	}


}

//////////////////////////////////
//This synchronizes the gba audio to around 60 hz
//Returns 0 if no audio is playing.
/////////////////////////////////
int Audio_sync() {
	u32 playCursor, writeCursor, status;
	u32 subBlock;	//an interval of size bytesPerFrame
	//char text[256];
	audio.dxSoundBuffer->GetStatus(&status);	//Find if it's playing
	//If it's not already playing, play it (will I regret this?)
	if ((status & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) {
		return 0;
	}
	

	//Get the current position so we can figure out where to start writing
	audio.dxSoundBuffer->GetCurrentPosition(&playCursor, &writeCursor);
//	if (playCursor < audio.lastPlayCursor)
//		Console_print("\nlooped.");
//	audio.lastPlayCursor = playCursor;

	//Find out where we are in the buffer, in units equal to the number of
	//bytes we want to play in a frame.
	subBlock = writeCursor /(audio.bytesPerFrame* graphics.framesPerRefresh);
	


	while (subBlock == audio.lastSubBlock) {
		audio.lastSubBlock = subBlock;
		audio.dxSoundBuffer->GetCurrentPosition(&playCursor, &writeCursor);
		subBlock = writeCursor /audio.bytesPerFrame;
//		if (playCursor < audio.lastPlayCursor)
//			Console_print("\nlooped.");
//		audio.lastPlayCursor = playCursor;
	}
//	if ((s32)(subBlock - audio.lastSubBlock) < 0) {
//		sprintf(text, "\nsubBlock - lastSubBlock: %d", subBlock - audio.lastSubBlock);
//		Console_print(text);
//	}
	audio.lastSubBlock = subBlock;
	return 1;
}

////////////////////////
//This stops all sound.
//////////////////////
void Audio_stopSound() {
	if (audio.dxSoundBuffer != NULL) {
		//audio.bufferAOffset = 0;
		audio.dxSoundBuffer->Stop();
	}
}
