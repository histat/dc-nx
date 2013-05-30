#ifndef _VMFILE_H_
#define _VMFILE_H_

struct VMFILE
{
	int _fd;
	char *_ptr;
	int _cnt;
	char *_base;
	char _vm;
	char filename[32];
};


class VMFileBuffer
{
public:
	VMFileBuffer();
	void SetBufferSize(int maxsize);
	void SetFile(VMFILE *fp);
	
	void Write8(uint8_t data);
	void Write16(uint16_t data);
	void Write32(uint32_t data);

	void Flush();
	void Dump();
	
private:
	void CheckFlush(int maxsize);
	
	int _pos;
	int fMaxSize;
	
	VMFILE *fFP;
	char fbuffer[512];
};

#endif
