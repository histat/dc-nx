

struct ATNode
{
	// unlike QSNodes, an ATNode is always exactly 4 levels deep.
	// thus there is no 4th level; once at the 3rd level, all the
	// branches point to answers, using the answer[] array.
	union
	{
		ATNode *nodes[256];
		void *answers[256];
	};
};

// fromitem: the "key" (map_from) of the entry
// toitem: the object (map_to) of the entry
typedef void (*ATreeCallback)(void *fromitem, void *toitem, void *userdata);


class ATree
{
public:
	ATree();
	~ATree();
	
	void AddMapping(void *map_from, void *map_to);
	void *Lookup(void *map_from);
	void Delete(void *object_to_delete);
	void DoForEach(ATreeCallback func, void *userdata);

private:
	ATNode *BaseNode;
};
