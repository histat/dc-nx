
#include <OS.h>
#include "ThreadFlag.h"
#include "ThreadFlag.fdh"

ThreadFlag::ThreadFlag()
{
	fSemaphore = create_sem(0, "TF");
}

ThreadFlag::~ThreadFlag()
{
	if (fSemaphore != -1)
		delete_sem(fSemaphore);
}

/*
void c------------------------------() {}
*/

// "signal" the threadflag.
void ThreadFlag::Raise()
{
	// deleting the sem unblocks all who are waiting on it
	if (!IsRaised())
	{
		fOrgSemaphore = fSemaphore;
		fSemaphore = -1;
		delete_sem(fOrgSemaphore);
	}
	else
	{
		stat(">>>> ThreadFlag::Raise(): semaphore %08x has already been raised!", fOrgSemaphore);
	}
}

/*
void c------------------------------() {}
*/

// block calling thread until the flag is raised
void ThreadFlag::WaitUntilRaised()
{
	if (!IsRaised())
		acquire_sem_etc(fSemaphore, 1, B_DO_NOT_RESCHEDULE, 0);
}

// block calling thread until the flag is raised, or the timeout,
// given in milliseconds, expires.
//
// if returns 0 if it returned because the semaphore was raised,
// or 1 if it returned because the timeout elapsed.
bool ThreadFlag::WaitUntilRaised(bigtime_t timeout_ms)
{
	if (!IsRaised())
	{
		uint32 flags = B_DO_NOT_RESCHEDULE;
		
		if (timeout_ms > 0)
		{
			timeout_ms *= 1000;
			flags |= B_RELATIVE_TIMEOUT;
		}
		
		int result = acquire_sem_etc(fSemaphore, 1, flags, timeout_ms);
		
		if (result == B_TIMED_OUT)
		{
			stat(" >>> semaphore timed out!");
			stat(" >>> timeout_ms = %d", timeout_ms);
			stat(" >>> fSemaphore = %d", fSemaphore);
			return 1;
		}
	}
	
	return 0;
}

// returns whether or not the flag has been signaled
bool ThreadFlag::IsRaised()
{
	return (fSemaphore == -1);
}


/*
void c------------------------------() {}
*/

struct tft
{
	ThreadFlag *Flag1, *Flag2;
	ThreadFlag *DoneFlag;
	int statuscode;
	int32 dontkillme;
};
thread_id my_spawn_thread(thread_func func,
         const char *name,
         int32 priority,
         void *data);

// waits until either flag one or flag two is raised, or the timeout expires.
// return value:
//	TF_ONE: I exited because flag 1 was raised.
//	TF_TWO: I exited because flag 2 was raised.
//	TF_TIMEOUT: I exited because the timeout expired.
int ThreadFlag::WaitTwo(ThreadFlag *flag1, ThreadFlag *flag2, bigtime_t timeout_ms)
{
ThreadFlag DoneFlag;
thread_id one, two;
tft tdata;

	tdata.Flag1 = flag1;
	tdata.Flag2 = flag2;
	tdata.DoneFlag = &DoneFlag;
	tdata.statuscode = -1;
	tdata.dontkillme = 0;
	
	one = spawn_thread(_tfwt_one, "tf_one", B_NORMAL_PRIORITY, &tdata);
	two = spawn_thread(_tfwt_two, "tf_two", B_NORMAL_PRIORITY, &tdata);
	resume_thread(one);
	resume_thread(two);
	
	bool timed_out;
	
	if (timeout_ms)
	{
		timed_out = DoneFlag.WaitUntilRaised(timeout_ms);
	}
	else
	{
		DoneFlag.WaitUntilRaised();
		timed_out = false;
	}
	
	while(tdata.dontkillme != 0)
		snooze(10);
	
	kill_thread(one);
	kill_thread(two);
	
	if (timed_out)
		tdata.statuscode = TF_TIMED_OUT;
	
	return tdata.statuscode;
}

status_t _tfwt_one(void *a)
{
tft *t = (tft *)a;

	t->Flag1->WaitUntilRaised();
	
	atomic_add(&t->dontkillme, 1);
	t->statuscode = TF_ONE;
	t->DoneFlag->Raise();
	
	atomic_add(&t->dontkillme, -1);
	for(;;) { snooze(10000 * 1000); }
}

status_t _tfwt_two(void *a)
{
tft *t = (tft *)a;

	t->Flag2->WaitUntilRaised();
	
	atomic_add(&t->dontkillme, 1);
	t->statuscode = TF_TWO;
	t->DoneFlag->Raise();
	
	atomic_add(&t->dontkillme, -1);
	for(;;) { snooze(10000 * 1000); }
}


