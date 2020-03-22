#if !defined(_INC_WSYNC_) && defined(__cplusplus)
#define _INC_WSYNC_

#include <windows.h>
#include <wbase.h>

// Encapsulated CRITICAL_SECTION
class WCriticalSection
{
   protected:
   CRITICAL_SECTION CriticalSection;

   public:
   VOID Enter()
   {
      EnterCriticalSection(&CriticalSection);
   }

   VOID Leave()
   {
      LeaveCriticalSection(&CriticalSection);
   }

#if _WIN32_WINNT >= 0x0400
   BOOL TryEnter()
   {
      return TryEnterCriticalSection(&CriticalSection);
   }
#endif

   WCriticalSection()
   {
      InitializeCriticalSection(&CriticalSection);
   }

   ~WCriticalSection()
   {
      DeleteCriticalSection(&CriticalSection);
   }
};

/**
   Base class for sync-object classes.
   Adds functionality for waiting to WKernelObject.
   */
class WSynchronizer : public WKernelObject
{
   protected:
   WSynchronizer(HANDLE handle)
   : WKernelObject(handle)
   {
   }

   public:
   DWORD Wait(DWORD dwMilliseconds = INFINITE)
   {
      return WaitForSingleObject(hObject, dwMilliseconds);
   }

#if _WIN32_WINNT >= 0x0400
   BOOL SignalAndWaitForObject(HANDLE hObjectToWaitOn,
      DWORD dwMilliseconds = INFINITE,	// time-out interval in milliseconds
      BOOL bAlertable = false)	// alertable flag
   {
      return ::SignalObjectAndWait(hObject, hObjectToWaitOn, dwMilliseconds,
         bAlertable);
   }

   BOOL SignalAndWaitForObject(const WSynchronizer &ObjectToWaitOn,
      DWORD dwMilliseconds = INFINITE,	// time-out interval in milliseconds
      BOOL bAlertable = false)	// alertable flag
   {
      return ::SignalObjectAndWait(hObject, ObjectToWaitOn.Handle(),
         dwMilliseconds, bAlertable);
   }

   BOOL SignalObjectAndWait(HANDLE hObjectToSignal,
      DWORD dwMilliseconds = INFINITE,	// time-out interval in milliseconds
      BOOL bAlertable = false)	// alertable flag
   {
      return ::SignalObjectAndWait(hObjectToSignal, hObject, dwMilliseconds,
         bAlertable);
   }

   BOOL SignalObjectAndWait(const WSynchronizer &ObjectToSignal,
      DWORD dwMilliseconds = INFINITE,	// time-out interval in milliseconds
      BOOL bAlertable = false)	// alertable flag
   {
      return ::SignalObjectAndWait(ObjectToSignal.Handle(), hObject,
         dwMilliseconds, bAlertable);
   }
#endif
};

class WEvent: public WSynchronizer
{
   public:
   explicit WEvent(
      BOOL bManualReset = TRUE,	// flag for manual-reset event
      BOOL bInitialState = FALSE,	// flag for initial state
      LPSECURITY_ATTRIBUTES lpEventAttributes = NULL,	// pointer to sec attr
      LPCSTR lpName = NULL) 	// pointer to event-object name
   : WSynchronizer(CreateEventA(lpEventAttributes, bManualReset, bInitialState,
      lpName))
   { }

   explicit WEvent(
      BOOL bManualReset,	// flag for manual-reset event
      BOOL bInitialState,	// flag for initial state
      LPSECURITY_ATTRIBUTES lpEventAttributes,	// pointer to sec attr
      LPCWSTR lpName) 	// pointer to event-object name
   : WSynchronizer(CreateEventW(lpEventAttributes, bManualReset, bInitialState,
      lpName))
   { }

   explicit WEvent(
      LPCSTR lpName, 	// pointer to event-object name
      DWORD dwDesiredAccess = EVENT_ALL_ACCESS,	// access flag
      BOOL bInheritHandle = FALSE)	// inherit flag
   : WSynchronizer(OpenEventA(dwDesiredAccess, bInheritHandle, lpName))
   { }

