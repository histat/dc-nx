
#ifndef _QUEUE_H
#define _QUEUE_H

typedef void (*queue_freer_func)(void *item);


class Queue
{
public:
	Queue(queue_freer_func func);
	~Queue();
	
	void EnqueueItem(void *item);
	void *GetNextItem();
	int GetItemCount();

// ---------------------------------------

	BList fQueue;
	BLocker fLock;
	
	queue_freer_func fFreeItemFunc;

};


#endif
