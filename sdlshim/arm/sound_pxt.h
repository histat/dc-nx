
#define PXT_NUM_CHANNELS 14

struct pxt_status {
	int samplepos;
};

struct pxt_buffer {
	int buffer;
	int len;
};


#define CMD_SET_LOOP(n) (0x30|(n))
#define CMD_SET_PLAYCHAN(n) (0x40|(n))
#define CMD_SET_FREEPOS(n) (0x50|(n))
#define CMD_SET_PAUSECHAN(n) (0x60|(n))

#define PXT_STATUS_ADDR (0x40400)
#define PXT_BUFFER_ADDR (0x40500)
#define SPU_BASE_ADDR (0x41000)

#define PXT_STATUS ((volatile struct pxt_status *)(void *)(0xa0800000+PXT_STATUS_ADDR))

#define PXT_BUFFER(n) ((volatile struct pxt_buffer *)(void *)(0xa0800000+PXT_BUFFER_ADDR+sizeof(pxt_buffer)*(n)))