   explicit WEvent(
      LPCWSTR lpName, 	// pointer to event-object name
      DWORD dwDesiredAccess = EVENT_ALL_ACCESS,	// access flag
      BOOL bInheritHandle = FALSE)	// inherit flag
   : WSynchronizer(OpenEventW(dwDesiredAccess, bInheritHandle, lpName))
   { }

   BOOL Set()
   {
      return SetEvent(hObject);
   }

   BOOL Reset()
   {
      return ResetEvent(hObject);
   }

   BOOL Pulse()
   {
      return PulseEvent(hObject);
   }
};

class WMutex: public WSynchronizer
{
   public:
   explicit WMutex(
      BOOL bInitialOwner = true,	// flag for initial ownership
      LPCTSTR lpName = NULL, 	// pointer to mutex-object name
      LPSECURITY_ATTRIBUTES lpMutexAttributes = NULL)	// pointer to sec attr
   : WSynchronizer(CreateMutex(lpMutexAttributes, bInitialOwner, lpName))
   { }

   explicit WMutex(
      LPCTSTR lpName, 	// pointer to mutex-object name
      DWORD dwDesiredAccess = MUTEX_ALL_ACCESS,	// access flag
      BOOL bInheritHandle = false)	// inherit flag
   : WSynchronizer(OpenMutex(dwDesiredAccess, bInheritHandle, lpName))
   { }

   BOOL Release()
   {
      return ReleaseMutex(hObject);
   }
};

class WSemaphore: public WSynchronizer
{
   public:
   explicit WSemaphore(
      LONG lInitialCount = 0,	// initial count
      LONG lMaximumCount = 1,	// maximum count
      LPSECURITY_ATTRIBUTES lpSemaphoreAttributes = NULL, // ptr to sec attr
      LPCTSTR lpName = NULL)
   : WSynchronizer(CreateSemaphore(lpSemaphoreAttributes, lInitialCount,
      lMaximumCount, lpName))
   { }

   explicit WSemaphore(
      LPCTSTR lpName, 	// pointer to mutex-object name
      DWORD dwDesiredAccess = MUTEX_ALL_ACCESS,	// access flag
      BOOL bInheritHandle = false)	// inherit flag
   : WSynchronizer(OpenSemaphore(dwDesiredAccess, bInheritHandle, lpName))
   { }

   BOOL Release(
      LONG lReleaseCount = 1,	// amount to add to current count
      LPLONG lpPreviousCount = NULL) 	// address of previous count
   {
      return ReleaseSemaphore(hObject, lReleaseCount, lpPreviousCount);
   }
};

#if _WIN32_WINNT >= 0x0400
#ifndef TIMER_ALL_ACCESS
#  define TIMER_QUERY_STATE       0x0001
#  define TIMER_MODIFY_STATE      0x0002
#  define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)
#endif

class WWaitableTimer : public WSynchronizer
{
   public:
   explicit WWaitableTimer(
      BOOL bManualReset = false,	// flag for manual reset state
      LPSECURITY_ATTRIBUTES lpTimerAttributes = NULL,	// pointer to sec attribs
      LPCTSTR lpTimerName = NULL)	// pointer to timer object name
   : WSynchronizer(CreateWaitableTimer(lpTimerAttributes, bManualReset,
      lpTimerName))
   { }

   explicit WWaitableTimer(
      LPCTSTR lpTimerName,	// pointer to timer object name
      DWORD dwDesiredAccess = TIMER_ALL_ACCESS,	// access flag
      BOOL bInheritHandle = false)	// inherit flag
   : WSynchronizer(OpenWaitableTimer(dwDesiredAccess, bInheritHandle,
      lpTimerName))
   { }

   BOOL Set(
      const LARGE_INTEGER *pDueTime,	// when timer will become signaled
      LONG lPeriod,	// periodic timer interval
      PTIMERAPCROUTINE pfnCompletionRoutine,	// pointer to completion routine
      LPVOID lpArgToCompletionRoutine,	// data passed to completion routine
      BOOL fResume)	// flag for resume state
   {
      return SetWaitableTimer(hObject, pDueTime, lPeriod, pfnCompletionRoutine,
         lpArgToCompletionRoutine, fResume);
   }

   BOOL Cancel()
   {
      return CancelWaitableTimer(hObject);
   }
};
#endif  // _WIN32_WINNT >= 0x0400
#endif  // _INC_WSYNC_

