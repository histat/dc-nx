#include "shim.h"
#include "scrnsave.h"
#include <kos.h>
#include "private.h"
#include <../hardware/pvr/pvr_internal.h>
#include "audio.h"
#include "watchdog.h"

extern void init_lcd();
extern void handleInput();

uint16_t *vram = NULL;
void *screen_tx[SCREEN_BUFFER_SIZE] = {NULL};

static unsigned char dc_screen[VRAM_SIZE] __attribute__((aligned (32)));

extern "C" {

int  arch_auto_init() {
    /* Initialize memory management */
    mm_init();

    /* Do this immediately so we can receive exceptions for init code
       and use ints for dbgio receive. */
    irq_init();         /* IRQs */
    irq_disable();          /* Turn on exceptions */

    fs_dcload_init_console();   /* Init dc-load console, if applicable */

    /* Init SCIF for debug stuff (maybe) */
    scif_init();

    /* Init debug IO */
    dbgio_init();

    timer_init();           /* Timers */
    hardware_sys_init();        /* DC low-level hardware init */

    /* Initialize our timer */
    timer_ms_enable();
    rtc_init();

    /* Threads */
    if(__kos_init_flags & INIT_THD_PREEMPT)
        thd_init(THD_MODE_PREEMPT);
    else
        thd_init(THD_MODE_COOP);

    nmmgr_init();

    fs_init();          /* VFS */

    hardware_periph_init();     /* DC peripheral init */

    if(*DCLOADMAGICADDR == DCLOADMAGICVALUE) {
#ifndef NOSERIAL
        dbglog(DBG_INFO, "dc-load console support enabled\n");
#endif
        //fs_dcload_init();
    }

    fs_iso9660_init();

    vmufs_init();

    /* Now comes the optional stuff */
    irq_enable();       /* Turn on IRQs */
    maple_wait_scan();  /* Wait for the maple scan to complete */

    return 0;
}

void  arch_auto_shutdown() {
    fs_dclsocket_shutdown();

    irq_disable();
    timer_shutdown();
    hardware_shutdown();
    pvr_shutdown();
    //library_shutdown();
    //fs_dcload_shutdown();
    vmufs_shutdown();
    fs_iso9660_shutdown();
    fs_shutdown();
    thd_shutdown();
    rtc_shutdown();
}
}

//KOS_INIT_FLAGS(INIT_DEFAULT|INIT_QUIET);
int init_hardware()
{
  pvr_init_params_t pvr_params = {
    { PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },
    (int)(256 * 1024),
    0,	//dma
    0,	//fsaa
    0	//autosort disable
  };
  pvr_init (&pvr_params);

#ifndef NOSERIAL
  wdInit();
#endif

  _audio_init();

  pvr_ptr_t pvr_mem_base = (pvr_ptr_t)(PVR_RAM_INT_BASE + pvr_state.texture_base);

#ifndef NOSERIAL
  printf("reset pvr_mem_base at 0x%4x ", pvr_mem_base);
#endif

  memset(dc_screen, 0, sizeof(dc_screen));
  
  vram = (uint16_t *)dc_screen;
  
  *(volatile unsigned int*)(0xa05f80e4) = SCREEN_WIDTH >> 5; //for stride
  
  for (int i=0; i<SCREEN_BUFFER_SIZE; i++) {
    screen_tx[i] = (void *)pvr_mem_base;
    pvr_mem_base += VRAM_SIZE;
  }


  audio_init();

  init_lcd();

#ifndef NOSERIAL
  wdPause();
#endif

  return 0;
}

void close_hardware()
{
  audio_free();

  arch_set_exit_path(ARCH_EXIT_MENU);
}
