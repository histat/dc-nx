
#ifndef _QS_H
#define _QS_H

class QSTree
{
public:
	QSTree();
	~QSTree();
	
	void AddMapping(const char *str, void *answer);
	void AddMapping(const char *str, int32_t answer);
	
	void *Lookup(const char *str);
	int32_t LookupInt(const char *str);
	
	void Delete(const char *str);
	void MakeEmpty();

private:
	QSTree *branch[256];
	void *answer;
	bool has_answer;
};

#endif
