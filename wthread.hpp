#if !defined(__WTHREAD_HPP) && defined(_MT) && defined(__cplusplus)
#define _INC_WTHREAD_HPP_

/*
* Base classes for thread object execution and encapsulation.
*
* Note differences between deriving WThread and deriving WRunnable directly!
* WThread derives WSynchronizer to keep the thread handle returned when the
* thread is created. This makes it possible to wait for thread termination by
* calling Wait() method. However, the WSynchronizer destructor closes the
* thread handle for WThread objects. Closing the thread handle may only be
* done in the thread that called  _beginthreadex() (by calling Start() method
* on the WThread object). Therefore, the thread code executed by Run() may
* never attempt to delete the object in which it is running.
* If you want the thread code to be able to delete the object, create a class
* directly deriving WRunnable instead of WThread. WRunnable itself is not a
* WSynchronizer class and doesn't keep the thread handle.
*/

#include <process.h>

#include <wsync.h>

inline bool SafeBeginThread(UINT(WINAPI *codeaddress)(LPVOID),
	UINT uiStackSize, LPVOID lpArglist)
{
	UINT uiTid = 0;
	HANDLE hThrd = (HANDLE)_beginthreadex(NULL, uiStackSize, codeaddress,
		lpArglist, 0, &uiTid);
	if (hThrd == NULL)
		return false;
	else
	{
		CloseHandle(hThrd);
		return true;
	}
}

/**
Base class for threadable objects. Contains methods to start execution of
implemented Run() method in a new thread.
*/
class WRunnable
{
private:
	/// Override this method with the method you want to run in a new thread.
	virtual UINT Run() = 0;

	/// Override when you don't want the thread handle to be closed immediately.
	/**
	This method is called after a successful call to _beginthreadex() by
	Start() method. You can override it with your own if you need the thread
	handle and don't want it to be closed immediately. The value returned by
	SetHandle() is then returned by Start().
	*/
	virtual BOOL SetHandle(HANDLE inith)
	{
		if (inith == INVALID_HANDLE_VALUE)
			return FALSE;
		else
			return CloseHandle(inith);
	}

	/// Static wrapper for Run() method.
	static UINT WINAPI WrapThread(LPVOID obj)
	{
		return ((WRunnable*)obj)->Run();
	}

public:
	/// Starts execution of Run() in a new thread.
	BOOL Start(DWORD dwCreationFlags = 0,
		DWORD dwStackSize = 0,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL)
	{
		// Use wrapper in WRunnable as address in _beingthreadex() call
		UINT uiTid = 0;
		return SetHandle((HANDLE)_beginthreadex(lpThreadAttributes, dwStackSize,
			WrapThread, this, dwCreationFlags,
			&uiTid));
	}
};

/**
WThread base class.
Instruction: Create a class deriving WThread. Let the deriving class
override the Run() method. Create an object of your class and call Start()
to start your Run() method in it's own thread.
Note! If you call Start() more than once, the handle of the already started
thread is automatically closed. This may lead to unwanted behaviour if
anything else in your code is using the handle, e.g. Wait() method will
immediately return WAIT_FAILED and GetLastError() returning something like
ERROR_INVALID_HANDLE.
*/
class WThread : public WSynchronizer, public WRunnable
{
private:
	/**
	Initialize WSynchronizer handle member with the thread handle.
	This implements the Wait() method to wait for the thread to terminate,
	and GetHandle() method to return the process-global thread handle.
	*/
	/// Initialize WSynchronizer handle member with the thread handle.
	BOOL SetHandle(HANDLE inith)
	{
		Close();

		hObject = inith;

		return hObject != INVALID_HANDLE_VALUE;
	}

protected:
	/// Initializes with no synchronizing object (no thread created yet).
	WThread() : WSynchronizer(INVALID_HANDLE_VALUE) { }

public:
	/// Suspends thread execution.
	DWORD Suspend()
	{
		return SuspendThread(hObject);
	}

	/// Resumes thread execution.
	/**
	Resumes thread execution.
	After a call to Suspend() or if Start() is called with CREATE_SUSPENDED
	you can use Resume() to begin execution of your thread code.
	*/
	DWORD Resume()
	{
		return ResumeThread(hObject);
	}
};
#endif  // _INC_WTHREAD_HPP_
