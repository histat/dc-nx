#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL  0

struct sys_sem;
typedef struct sys_sem * sys_sem_t;

struct sys_mbox;
typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;

#define YIELD_MODE_STOP   0
#define YIELD_MODE_RUN    1
#define YIELD_MODE_DIE    2

extern int sys_thread_yield(int mode);


#endif /* __ARCH_SYS_ARCH_H__ */

