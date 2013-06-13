/*-----------------------------------------------------------------------*/
/* MMC/SD (in SPI mode) control module  (C)ChaN, 2007                    */
/*-----------------------------------------------------------------------*/
/* Only spi_wr(), spi_wr(), disk_timerproc() and some macros are     */
/* platform dependent.                                                   */
/*-----------------------------------------------------------------------*/


#include "diskio.h"
#include "sci.h"
#include <time.h>
#include "crc.h"


typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;

/* MMC/SD command (in SPI) */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD41	(0x40+41)	/* SEND_OP_COND (ACMD) */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */
#define CMD59	(0x40+59)	/* CRC_ON_OFF */


/* Control signals (Platform dependent) */
#define SELECT()	rts_bit(0)		/* MMC CS = L */
#define	DESELECT()	rts_bit(1)		/* MMC CS = H */

#define SOCKPORT	IO.PDR5.uchar	/* Socket control port */
#define SOCKWP		0x10			/* Write protect switch (PB5) */
#define SOCKINS		0x20			/* Card detect switch (PB4) */
#define CARDPWR		0x80			/* Card power (PE7) */

//#define	MAX_RETRY	100
#define	MAX_RETRY	1000

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
uchar Timer1, Timer2;	/* 100Hz decrement timer */

//static
uchar CardType;			/* b0:MMC, b1:SDC, b2:Block addressing */

