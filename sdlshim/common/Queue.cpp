
#include <Locker.h>
#include <List.h>
#include "Queue.h"

Queue::Queue(queue_freer_func func)
	: fFreeItemFunc(func)
{

}

Queue::~Queue()
{
	fLock.Lock();
	
	if (fFreeItemFunc)
	{
		for(int i=0;
			void *item = fQueue.ItemAt(i);
			i++)
		{
			(*fFreeItemFunc)(item);
		}
	}
	
	fQueue.MakeEmpty();
}

/*
void c------------------------------() {}
*/

void Queue::EnqueueItem(void *item)
{
	fLock.Lock();
	fQueue.AddItem(item);
	fLock.Unlock();
}

void *Queue::GetNextItem()
{
	fLock.Lock();
	void *item = fQueue.RemoveItem((int32)0);
	fLock.Unlock();
	
	return item;
}

int Queue::GetItemCount()
{
int count;

	fLock.Lock();
	count = fQueue.CountItems();
	fLock.Unlock();
	
	return count;
}


