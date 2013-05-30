
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basics.h"
#include "quicksearch.h"

QSTree::QSTree()
{
	memset(branch, 0, sizeof(branch));
	answer = NULL;
}

QSTree::~QSTree()
{
	MakeEmpty();
}

/*
void c------------------------------() {}
*/

// add a string to the node tree.
void QSTree::AddMapping(const char *str, void *answer)
{
QSTree *node = this;
uchar ch;
int i;

	//stat("QSAddString '%s' to basenode %08x", str, basenode);
	
	for(i=0;str[i];i++)
	{
		ch = (uchar)str[i];
		
		if (!node->branch[ch])
			node->branch[ch] = new QSTree;
		
		node = node->branch[ch];
	}
	
	node->answer = answer;
	node->has_answer = true;
}

void QSTree::AddMapping(const char *str, int32_t answer)
{
	AddMapping(str, (void *)answer);
}

/*
void c------------------------------() {}
*/

// looks up the string you give it, and returns the answer, or NULL if it doesn't exist
void *QSTree::Lookup(const char *str)
{
QSTree *node = this;
int i;

	for(i=0;str[i];i++)
	{
		node = node->branch[(uchar)str[i]];
		if (!node)
			return NULL;
	}
	
	return node->has_answer ? node->answer : NULL;
}

// looks up the string you give it, and returns the answer, or -1 if it doesn't exist
int32_t QSTree::LookupInt(const char *str)
{
QSTree *node = this;
int i;

	for(i=0;str[i];i++)
	{
		node = node->branch[(uchar)str[i]];
		if (!node)
			return -1;
	}
	
	return node->has_answer ? (int32_t)(size_t)node->answer : -1;
}

/*
void c------------------------------() {}
*/

void QSTree::Delete(const char *str)
{
QSTree *node = this;
int i;

	for(i=0;str[i];i++)
	{
		node = node->branch[(uchar)str[i]];
		if (!node)
			return;
	}
	
	node->has_answer = false;
}

void QSTree::MakeEmpty()
{
int i;

	for(i=0;i<256;i++)
	{
		if (branch[i])
		{
			delete branch[i];
			branch[i] = NULL;
		}
	}
}
