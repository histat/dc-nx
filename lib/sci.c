// $$$ : sci.c -- Dreamcast Serial <=> SPI interface by jj1odm
//
// DC SCI          SD Card
// -----------------------
// RTS (SPI CS)   --> /CS
// CTS (SPI CLK)  --> CLK
// TX  (SPI DOUT) --> DIN
// RX  (SPI DIN)  <-- DOUT

#include "sci.h"

typedef unsigned char uchar;
typedef unsigned short ushort;

#define MSB     0x80

//#define	SCSMR2	0xffe80000
//#define	SCSPTR2	(SCSMR2+32)
#define	SCSMR2	*((volatile unsigned short *)0xffe80000)
#define	SCSCR2	*((volatile unsigned short *)0xffe80008)
#define	SCFCR2	*((volatile unsigned short *)0xffe80018)
#define	SCSPTR2	*((volatile unsigned short *)0xffe80020)
#define	SCFSR2	*((volatile unsigned short *)0xffe80010)
#define	SCLSR2	*((volatile unsigned short *)0xffe80024)


// SCSPTR2 register bit assign
#define	SPB2DT	0x0001		// TX data bit
#define	SPB2IO	0x0002		// TX I/O bit
#define	CTSDT	0x0010		// CTS data bit
#define	CTSIO	0x0020		// CTS I/O bit
#define	RTSDT	0x0040		// RTS data bit
#define	RTSIO	0x0080		// RTS I/O bit

static unsigned short scsptr2;

void sci_init(void)
{
  scsptr2 = SPB2IO | CTSIO | RTSIO | SPB2DT | RTSDT;	// TX:1 CTS:0 RTS:1 (spiout:1 spiclk:0 spics:1)

  SCSCR2 = 0;
  SCSMR2 = 0;
  SCFCR2 = 0x06;
  SCFCR2 = 0;
  SCSPTR2 = scsptr2;
  SCFSR2 = 0;
  SCLSR2 = 0;
  SCSCR2 = 0;
}

void tx_bit(int sw)
{
  scsptr2 = sw ? (scsptr2 | SPB2DT) : (scsptr2 & ~SPB2DT);
  SCSPTR2 = scsptr2;
}

unsigned char rx_bit(void)
{
	return (SCSPTR2 & 0x0001);
}

void cts_bit(int sw)	// spi clock
{
  scsptr2 = sw ? (scsptr2 | CTSDT) : (scsptr2 & ~CTSDT);
  SCSPTR2 = scsptr2;
}

void rts_bit(int sw)	// spi cs
{
  scsptr2 = sw ? (scsptr2 | RTSDT) : (scsptr2 & ~RTSDT);
  SCSPTR2 = scsptr2;
}

static void custom_delay(void)
{
  int i;
    
  for (i = 0; i < 5; i++)
    ;
}

#define TX_BIT	{_pwork = data & MSB ? (_pwork | SPB2DT) : (_pwork & ~SPB2DT); SCSPTR2 = _pwork;};
#define CTS_ON	{SCSPTR2 = _pwork | CTSDT; custom_delay();}
#define CTS_OFF	{SCSPTR2 = _pwork & ~CTSDT;}

static void delay1u5(void)	// 1.5uSEC delay
{
  int i;
    
  for (i = 0; i < 100; i++)
    ;
}

static void delay1u(void)	// 1uSEC delay
{
  int i;
    
  for (i = 0; i < 66; i++)
    ;
}

void delay_m(int n)
{
    int i;
    
    while (n--) {
        for (i = 1000; i--;) delay1u();
    }
}

unsigned char spi_slow_wr(register unsigned char data)
{
  register int cnt;
    
  for (cnt = 0; cnt < 8; cnt++) {
    tx_bit(data & MSB);
    delay1u5();
    cts_bit(1);		/* SPI clock ON */
    data = (data << 1) | rx_bit();	/* SPI data input */
    delay1u5();
    cts_bit(0);		/* SPI clock OFF */
  }
  return(data);
}

unsigned char spi_wr(register unsigned char data)
{
  register unsigned short _pwork;
  
  _pwork = scsptr2;
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  return(data);
}

void spi_w(register unsigned char data)
{
  register unsigned short _pwork;
  
  _pwork = scsptr2;
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
   CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1);
  CTS_OFF;		/* SPI clock OFF */
}

unsigned char spi_r(void)
{
  register unsigned char data;
  register unsigned short _pwork;
  
  _pwork = scsptr2;
  data = 0xff;
  TX_BIT;
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  CTS_ON;		/* SPI clock ON */
  data = (data << 1) | (SCSPTR2 & 0x0001);	/* SPI data input */
  CTS_OFF;		/* SPI clock OFF */
  return(data);
}

// end of : sci.c

