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

#ifndef AUDIO_H
#define AUDIO_H

#include <windows.h>
#include <dsound.h>
#include "Support.h"
#include "IO.h"
#include "Memory.h"

#define MAX_AUDIO_EVENTS 16

//#define DIRECTSOUNDBUFFERSIZE 4096	//For a refresh of 60hz
#define AUDIO_LENGTH1 0	//For a refresh of 60hz (16.67 ms) - update every refresh
#define AUDIO_LENGTH2 1	//For a refresh of 30hz (33.33 ms) - update every 2 refreshes
#define AUDIO_LENGTH4 3	//For a refresh of 15hz (66.67 ms) - update every 4 refreshes
#define AUDIO_LENGTH8 7	//For a refresh of 7.5hz (133.33 ms) - update every 8 refreshes
#define AUDIO_LENGTH16 0xF	//For a refresh of 3.75hz (266.67 ms) - update every 16 refreshes
#define AUDIO_LENGTH32 0x1F	//For a refresh of 533.33 ms - update every 32 refreshes
#define AUDIO_LENGTH64 0x3F	//For a refresh of 1066.67 ms - update very 64 refreshes



//Now for some math.  
//ClockSpeed:  2^24 cycles/sec
//MaximumSampleRate:  65000 bytes/sec	(samples are 1 byte)
//CyclesPerRefresh = 280896 cycles/refresh
//RefreshRate = CyclesPerRefresh/ClockSpeed = refreshes/sec
//MaxBytesPerRefresh = MaximumSampleRate/RefreshRate = 65000/(CyclesPerRefresh/ClockSpeed)
//MaxBytesPerRefresh = 65000/(2^24/280896) = 1088.2759

//To get the maximum number of bytes at a very slow latancy (if you refresh the dsound buffer
//every 64 frames, about once a second)...
//MaxBytesPerUpdate = MaxBytesPerRefresh * MaxRefreshes = 1088.2759 * 64 = 69,649.65

//Now, if we want to up or downsample, we need to figure out the goal refresh rate
//(let's say 44.1 khz) and calculate the ratio of this versus the number of samples
//that the gba transfered (or would have transfered) during this update.  It may be
//necessary to upsample more frequently than we update to directx (say once a refresh)
//in order to prevent stretching artifacts.
//GoalSampleRate = 441000 bytes/second (example)
//Now find the number of bytes we'll need in a single refresh
//GoalRefreshSize = GoalSampleRate / (refreshes per second)
//GoalRefreshSize = GoalSamplerate /RefreshRate = 44100/(2^24/280896) = 738.35
//This may be fine, if I choose to upsample once a frame.  But if not . ..
//GoalUpdateSize = GoalRefreshSize * FramesPerUpdate; //max 64 frames per update

//A good reason to update once per frame rather than after several frames is that the program
//made change sample rates somewhere inbetween, or just stop the sound; in which case
//cowbite would see less (or more?) bytes than usual and try to stretch them out, resulting in
//a pitch shift.  

//However, it might be possible that whenever the frequency changes
//or sound stops, we immediately perform an upsample on the current bits in the source buffer
//to an apprpriate fraction of the desination buffer; then on the next update
//we do the upsample on the remaining fraction of the destination buffer.  then go back to normal.


//Another alternative would be to have a clock calculate the number of seconds that have
//elapsed.  But if something were to slow the emulator down (user opened up something else)
//there would be a distinct pitch change.  So I think that probably is out of the question.
//GoalRefreshSize = GoalSampleRate * SecondsElapsed

#define DIRECTSOUNDBUFFERSIZE 70000		//Maximum bytes needed at a refresh of 1 hz
#define MAXDXSOUNDBUFFERSIZE 280000		//Maximum to convert the above to 16 bit stereo

typedef struct tAudio {
	//u32 lastWriteCursor;	//The lats position of dx's write cursor.
	//u32 lastWritePosition;	//Th elast positino we wrote to
	//u32 nextWritePosition;	//Next position we'll want to write to
	u32 frameCount;
	//u32 updateMask;			//use to mask out the frames counter to determine update
	u32 framesPerUpdate;
	//u32 mixMask;
	//u32 framesPerMix;
	u32 sampleFramesPerFrame;	//Number of sample frames per frame
	u32 bytesPerFrame;		//Number of bytes we wait between frames.
	u32 bytesPerUpdate;		//Number of bytes we wait between updates
	int fifoAPos;
	int fifoBPos;
	u32 bufferAOffset;
	u32 bufferBOffset;
	s8 directSoundBufferA[DIRECTSOUNDBUFFERSIZE];
	s8 directSoundBufferB[DIRECTSOUNDBUFFERSIZE];
	u8 mixBuffer[MAXDXSOUNDBUFFERSIZE];
	//u32 mixSize;
	u32 mixGoalSize;
	u32 lastMixPos;		//Last position in the mix buffer we mixed up to
	u32 numBlocks;
	u32 lastBlock;		//Last "chunk" of audio played
	u32 lastSubBlock;	//Last block of size "bytesPerFrame" blayed.
	u32 lastPlayCursor;	//Used for syncing to audio.

	//s8 deadAir[DIRECTSOUNDBUFFERSIZE];
	LPDIRECTSOUNDBUFFER dxSoundBuffer;
	LPDIRECTSOUND dxSound;
	LPDIRECTSOUNDNOTIFY dxNotify;
	DSBPOSITIONNOTIFY dxNotifyEvents[MAX_AUDIO_EVENTS];
	HANDLE cursorEvents[MAX_AUDIO_EVENTS];
	u32 numEvents;
	u32 dxBufSize;
	u32 dxFrequency;
	u32 dxBPS;
	u16 dxChannels;	//Meaning 1 for mono, 2 for stereo
} Audio;

void Audio_init(HWND hWnd);
void Audio_delete();
void Audio_setSoundAttributes (HWND hWnd, u32 bufSize, u32 frequency, u32 bps, u32 channels);
void Audio_mix();
void Audio_mixUpdate();
void Audio_resample(u8* source, u8* dest, u32 sourceSize, u32 destSize);
void Audio_playSound();
int Audio_sync();
void Audio_stopSound();
int Audio_processFifoA();
int Audio_processFifoB();

extern Audio audio;







#endif