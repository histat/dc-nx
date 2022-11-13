
#include "shim.h"
#include <string.h>
#include <zlib.h>

#include <dc/maple.h>
#include <dc/maple/vmu.h>
#include <dc/vmu_pkg.h>
#include <dc/vmufs.h>
#include "cave_icon.h"
#include "icon_data_2bpp.h"

#define MAX_VMU_SIZE 128 * 1024

uint8 lcd_icon[48*32 /8];

int vm_file;

static void conv_lcd_icon(unsigned char *bit, const unsigned char *in)
{
  unsigned short *src = (unsigned short *)in;
  unsigned char *dst = bit + (48/8) * 32;

  for (int i=0; i<16; i++) {
    unsigned char v;
    unsigned int b = *src++;

    *--dst = 0xff;
    *--dst = 0xff;
    for (int j= 0; j<16/8; j++) {
      v = 0;
      for (int x=0; x<8; x++) {
	      v <<= 1;
	      v |= (b & 1)?1:0;
	      b >>= 1;
      }
      *--dst = v;
    }
    *--dst = 0xff;
    *--dst = 0xff;
  }
}

void init_lcd()
{
    conv_lcd_icon(lcd_icon, icon_data_2bpp);

    /*
     maple_device_t * dev;
     int i=0;

     while((dev = maple_enum_type(i++, MAPLE_FUNC_LCD))) {
       vmu_draw_lcd(dev, lcd_icon);
     }
    */
}

static void clearlcd()
{
}

bool vmfile_search(const char *fname, int *vm)
{
  maple_device_t *dev;
  vmu_pkg_t   pkg;
  uint8 *pkg_buf;
  int pkg_size;


  for (int i=0; i<8; i++) {
    dev = maple_enum_type(i, MAPLE_FUNC_MEMCARD);

    if (!dev) {
      continue;
    }

    if (vmufs_read(dev, fname, (void **)&pkg_buf, &pkg_size) < 0) {
      continue;
    }

    vmu_pkg_parse(pkg_buf, &pkg);
    if(strcmp(pkg.app_id, "NXEngine")) {
      free(pkg_buf);
      continue;
    }

    free(pkg_buf);

    *vm = i;
    return true;
  }

  *vm = -1;

    return false;
}

bool vmfile_exists(const char *fname)
{
    return vmfile_search(fname, &vm_file);
}

bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len)
{
    maple_device_t *dev;
    int free_bytes;
    vmu_pkg_t   pkg;
    uint8 *pkg_buf;
    int bufSize;
    int ret;


    free_bytes = 0;

    dev = maple_enum_type(unit, MAPLE_FUNC_MEMCARD);

    if (dev) {
      free_bytes = vmufs_free_blocks(dev);
    }

    if (free_bytes < ((128+512+buf_len+511)/512)) {
      return false;
    }

    if (dev->info.functions & MAPLE_FUNC_LCD) {
      vmu_draw_lcd(dev, lcd_icon);
    }

    memset(&pkg, 0, sizeof(struct vmu_pkg));
    strncpy(pkg.desc_short, "NXEngine SAVE", 16);
    strncpy(pkg.desc_long, "Save Data", 32);
    strncpy(pkg.app_id, "NXEngine", 16);
    pkg.icon_cnt = 1;
    pkg.icon_anim_speed = 0;
    pkg.eyecatch_type = VMUPKG_EC_NONE;
    memcpy(&pkg.icon_pal, &cave_icon, 32);
    pkg.icon_data =  cave_icon + 32;
    pkg.eyecatch_data = NULL;
    
    pkg.data = (const uint8*)buf;

    pkg.data_len = buf_len;

    ret = vmu_pkg_build(&pkg, &pkg_buf, &bufSize);

    if (ret < 0) {
      return false;
    }

    vmufs_write(dev, filename, pkg_buf, bufSize, VMUFS_OVERWRITE);

    free(pkg_buf);

    clearlcd();
    vmu_draw_lcd(dev, lcd_icon);

    return true;
}

bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len)
{
    maple_device_t *dev;
    vmu_pkg_t pkg;
    char *data;
    int len;

    uint8 *pkg_buf;
    int pkg_size;

    dev = maple_enum_type(unit, MAPLE_FUNC_MEMCARD);

    if (!dev) {
      return false;
    }

    if (dev->info.functions & MAPLE_FUNC_LCD) {
      vmu_draw_lcd(dev, lcd_icon);
    }

    if (vmufs_read(dev, filename, (void **)&pkg_buf, &pkg_size) < 0) {
      return false;
    }

    vmu_pkg_parse(pkg_buf, &pkg);

    data = (char *) pkg.data;
    len = pkg.data_len;
    memcpy(buf, data, len);
    *buf_len = len;

    free(pkg_buf);

    return true;
}


bool delete_file_vmu(int unit, const char *filename)
{
#ifndef NOSERIAL
    printf("delete %s on %d", filename,unit);
#endif
	
    return true;
}

bool rename_vmu_file(const char *oldpath, const char *newpath)
{
	return false;
}

int remove(const char *fname)
{
	return -1;
}
