
#include "shim.h"
#include "audio.h"
#include "audio.fdh"

static HWAVEOUT hWaveOut;

static BList finished_blocks;
static CRITICAL_SECTION listlock;

static void (*BlockFinishedCallback)(void *userdata) = NULL;


bool audio_init(void)
{
WAVEFORMATEX wfx;

	InitializeCriticalSection(&listlock);
	
	// first we need to set up the WAVEFORMATEX structure.
	// the structure describes the format of the audio.
	wfx.nSamplesPerSec = AUDIO_SAMPLE_RATE;
	wfx.wBitsPerSample = AUDIO_BPP;
	wfx.nChannels = AUDIO_CHANNELS;
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	
	// try to open the default wave device. WAVE_MAPPER is
	// a constant defined in mmsystem.h, it always points to the
	// default wave device on the system (some people have 2 or
	// more sound cards).
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD)waveOutProc, 0, CALLBACK_FUNCTION) \
		!= MMSYSERR_NOERROR)
	{
		stat("unable to open WAVE_MAPPER device\n");
		return 1;
	}
	
	stat("audio: WAVE_MAPPER opened");
	return 0;
}

void audio_close(void)
{
	waveOutClose(hWaveOut);
	DeleteCriticalSection(&listlock);
}

void audio_set_callback(void (*cb)(void *))
{
	BlockFinishedCallback = cb;
}

/*
void c------------------------------() {}
*/

void audio_submit_block(uint8_t *blockdata, int size)
{
WAVEHDR *header = (WAVEHDR *)malloc(sizeof(WAVEHDR));

	ZeroMemory(header, sizeof(WAVEHDR));
	header->dwBufferLength = size;
	header->lpData = (CHAR *)blockdata;
	
	waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, header, sizeof(WAVEHDR));
}

void audio_handle_finished(void)
{
WAVEHDR *block;

	while((block = GetNextFinishedBlock()))
	{
		waveOutUnprepareHeader(hWaveOut, block, sizeof(WAVEHDR));
		//free(block->lpData);
		free(block);
		
		if (BlockFinishedCallback)
			(*BlockFinishedCallback)(NULL);
	}
}

/*
void c------------------------------() {}
*/

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, \
					DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (uMsg == WOM_DONE)
	{
		EnterCriticalSection(&listlock);
		{
			finished_blocks.AddItem((WAVEHDR *)dwParam1);
		}
		LeaveCriticalSection(&listlock);
	}
}

static WAVEHDR *GetNextFinishedBlock(void)
{
WAVEHDR *block = NULL;
int nitems;

	EnterCriticalSection(&listlock);
	{
		if ((nitems = finished_blocks.CountItems()))
			block = (WAVEHDR *)finished_blocks.RemoveItem(nitems - 1);
	}
	LeaveCriticalSection(&listlock);
	
	return block;
}

void audio_lock(void)
{
	EnterCriticalSection(&listlock);
}

void audio_unlock(void)
{
	LeaveCriticalSection(&listlock);
}


/*
void c------------------------------() {}
*/
















