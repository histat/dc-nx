// $$$ : sci.h --
#ifndef __SCI_H__
#define __SCI_H__

void sci_init(void);
void tx_bit(int sw);
unsigned char rx_bit(void);
void cts_bit(int sw);
void rts_bit(int sw);
unsigned char spi_slow_wr(register unsigned char);
unsigned char spi_wr(unsigned char);
void spi_w(register unsigned char);
unsigned char spi_r(void);
void spi_block_w(unsigned char *p);
void spi_block_r(unsigned char *p, int len);
void delay_m(int n);

#endif
// end of : sci.h

