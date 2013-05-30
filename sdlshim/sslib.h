
#ifndef _SSLIB_H
#define _SSLIB_H

#define MAX_QUEUED_CHUNKS		(180 +1)
#define SS_NUM_CHANNELS			32
#define SAMPLE_RATE				22050
#define SDL_MIX_MAXVOLUME		128

struct SSChunk
{
	signed short *buffer;
	int length;
	
	signed char *bytebuffer;			// same as bytebuffer but in BYTES
	int bytelength;						// TOTAL length in BYTES
	
	// current read position. this is within bytebuffer and is in BYTES.
	int bytepos;
	
	void *userdata;						// user data to be sent to FinishedCallback when finished
};


struct SSChannel
{
	SSChunk chunks[MAX_QUEUED_CHUNKS];
	int head, tail;
	
	int volume;
	bool reserved;						// if 1, can only be played on explicitly, not by passing -1
	
	void *FinishedChunkUserdata[MAX_QUEUED_CHUNKS];
	int nFinishedChunks;
	
	void (*FinishedCB)(int channel, void *userdata);
};

#endif
