
#include <OS.h>

// for ThreadFlag::WaitTwo
#define TF_ONE			0
#define TF_TIMED_OUT	1
#define TF_TWO			2

class ThreadFlag
{
public:
	ThreadFlag();
	~ThreadFlag();
	
	void Raise();
	
	void WaitUntilRaised();
	bool WaitUntilRaised(bigtime_t timeout_ms);
	bool IsRaised();
	
	static int WaitTwo(ThreadFlag *flag1, ThreadFlag *flag2, bigtime_t timeout_ms);

private:
	sem_id fSemaphore;
	sem_id fOrgSemaphore;
};
