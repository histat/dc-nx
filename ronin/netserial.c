#include "lwip/api.h"
#include <string.h>

static char putc_buf[1024], oob_buf[1024];
static int putc_pos = 0, oob_pos = 0;
static struct netconn *ser_conn = NULL;
static struct netbuf *buf = NULL;

static void low_send_putc(char *ptr, int l)
{
  if(ser_conn && buf && netconn_connect(ser_conn, IP_ADDR_BROADCAST, 1445)==ERR_OK) {
    netbuf_ref(buf, ptr, l);
    netconn_send(ser_conn, buf);
  }
}

static void low_send_oob(char *ptr, int l)
{
  if(ser_conn && buf && netconn_connect(ser_conn, IP_ADDR_BROADCAST, 1447)==ERR_OK) {
    netbuf_ref(buf, ptr, l);
    netconn_send(ser_conn, buf);
  }
}

void serial_init(int baudrate)
{
  lwip_init();
  ser_conn = netconn_new(NETCONN_UDP);
  netconn_bind(ser_conn, IP_ADDR_ANY, 1441);
  buf = netbuf_new();
  putc_pos = oob_pos = 0;
}

void serial_puts(const char *message)
{
  int l = strlen(message);
  if(putc_pos && l+putc_pos > sizeof(putc_buf)) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
  if(l>sizeof(putc_buf))
    low_send_putc((char *)message, l);
  else {
    memcpy(putc_buf+putc_pos, message, l);
    putc_pos += l;
    if(putc_pos >= sizeof(putc_buf)) {
      low_send_putc(putc_buf, putc_pos);
      putc_pos = 0;
    }
  }
}

void oob_send_data(const char *data, int l)
{
  if(oob_pos && l+oob_pos > sizeof(oob_buf)) {
    low_send_oob(oob_buf, oob_pos);
    oob_pos = 0;
  }
  if(l>sizeof(oob_buf))
    low_send_oob((char *)data, l);
  else {
    memcpy(oob_buf+oob_pos, data, l);
    oob_pos += l;
    if(oob_pos >= sizeof(oob_buf)) {
      low_send_oob(oob_buf, oob_pos);
      oob_pos = 0;
    }
  }
}

void serial_putc(int c)
{
  putc_buf[putc_pos++] = c;
  if(putc_pos >= sizeof(putc_buf)) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
}

void serial_flush()
{
  if(putc_pos) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
}

void oob_flush()
{
  if(oob_pos) {
    low_send_oob(oob_buf, oob_pos);
    oob_pos = 0;
  }
}
