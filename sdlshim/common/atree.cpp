
// the atree is a quicksearch-like algorithm optimized for
// mapping one void * value to another.
#include <stdlib.h>
#include <memory.h>
#include "basics.h"
#include "atree.h"
#include "atree.fdh"

#ifdef __x86_64__
	#define _64BIT
	#define NODE_LEVELS	8
#else
	#define _32BIT
	#define NODE_LEVELS	4
#endif

#ifdef _32BIT
	#define SPLIT_ADDRESS(addr, n1, n2, n3, n4)	\
	{	\
		n1 = (uchar)((addr) >> 24); \
		n2 = (uchar)((addr) >> 16); \
		n3 = (uchar)((addr) >> 8); \
		n4 = (uchar)((addr));	\
	}
#else
	#define SPLIT_ADDRESS(addr, n1, n2, n3, n4, n5, n6, n7, n8)	\
	{	\
		n1 = (uchar)((addr) >> 56);	\
		n2 = (uchar)((addr) >> 48);	\
		n3 = (uchar)((addr) >> 40);	\
		n4 = (uchar)((addr) >> 32);	\
		n5 = (uchar)((addr) >> 24);	\
		n6 = (uchar)((addr) >> 16);	\
		n7 = (uchar)((addr) >> 8);	\
		n8 = (uchar)((addr));	\
	}
#endif

ATree::ATree()
{
	BaseNode = NewATNode();
}

ATree::~ATree()
{
	RecursiveFree(BaseNode, 0);
}

static void RecursiveFree(ATNode *tree, uchar rlevel)
{
int i;
	
	if (rlevel < (NODE_LEVELS-1) && tree)
	{
		for(i=0;i<256;i++)
		{
			if (tree->nodes[i])
			{
				RecursiveFree(tree->nodes[i], rlevel+1);
			}
		}
	}
	
	free(tree);
}


/*
void c------------------------------() {}
*/

static ATNode *NewATNode()
{
	ATNode *node = (ATNode *)malloc(sizeof(ATNode));
	memset(node, 0, sizeof(ATNode));
	
	return node;
}

static inline ATNode *follow_tree(ATNode **node)
{
	if (!*node)
	{
		ATNode *newnode = NewATNode();
		*node = newnode;
	}
	
	return *node;
}

void ATree::AddMapping(void *map_from, void *map_to)
{
ATNode *node = BaseNode;

#ifdef _32BIT
uchar n1, n2, n3, n4;
	SPLIT_ADDRESS((uint)map_from, n1, n2, n3, n4);
	
	node = follow_tree(&node->nodes[n1]);
	node = follow_tree(&node->nodes[n2]);
	node = follow_tree(&node->nodes[n3]);
	
	node->answers[n4] = (void *)map_to;
#else
uchar n[8];
	SPLIT_ADDRESS((long)map_from, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);
	
	node = follow_tree(&node->nodes[n[0]]);
	node = follow_tree(&node->nodes[n[1]]);
	node = follow_tree(&node->nodes[n[2]]);
	node = follow_tree(&node->nodes[n[3]]);
	node = follow_tree(&node->nodes[n[4]]);
	node = follow_tree(&node->nodes[n[5]]);
	node = follow_tree(&node->nodes[n[6]]);
	
	node->answers[n[7]] = (void *)map_to;
#endif
}

void *ATree::Lookup(void *map_from)
{
ATNode *node = BaseNode;

#ifdef _32BIT
uchar n1, n2, n3, n4;
	SPLIT_ADDRESS((uint)map_from, n1, n2, n3, n4);
	
	node = node->nodes[n1]; if (!node) return NULL;
	node = node->nodes[n2]; if (!node) return NULL;
	node = node->nodes[n3]; if (!node) return NULL;
	return node->answers[n4];
#else
uchar n1, n2, n3, n4, n5, n6, n7, n8;
	SPLIT_ADDRESS((long)map_from, n1, n2, n3, n4, n5, n6, n7, n8);
	
	node = node->nodes[n1]; if (!node) return NULL;
	node = node->nodes[n2]; if (!node) return NULL;
	node = node->nodes[n3]; if (!node) return NULL;
	node = node->nodes[n4]; if (!node) return NULL;
	node = node->nodes[n5]; if (!node) return NULL;
	node = node->nodes[n6]; if (!node) return NULL;
	node = node->nodes[n7]; if (!node) return NULL;
	return node->answers[n8];
#endif
}

void ATree::Delete(void *object_to_delete)
{
	AddMapping(object_to_delete, NULL);
}

/*
void c------------------------------() {}
*/

void ATree::DoForEach(ATreeCallback func, void *userdata)
{
	DoForEachInternal(this->BaseNode, 0, 0, func, userdata);
}

static void DoForEachInternal(ATNode *node, long addy, int level, ATreeCallback func, void *userdata)
{
long i;
	if (!node) return;
	
	if (level < (NODE_LEVELS-1))
	{
		for(i=0;i<256;i++)
		{
			if (node->nodes[i])
			{
				long subaddy = addy;
				switch(level)
				{
					case 0: subaddy |= i; break;
					case 1: subaddy |= (i<<8); break;
					case 2: subaddy |= (i<<16); break;
					#ifdef _64BIT
					case 3: subaddy |= (i<<24); break;
					case 4: subaddy |= (i<<32); break;
					case 5: subaddy |= (i<<40); break;
					case 6: subaddy |= (i<<48); break;
					case 7: subaddy |= (i<<56); break;
					#endif
				}
				DoForEachInternal(node->nodes[i], subaddy, level+1, func, userdata);
			}
		}
	}
	else
	{
		for(i=0;i<256;i++)
		{
			if (node->answers[i])
			{
				#ifdef _64BIT
					#define TOPSHIFT	56
				#else
					#define TOPSHIFT	24
				#endif
				int subaddy = addy | (i << TOPSHIFT);
				(*func)((void *)subaddy, node->answers[i], userdata);
			}
		}
	}
}
