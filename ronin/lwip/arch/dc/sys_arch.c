
#include "lwip/debug.h"
#include "lwip/sys.h"
#include "lwip/def.h"

#include <stdlib.h>
#include "dc_time.h"

#define MAX_MBOX_QUEUE 16

#define THREAD_STACK_SIZE 8192

struct sys_thread {
  void *sp;
  struct sys_thread *next;
  struct sys_timeouts timeouts;
};

struct sys_sem {
  int count;
  struct sys_thread *wait_q_head, *wait_q_tail;
};

struct sys_mbox {
  struct sys_sem write_sem, read_sem;
  void *msgs[MAX_MBOX_QUEUE];
};


static struct sys_thread *current_thread, *last_thread;
static struct sys_thread *thread_run_queue_head, *thread_run_queue_tail;
static struct sys_thread main_thread;

static void sys_thread_kill(struct sys_thread *t)
{
  free(t);
}

int sys_thread_yield(int mode)
{
  extern int sys_thread_switch(struct sys_thread *, struct sys_thread *, int);

  if(thread_run_queue_head) {
    int rc;
    last_thread = current_thread;
    if(mode == YIELD_MODE_RUN) {
      thread_run_queue_tail->next = last_thread;
      thread_run_queue_tail = last_thread;
      current_thread->next = NULL;
    }
    current_thread = thread_run_queue_head;
    thread_run_queue_head = current_thread->next;
    rc = sys_thread_switch(last_thread, current_thread, mode);
    if(rc == YIELD_MODE_DIE)
      sys_thread_kill(last_thread);
    return rc;
  } else if(mode == YIELD_MODE_RUN)
    return 0;
  else
    for(;;);
}

static int sys_thread_activate(struct sys_thread *t, int yieldmode)
{
  if(!(t->next = thread_run_queue_head))
    thread_run_queue_tail = t;
  thread_run_queue_head = t;
  return sys_thread_yield(yieldmode);
}

static void sys_thread_init(void)
{
  register void (* thread)(void *arg) asm ("macl");
  register void *arg asm ("mach");

  thread(arg);
  sys_thread_yield(YIELD_MODE_DIE);
}

void sys_thread_new(void (* thread)(void *arg), void *arg)
{
  struct sys_thread *t = malloc(THREAD_STACK_SIZE);
  ASSERT("sys_thread_new: malloc", t);
  t->sp = ((char *)t)+THREAD_STACK_SIZE-10*4;
  t->timeouts.next = NULL;
  ((void **)t->sp)[0] = sys_thread_init;
  ((void **)t->sp)[1] = thread;
  ((void **)t->sp)[2] = arg;
  sys_thread_activate(t, YIELD_MODE_RUN);
}


struct sys_sem *sys_sem_new(u8_t count)
{
  struct sys_sem *s = malloc(sizeof(struct sys_sem));
  if(s) {
    s->count = count;
    s->wait_q_head = NULL;
  }
  return s;
}

void sys_sem_free(struct sys_sem *s)
{
  ASSERT("sys_sem_free: waiting threads", !s->wait_q_head);
  free(s);
}

void sys_sem_signal(struct sys_sem *s)
{
  struct sys_thread *t;
  if(++s->count >= 0 && (t = s->wait_q_head)) {
    s->wait_q_head = t->next;
    sys_thread_activate(t, YIELD_MODE_RUN);
  }
}

u16_t sys_arch_sem_wait(struct sys_sem *s, u16_t timeout)
{
  if(s->count>0) {
    --s->count;
    return 1;
  }
  if(!timeout) {
    while(s->count<=0) {
      current_thread->next = NULL;
      if(s->wait_q_head)
	s->wait_q_tail->next = current_thread;
      else
	s->wait_q_head = s->wait_q_tail = current_thread;
      sys_thread_yield(YIELD_MODE_STOP);
    }
    --s->count;
    return 1;
  } else {
    unsigned int t=0, t0 = Timer();
    unsigned int dly = USEC_TO_TIMER(timeout*1000);
    while( s->count<=0 && (t = (unsigned int)(Timer()-t0)) < dly)
      sys_thread_yield(YIELD_MODE_RUN);
    if(s->count<=0)
      return 0;
    if((t = (t<<6)/3125)<1) t = 1;
    --s->count;
    return t;
  }
}

struct sys_mbox *sys_mbox_new(void)
{
  struct sys_mbox *m = malloc(sizeof(struct sys_mbox));
  if(m) {
    m->read_sem.count = 0;
    m->read_sem.wait_q_head = NULL;
    m->write_sem.count = MAX_MBOX_QUEUE;
    m->write_sem.wait_q_head = NULL;
  }
  return m;
}

void sys_mbox_free(struct sys_mbox *m)
{
  ASSERT("sys_sem_free: waiting threads",
	 (!m->read_sem.wait_q_head) && (!m->write_sem.wait_q_head));
  free(m);
}

void sys_mbox_post(struct sys_mbox *m, void *msg)
{
  sys_arch_sem_wait(&m->write_sem, 0);
  m->msgs[m->read_sem.count] = msg;
  sys_sem_signal(&m->read_sem);
}

u16_t sys_arch_mbox_fetch(struct sys_mbox *m, void **msg, u16_t timeout)
{
  u16_t res = sys_arch_sem_wait(&m->read_sem, timeout);
  if(res>0) {
    int i;
    if(msg != NULL) *msg = m->msgs[0];
    for(i=0; i<m->read_sem.count; i++)
      m->msgs[i] = m->msgs[i+1];
    sys_sem_signal(&m->write_sem);
  }
  return res;
}

struct sys_timeouts *sys_arch_timeouts(void)
{
  return &current_thread->timeouts;
}

u32_t htonl(u32_t n)
{
  __asm__("swap.b %0,%0\n\tswap.w %0,%0\n\tswap.b %0,%0" : "=r" (n) : "0" (n));
  return n;
}

u16_t htons(u16_t n)
{
  __asm__("swap.b %0,%0" : "=r" (n) : "0" (n));
  return n;
}

void sys_init(void)
{
  main_thread.timeouts.next = NULL;
  thread_run_queue_head = NULL;
  thread_run_queue_tail = NULL;
  current_thread = &main_thread;
  last_thread = NULL;
  return;
}

