#include <ronin.h>
#include "report.h"

struct sysinfo {
  unsigned char id[8];
  unsigned char settings[16];
};

int syscall_read_flash(int offs, void *ptr, int cnt)
{
  return (*(int (**)(int, void *, int, int))0x8c0000b8)(offs,ptr,cnt,1);  
}

void get_sysinfo(struct sysinfo *si)
{
  int i;

  syscall_read_flash(0x1a056, si->id, 8);
  syscall_read_flash(0x1a000, si->settings, 5);
  for(i=0; i<11; i++)
    si->settings[5+i] = 0;
}

void query_tv()
{
  //PALM (Brazil) is 60Hz. The sweep frequency is closer to NTSC than
  //PAL and it only has 525 lines.
  //
  //PALM (Uruguay, Paraguay and Argentina) has Lower bandwith than
  //PAL, but otherwise pretty much the same.
  //
  //http://www.alkenmrs.com/video/standards.html for details

  struct sysinfo si;
  int system;

  report("TV system... ");  
  //get_sysinfo(&si);
  //system = si.settings[4]-'0';
  pvr_check_tvsys(fb_devconfig);
  system = fb_devconfig->dc_tvsystem;

  switch(system) {
   case 0: // NTSC
     break;
   case 1: // PAL
     Settings.TVStandard = 1;
     Settings.Hz50 = 1;
     break;
   case 2: // PAL M
     Settings.TVStandard = 2;
     break;
   case 3: // PAL N
     Settings.TVStandard = 3;
     Settings.Hz50 = 1;
     break;
   default:
     report("Unknown! Defaulting to ");
  }

  report(Settings.TVStandard?"PAL/":"NTSC/");
  report(Settings.Hz50?"50\n":"60\n");
}
