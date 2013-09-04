#ifndef _MAPLE_H
#define _MAPLE_H been_here_before

/* Maple Bus command and response codes */

#define MAPLE_RESPONSE_FILEERR -5
#define MAPLE_RESPONSE_AGAIN   -4  /* request should be retransmitted */
#define MAPLE_RESPONSE_BADCMD  -3
#define MAPLE_RESPONSE_BADFUNC -2
#define MAPLE_RESPONSE_NONE    -1  /* unit didn't respond at all */
#define MAPLE_COMMAND_DEVINFO  1
#define MAPLE_COMMAND_ALLINFO  2
#define MAPLE_COMMAND_RESET    3
#define MAPLE_COMMAND_KILL     4
#define MAPLE_RESPONSE_DEVINFO 5
#define MAPLE_RESPONSE_ALLINFO 6
#define MAPLE_RESPONSE_OK      7
#define MAPLE_RESPONSE_DATATRF 8
#define MAPLE_COMMAND_GETCOND  9
#define MAPLE_COMMAND_GETMINFO 10
#define MAPLE_COMMAND_BREAD    11
#define MAPLE_COMMAND_BWRITE   12
#define MAPLE_COMMAND_SETCOND  14


/* Function codes */

#define MAPLE_FUNC_CONTROLLER 0x001
#define MAPLE_FUNC_MEMCARD    0x002
#define MAPLE_FUNC_LCD        0x004
#define MAPLE_FUNC_CLOCK      0x008
#define MAPLE_FUNC_MICROPHONE 0x010
#define MAPLE_FUNC_ARGUN      0x020
#define MAPLE_FUNC_KEYBOARD   0x040
#define MAPLE_FUNC_LIGHTGUN   0x080
#define MAPLE_FUNC_PURUPURU   0x100
#define MAPLE_FUNC_MOUSE      0x200


struct maple_devinfo {
  unsigned int func;              /* big endian! */
  unsigned int function_data[3];  /* big endian! */
  unsigned char area_code;
  unsigned char connector_direction;
  char product_name[30];
  char product_license[60];
  unsigned short standby_power;   /* little endian */
  unsigned short max_power;       /* little endian */
};

struct mapledev
{
  unsigned int func, xfunc, xcond;
  char ttl, present, dummy1, dummy2;
  union {
    struct {
      unsigned short buttons;
      unsigned char rtrigger;
      unsigned char ltrigger;
      unsigned char joyx;
      unsigned char joyy;
      unsigned char joyx2;
      unsigned char joyy2;
    } controller;
    struct {
      unsigned char shift;
      unsigned char led;
      unsigned char key[6];
    } kbd;
    struct {
      unsigned int buttons;
      short axis1;
      short axis2;
      short axis3;
      short axis4;
      short axis5;
      short axis6;
      short axis7;
      short axis8;
    } mouse;
    unsigned int data[5];
  } cond;
};

struct maple_minfo {
  unsigned int dunno1;
  unsigned short root_loc;
  unsigned short fat_loc;
  unsigned short fat_size;
  unsigned short dir_loc;
  unsigned short dir_size;
  unsigned short icon_shape;
  unsigned short num_blocks;
  unsigned short dunno2;
  unsigned short dunno3;
  unsigned short dunno4;
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
extern int gun_x, gun_y;
void maple_init(void);
/* void maple_wait_dma(void); */
void *maple_docmd(int port, int unit, int cmd, int datalen, void *data);
void maple_set_gun_mode(int mode);
/* struct mapledev *check_pads(void); */
struct mapledev *locked_get_pads(void);
void maple_vbl_handler(void);
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static __inline int getimask(void)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  return (sr >> 4) & 0x0f;
}

static __inline void setimask(int m)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  sr = (sr & ~0xf0) | (m << 4);
  __asm__("ldc %0,sr" : : "r" (sr));
}

#endif //_MAPLE_H
