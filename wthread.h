#if !defined(_INC_WTHREAD_) && defined(_MT)
#ifdef __cplusplus

#include <wthread.hpp>

#else // __cplusplus

#include <process.h>

EXTERN_C BOOL CDECL
SafeBeginThread(UINT (WINAPI *codeaddress)(LPVOID), UINT uiStackSize,
		LPVOID lpArglist);

#endif // __cplusplus

#endif /* _INC_WTHREAD_ && _MT */
