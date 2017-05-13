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

VMFILE *vm_fileopen(const char *fname, const char *mode);
void vm_fclose(VMFILE *fp);
int vm_fread(void *buf, int size, int n, VMFILE *fp);
size_t vm_fwrite(const void *buf, int size, int n, VMFILE *fp);
int vm_fseek(VMFILE *fp, int offs, int whence);
int vm_fprintf(VMFILE *fp , const char *format , ... );
int vm_fgetc(VMFILE *fp );
int vm_fputc(int c, VMFILE *fp );
int vm_feof( VMFILE *fp );
uint16_t vm_fgeti(VMFILE *fp);
uint32_t vm_fgetl(VMFILE *fp);
void vm_fputi(uint16_t word, VMFILE *fp);
void vm_fputl(uint32_t word, VMFILE *fp);
void vm_fputstringnonull(const char *buf, VMFILE *fp);
bool vm_fverifystring(VMFILE *fp, const char *str);
void vm_fresetboolean(void);
char vm_fbooleanread(VMFILE *fp);
void vm_fbooleanwrite(char bit, VMFILE *fp);
void vm_fbooleanflush(VMFILE *fp);


#endif
