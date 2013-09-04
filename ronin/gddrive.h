#ifndef _RONIN_GDDRIVE_H
#define _RONIN_GDDRIVE_H been_here_before

#include "common.h"

START_EXTERN_C
int gdGdcReqCmd(int command, unsigned int *parameterblock);
int gdGdcGetCmdStat(int reqid, unsigned int *status);
void gdGdcExecServer();
void gdGdcInitSystem();
int gdGdcGetDrvStat(unsigned int *status);
/* FIXME: Not documented
gdGdcG1DmaEnd
	bra	do_syscall
	mov	#5,r7
gdGdcReqDmaTrans:
	bra	do_syscall
	mov	#6,r7
gdGdcCheckDmaTrans:
	bra	do_syscall
	mov	#7,r7
*/
int gdGdcReadAbort(int reqid);
void gdGdcReset();
int gdGdcChangeDataType(unsigned int *format);
END_EXTERN_C

#endif /* _RONIN_GDDRIVE_H */
