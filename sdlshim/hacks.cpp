
#include <windows.h>

// stupid compiler requires this doesn't provide it itself.
// stubbing this out seems it won't break anything unless we use exceptions
extern "C"
{
	void __mingwthr_key_dtor(DWORD key, void (*dtor) (void *))
	{
	}
}