static void wait_ms(int ms) {
    int i, cnt;
    volatile ulong *a05f688c = (volatile ulong *)0xa05f688c;

    cnt = 0x1800 * 0x58e * ms / 1000;
    for (i=0; i<cnt; i++)
        (void)*a05f688c;
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
uchar wait_ready (void)
{
        int i;
	uchar res;


	(void)spi_r();
	i = 0;
	do {
		res = spi_r();
		i++;
	} while ((res != 0xFF) && i < 500000);
	return res;
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
        sci_init();
	wait_ms(20);
}


static
void power_off (void)
{
	SELECT();				/* Wait for card ready */
	wait_ready();
	DESELECT();
	(void)spi_r();
	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL rcvr_datablock (
	uchar *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be even number) */
)
{
	uchar token;
	int i;
	
	int len;
	uchar *p;
	ushort crc, crc2;

	len = btr;
	p = buff;
	i = 0;
	do {							/* Wait for data packet in timeout of 100ms */
		token = spi_r();
		++i;
	} while ((token == 0xFF) && i < 30000);

//printf("token = 0x%02X\n",token);
	if(token != 0xFE) return FALSE;	/* If not valid data token, retutn with error */

	do {							/* Receive the data block into buffer */
		*buff++ = spi_r();
		*buff++ = spi_r();
	} while (btr -= 2);
	crc2 = (ushort)spi_r() << 8;	// get CRC16 high
	crc2 |= (ushort)spi_r();	// get CRC16 low
	crc = crc16(p, len);	// calc CRC16

	return(crc == crc2 ? TRUE : FALSE);					/* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
BOOL xmit_datablock (
	uchar *buff,	/* 512 byte data block to be transmitted */
	uchar token			/* Data/Stop token */
)
{
	uchar resp, wc;
	ushort crc;

	if (wait_ready() != 0xFF) return FALSE;

	spi_w(token);					/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
		crc = crc16(buff, 512);	// calc CRC16
		wc = 0;
		do {							/* Xmit the 512 byte data block to MMC */
			spi_w(*buff++);
			spi_w(*buff++);
		} while (--wc);
		spi_w((uchar)(crc >> 8));	// put CRC16 high
		spi_w((uchar)crc);		// put CRC16 low
		resp = spi_r();				/* Reveive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}
	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
uchar send_cmd (
	uchar cmd,		/* Command byte */
	ulong arg		/* Argument */
)
{
	uchar n, res;
	uchar cb[6];

	if (wait_ready() != 0xFF)
	{
	 return 0xFF;
	}

	cb[0] = cmd;
	cb[1] = (uchar)(arg >> 24);
	cb[2] = (uchar)(arg >> 16);
	cb[3] = (uchar)(arg >> 8);
	cb[4] = (uchar)arg;
	cb[5] = crc7(cb, 5);
	/* Send command packet */
	spi_w(cmd);						/* Command */
	spi_w(cb[1]);		/* Argument[31..24] */
	spi_w(cb[2]);		/* Argument[23..16] */
	spi_w(cb[3]);		/* Argument[15..8] */
	spi_w(cb[4]);		/* Argument[7..0] */
	spi_w(cb[5]);           // CRC7

	/* Receive command response */
	if (cmd == CMD12) (void)spi_r();		/* Skip a stuff byte when stop reading */
	n = 20;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = spi_r();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}

static uchar send_slow_cmd (
	uchar cmd,		/* Command byte */
	ulong arg		/* Argument */
)
{
	uchar n, res;
	uchar cb[6];

        int i;

	(void)spi_slow_wr(0xff);
	i = 0;
	do {
		res = spi_slow_wr(0xff);
		i++;
	} while ((res != 0xFF) && i < 100000);
	if (res != 0xff) return(0xff);

	cb[0] = cmd;
	cb[1] = (uchar)(arg >> 24);
	cb[2] = (uchar)(arg >> 16);
	cb[3] = (uchar)(arg >> 8);
	cb[4] = (uchar)arg;
	cb[5] = crc7(cb, 5);
	/* Send command packet */
	spi_slow_wr(cmd);		/* Command */
	spi_slow_wr(cb[1]);		/* Argument[31..24] */
	spi_slow_wr(cb[2]);		/* Argument[23..16] */
	spi_slow_wr(cb[3]);		/* Argument[15..8] */
	spi_slow_wr(cb[4]);		/* Argument[7..0] */
	spi_slow_wr(cb[5]);		// CRC7

	/* Receive command response */
	if (cmd == CMD12) (void)spi_slow_wr(0xff);		/* Skip a stuff byte when stop reading */
	n = 20;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = spi_slow_wr(0xff);
	while ((res & 0x80) && --n);
	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

DWORD get_fattime ()
{
	time_t curtime;
	time(&curtime);

	struct tm *t = localtime(&curtime);
  
	DWORD rv =  (((DWORD)t->tm_year - 80) << 25)
		| ((DWORD)(t->tm_mon + 1) << 21)
		| ((DWORD)t->tm_mday << 16)
		| (WORD)(t->tm_hour << 11)
		| (WORD)(t->tm_min << 5)
		| (WORD)(t->tm_sec >> 1);
	return rv;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	uchar drv		/* Physical drive nmuber (0) */
)
{
	int i;
	uchar n, ty, ocr[4];

	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	for (n = 10; n; n--) (void)spi_slow_wr(0xff);	/* 80 dummy clocks */

	SELECT();				/* CS = L */
	ty = 0;
	if (send_slow_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		delay_m(300);
		//Timer1 = 100;						/* Initialization timeout of 1000 msec */
		i = 0;
		if (send_slow_cmd(CMD8, 0x1AA) == 1) {	/* SDC Ver2+  */
			for (n = 0; n < 4; n++) ocr[n] = spi_slow_wr(0xff);
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {	/* The card can work at vdd range of 2.7-3.6V */
				do {
					if (send_slow_cmd(CMD55, 0) <= 1 && send_slow_cmd(CMD41, 1UL << 30) == 0) break; /* ACMD41 with HCS bit */
					++i;
				} while (i < 300000);
				if (i < 300000 && send_slow_cmd(CMD58, 0) == 0) {	/* Check CCS bit */
					for (n = 0; n < 4; n++) ocr[n] = spi_slow_wr(0xff);
					ty = (ocr[0] & 0x40) ? 6 : 2;
				}
			}
		} else {							/* SDC Ver1 or MMC */
			ty = (send_slow_cmd(CMD55, 0) <= 1 && send_slow_cmd(CMD41, 0) <= 1) ? 2 : 1;	/* SDC : MMC */
			do {
				if (ty == 2) {
					if (send_slow_cmd(CMD55, 0) <= 1 && send_slow_cmd(CMD41, 0) == 0) break;	/* ACMD41 */
				} else {
					if (send_slow_cmd(CMD1, 0) == 0) break;								/* CMD1 */
				}
				++i;
			} while (i < 300000);
			if (!(i < 300000) || send_slow_cmd(CMD16, 512) != 0)	/* Select R/W block length */
				ty = 0;
		}
	}
	
	send_slow_cmd(CMD59, 1);		// crc check
	
	CardType = ty;
	DESELECT();			/* CS = H */
	(void)spi_slow_wr(0xff);			/* Idle (Release DO) */

	if (ty) {			/* Initialization succeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
	} else {			/* Initialization failed */
		power_off();
        
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	uchar drv			/* Drive number (0) */
)
{
	return (drv) ? STA_NODISK : Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	uchar drv,			/* Physical drive nmuber (0) */
	uchar *buff,			/* Pointer to the data buffer to store read data */
	ulong sector,		/* Start sector number (LBA) */
	uchar count			/* Sector count (1..255) */
)
{
	uchar cnt, *p;
	int retry;

	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) 
	{
		return RES_NOTRDY;
	}

	if (!(CardType & 4)) sector *= 512;	/* Convert to byte address if needed */


	for (retry = 0; retry < MAX_RETRY; retry++) {
		p = buff;
		cnt = count;
		
		SELECT();			/* CS = L */

		if (cnt == 1) {	/* Single block read */
			if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
				&& rcvr_datablock(p, 512))
			{
				cnt = 0;
			}
		}
		else {				/* Multiple block read */
			if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
				do {
					if (!rcvr_datablock(p, 512)) break;
					p += 512;
				} while (--cnt);
				send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
			}
		}

		DESELECT();			/* CS = H */
		(void)spi_r();			/* Idle (Release DO) */
		if (cnt == 0) break;
	}

//printf("retry = %d (MAX=%d)\n",retry,MAX_RETRY);

	return((retry < MAX_RETRY && cnt == 0) ? RES_OK : RES_ERROR);
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	uchar drv,			/* Physical drive nmuber (0) */
	const uchar *buff,	/* Pointer to the data to be written */
	ulong sector,		/* Start sector number (LBA) */
	uchar count			/* Sector count (1..255) */
)
{
	uchar cnt, *p;
	int retry;
	
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & 4)) sector *= 512;	/* Convert to byte address if needed */

	for (retry = 0; retry < MAX_RETRY; retry++) {
		p = (uchar *)buff;
		cnt = count;
		
		SELECT();			/* CS = L */

		if (count == 1) {	/* Single block write */
			if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
				&& xmit_datablock(p, 0xFE))
				cnt = 0;
		}
		else {				/* Multiple block write */
			if (CardType & 2) {
				send_cmd(CMD55, 0); send_cmd(CMD23, cnt);	/* ACMD23 */
			}
			if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
				do {
					if (!xmit_datablock(p, 0xFC)) break;
					p += 512;
				} while (--cnt);
				if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
					cnt = 1;
			}
		}

		DESELECT();			/* CS = H */
		(void)spi_r();			/* Idle (Release DO) */
		if (cnt == 0) break;
	}
	return((retry < MAX_RETRY && cnt == 0) ? RES_OK : RES_ERROR);
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	uchar drv,		/* Physical drive nmuber (0) */
	uchar ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	uchar n, csd[16];
	ushort csize;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	SELECT();		/* CS = L */

	res = RES_ERROR;
	switch (ctrl) {
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (ulong) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((ushort)csd[8] << 8) + 1;
					*(ulong*)buff = (ulong)csize << 10;
				} else {					/* MMC or SDC ver 1.XX */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((ushort)csd[7] << 2) + ((ushort)(csd[6] & 3) << 10) + 1;
					*(ulong*)buff = (ulong)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (ushort) */
			*(ushort*)buff = 512;
			res = RES_OK;
			break;

		case CTRL_SYNC :	/* Make sure that data has been written */
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16/2))
				res = RES_OK;
			break;

		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16/2))
				res = RES_OK;
			break;

		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 0; n < 4; n++)
					*((uchar*)buff+n) = spi_r();
				res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
	}

	DESELECT();			/* CS = H */
	(void)spi_r();			/* Idle (Release DO) */
	return res;
}

#if 0
/*-----------------------------------------------------------------------*/
/* Device timer interrupt procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void disk_timerproc (void)
{
//	static uchar pv;
	uchar n, s;


//	n = Timer1;						/* 100Hz decrement timer */
//	if (n) Timer1 = --n;
//	n = Timer2;
//	if (n) Timer2 = --n;


//	n = pv;
//	pv = SOCKPORT & (SOCKWP | SOCKINS);	/* Sapmle socket switch */

//	if (n == pv) {					/* Have contacts stabled? */
		s = Stat;

//		if (pv & SOCKWP)			/* WP is H (write protected) */
//			s |= STA_PROTECT;
//		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

//		if (pv & SOCKINS)			/* INS = H (Socket empty) */
//			s |= (STA_NODISK | STA_NOINIT);
//		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
//	}
}
#endif


