#ifndef _INC_WIO_
#define _INC_WIO_

#include <windows.h>

EXTERN_C VOID CALLBACK
FileIoCompletionRoutine(DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped);
#ifdef __cplusplus
#include <wbase.h>

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

EXTERN_C DWORD
LineRecv(HANDLE h, LPSTR pBuf, DWORD dwBufSiz, DWORD dwTimeout = INFINITE);
EXTERN_C DWORD
LineRecvW(HANDLE h, LPWSTR pBuf, DWORD dwBufChrs, DWORD dwTimeout = INFINITE);
EXTERN_C DWORD
ReadPassword(HANDLE h, LPSTR pBuf, DWORD dwBufSiz);
EXTERN_C DWORD
BufRecv(HANDLE h, PVOID pBuf, DWORD dwBufSize, DWORD dwTimeout = INFINITE);
EXTERN_C BOOL
BufSend(HANDLE h, const void *pBuf, DWORD dwBufSize,
    DWORD dwTimeout = INFINITE);
EXTERN_C BOOL
ReadCommandLine(HANDLE hConn, LPSTR lpBuffer, DWORD dwBufferSize,
    HANDLE hEchoDevice = INVALID_HANDLE_VALUE,
    DWORD dwTimeOut = INFINITE);

/// Send characters until null-terminator is reached.
inline BOOL StrSend(HANDLE h, LPCSTR pBuf, DWORD dwTimeout = INFINITE)
{
    if (pBuf == NULL)
        return BufSend(h, "(null)", 6, dwTimeout);
    else
        return BufSend(h, pBuf, (DWORD)strlen(pBuf), dwTimeout);
}

/// Send characters until null-terminator is reached.
inline BOOL WcsSend(HANDLE h, LPCWSTR pBuf, DWORD dwTimeout = INFINITE)
{
    if (pBuf == NULL)
        return BufSend(h, L"(null)", 12, dwTimeout);
    else
        return BufSend(h, pBuf, (DWORD)wcslen(pBuf) << 1, dwTimeout);
}

/// Enhanced OVERLAPPED structure with encapsulated API functions.
struct WOverlapped : public OVERLAPPED
{
    BOOL Read(HANDLE hFile, LPVOID lpBuf, DWORD dwLength, DWORDLONG dwStart = 0)
    {
        if (!ResetEvent())
            return FALSE;

        Offset = (DWORD)dwStart;
        OffsetHigh = (DWORD)(dwStart >> 32);
        DWORD dw;
        return ReadFile(hFile, lpBuf, dwLength, &dw, this);
    }

    BOOL Write(HANDLE hFile, LPCVOID lpBuf, DWORD dwLength,
        DWORDLONG dwStart = 0)
    {
        if (!ResetEvent())
            return FALSE;

        Offset = (DWORD)dwStart;
        OffsetHigh = (DWORD)(dwStart >> 32);
        DWORD dw;
        return WriteFile(hFile, lpBuf, dwLength, &dw, this);
    }

    DWORD BufRecv(HANDLE hFile, PVOID pBuf, DWORD dwBufSize,
        DWORD dwTimeout = INFINITE);

    BOOL BufSend(HANDLE hFile, const void *pBuf, DWORD dwBufSize,
        DWORD dwTimeout = INFINITE);

    DWORD LineRecv(HANDLE hFile, LPSTR lpBuf, DWORD dwBufSize,
        DWORD dwTimeout = INFINITE);

    DWORD LineRecvW(HANDLE hFile, LPWSTR lpBuf, DWORD dwBufSize,
        DWORD dwTimeout = INFINITE);

    /// Send characters until null-terminator is reached.
    BOOL StrSend(HANDLE h, LPCSTR pBuf, DWORD dwTimeout = INFINITE)
    {
        if (pBuf == NULL)
            return BufSend(h, "(null)", 6, dwTimeout);
        else
            return BufSend(h, pBuf, (DWORD)strlen(pBuf), dwTimeout);
    }

    /// Send characters until null-terminator is reached.
    BOOL WcsSend(HANDLE h, LPCWSTR pBuf, DWORD dwTimeout = INFINITE)
    {
        if (pBuf == NULL)
            return BufSend(h, L"(null)", 12, dwTimeout);
        else
            return BufSend(h, pBuf, (DWORD)wcslen(pBuf) << 1, dwTimeout);
    }

    /* Read until end-of-line and optionally echo typed characters to
    * hEchoDevice.
    * Set hEchoDevice to INVALID_HANDLE_VALUE to turn off echoing.
    * This function supports backspace functionality. dwTimeOut is the timeout
    * in ms, set to INFINITE to disable timeouting.
    */
    BOOL
        ReadCommandLine(HANDLE hConn, LPSTR lpBuffer, DWORD dwBufferSize,
            HANDLE hEcho, DWORD dwTimeout = INFINITE);

    BOOL ConnectNamedPipe(HANDLE hNamedPipe)
    {
        return ::ConnectNamedPipe(hNamedPipe, this);
    }

    BOOL WaitCommEvent(HANDLE hFile, LPDWORD lpEvtMask)
    {
        return ::WaitCommEvent(hFile, lpEvtMask, this);
    }

    BOOL GetResult(HANDLE hFile, LPDWORD lpNumberOfBytesTransferred,
        BOOL bWait = TRUE)
    {
        return GetOverlappedResult(hFile, this, lpNumberOfBytesTransferred,
            bWait);
    }

    bool Wait(DWORD dwTimeout = INFINITE)
    {
        return WaitForSingleObject(hEvent, dwTimeout) == WAIT_OBJECT_0;
    }

    bool IsComplete()
    {
        return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
    }

    BOOL SetEvent()
    {
        return ::SetEvent(hEvent);
    }

    BOOL ResetEvent()
    {
        return ::ResetEvent(hEvent);
    }

    BOOL PulseEvent()
    {
        return ::PulseEvent(hEvent);
    }

    operator bool() const
    {
        return hEvent != NULL;
    }

    bool operator!() const
    {
        return hEvent == NULL;
    }

    explicit WOverlapped(OVERLAPPED &ol)
    {
        *(OVERLAPPED*)this = ol;
    }

    explicit WOverlapped(BOOL bManualReset = true, BOOL bSignalled = false)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = CreateEventA(NULL, bManualReset, bSignalled, NULL);
    }

    explicit WOverlapped(LPCSTR lpName)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = OpenEventA(EVENT_ALL_ACCESS, false, lpName);
    }

    explicit WOverlapped(LPCWSTR lpName)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = OpenEventW(EVENT_ALL_ACCESS, false, lpName);
    }

    ~WOverlapped()
    {
        if (hEvent != NULL)
            CloseHandle(hEvent);
    }
};

/// Enhanced OVERLAPPED stucture with encapsulated I/O completion API.
struct WOverlappedIOC : public OVERLAPPED
{
    BOOL Read(HANDLE hFile, LPVOID lpBuffer, DWORD dwBytes)
    {
        if (!ResetEvent())
            return false;
        return ReadFileEx(hFile, lpBuffer, dwBytes, this, FileIoCompletionRoutine);
    }

    BOOL Write(HANDLE hFile, LPCVOID lpBuffer, DWORD dwBytes)
    {
        if (!ResetEvent())
            return false;
        return WriteFileEx(hFile, lpBuffer, dwBytes, this,
            FileIoCompletionRoutine);
    }

    DWORD
        LineRecv(HANDLE h, LPSTR pBuf, DWORD dwBufSize, DWORD dwTimeout = INFINITE);

    DWORD
        LineRecvW(HANDLE h, LPWSTR pBuf, DWORD dwBufChrs,
            DWORD dwTimeout = INFINITE);

    DWORD
        BufRecv(HANDLE h, PVOID pBuf, DWORD dwBufSize, DWORD dwTimeout = INFINITE);

    BOOL
        BufSend(HANDLE h, const void *pBuf, DWORD dwBufSize,
            DWORD dwTimeout = INFINITE);

    /// Send characters until null-terminator is reached.
    BOOL StrSend(HANDLE h, LPCSTR pBuf, DWORD dwTimeout = INFINITE)
    {
        if (pBuf == NULL)
            return BufSend(h, "(null)", 6, dwTimeout);
        else
            return BufSend(h, pBuf, (DWORD)strlen(pBuf), dwTimeout);
    }

    /// Send characters until null-terminator is reached.
    BOOL WcsSend(HANDLE h, LPCWSTR pBuf, DWORD dwTimeout = INFINITE)
    {
        if (pBuf == NULL)
            return BufSend(h, L"(null)", 12, dwTimeout);
        else
            return BufSend(h, pBuf, (DWORD)wcslen(pBuf) << 1, dwTimeout);
    }

    HANDLE Handle()
    {
        return hEvent;
    }

    bool GetResult(LPDWORD lpNumberOfBytesTransfered, bool bWait = true)
    {
        *lpNumberOfBytesTransfered = 0;

        if (bWait)
        {
            if (!Wait())
                return false;
        }
        else
            if (!IsComplete())
            {
                SetLastError(ERROR_IO_INCOMPLETE);
                return false;
            }

        SetLastError((DWORD)InternalHigh);

        if (InternalHigh == ERROR_SUCCESS)
        {
            *lpNumberOfBytesTransfered = (DWORD)Internal;
            return true;
        }
        else
        {
            *lpNumberOfBytesTransfered = (DWORD)Internal;
            return false;
        }
    }

    bool Wait(DWORD dwTimeout = INFINITE)
    {
        for (;;)
            switch (WaitForSingleObjectEx(hEvent, dwTimeout, TRUE))
            {
            case WAIT_OBJECT_0:
                return true;
            case WAIT_IO_COMPLETION:
                continue;
            default:
                return false;
            }
    }

    bool IsComplete()
    {
        SleepEx(0, TRUE);
        return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
    }

    BOOL SetEvent()
    {
        return ::SetEvent(hEvent);
    }

    BOOL ResetEvent()
    {
        return ::ResetEvent(hEvent);
    }

    BOOL PulseEvent()
    {
        return ::PulseEvent(hEvent);
    }

    operator bool() const
    {
        return hEvent != NULL;
    }

    bool operator!() const
    {
        return hEvent == NULL;
    }

    explicit WOverlappedIOC(OVERLAPPED &ol)
    {
        *(OVERLAPPED*)this = ol;
    }

    explicit WOverlappedIOC(BOOL bManualReset = true, BOOL bSignalled = false)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = CreateEventA(NULL, bManualReset, bSignalled, NULL);
    }

    explicit WOverlappedIOC(LPCSTR lpName)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = OpenEventA(EVENT_ALL_ACCESS, false, lpName);
    }

    explicit WOverlappedIOC(LPCWSTR lpName)
    {
        ZeroMemory(this, sizeof *this);
        hEvent = OpenEventW(EVENT_ALL_ACCESS, false, lpName);
    }

    ~WOverlappedIOC()
    {
        if (hEvent != NULL)
            CloseHandle(hEvent);
    }
};

class WFile : public WKernelObject
{
public:
    WFile() { }

    WFile(LPCSTR lpFileName,	// pointer to name of the file
        DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,	// access mode
        DWORD dwShareMode = 0,	// share mode
        LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL,  // ptr to sec attr
        DWORD dwCreationDistribution = OPEN_EXISTING,	// how to create
        DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,  // file attributes
        HANDLE hTemplateFile = NULL)
        : WKernelObject(CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDistribution,
            dwFlagsAndAttributes, hTemplateFile))
    {
    }

    WFile(LPCWSTR lpFileName,	// pointer to name of the file
        DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,	// access mode
        DWORD dwShareMode = 0,	// share mode
        LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL,  // ptr to sec attr
        DWORD dwCreationDistribution = OPEN_EXISTING,	// how to create
        DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,  // file attributes
        HANDLE hTemplateFile = NULL)
        : WKernelObject(CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDistribution,
            dwFlagsAndAttributes, hTemplateFile))
    {
    }

    WFile(HANDLE handle)
        : WKernelObject(handle)
    {
    }

    BOOL Open(LPCTSTR lpFileName,	// pointer to name of the file
        DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,// access mode
        DWORD dwShareMode = 0,	// share mode
        LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL, // ptr to sec
        DWORD dwCreationDistribution = OPEN_EXISTING,  // how to create
        DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL, // file attr
        HANDLE hTemplateFile = NULL)
    {
        Close();
        hObject = CreateFile(lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDistribution,
            dwFlagsAndAttributes, hTemplateFile);
        return hObject != INVALID_HANDLE_VALUE;
    }

    DWORDLONG GetSize()
    {
        ULARGE_INTEGER FileSize = { 0 };
        FileSize.LowPart = GetFileSize(hObject, &FileSize.HighPart);

        if (FileSize.LowPart == INVALID_FILE_SIZE &&
            GetLastError() != NO_ERROR)
        {
            return (DWORDLONG)-1;
        }

        return FileSize.QuadPart;
    }
};

struct WPipe
{
    WKernelObject Read;
    WKernelObject Write;

    operator bool()
    {
        return Read && Write;
    }

    bool operator !()
    {
        return (!Read) || (!Write);
    }

    explicit WPipe(SECURITY_ATTRIBUTES *sa = NULL, DWORD dwBufferSize = 0)
    {
        HANDLE hRead;
        HANDLE hWrite;

        if (!CreatePipe(&hRead, &hWrite, sa, dwBufferSize))
            return;

        Read = hRead;
        Write = hWrite;
    }

    bool Close()
    {
        return Read.Close() | Write.Close();
    }
};

#else  // __cplusplus

DWORD LineRecv(HANDLE h, LPSTR pBuf, DWORD dwBufSiz, DWORD dwTimeout);
DWORD LineRecvW(HANDLE h, LPWSTR pBuf, DWORD dwBufChrs, DWORD dwTimeout);
DWORD ReadPassword(HANDLE h, LPSTR pBuf, DWORD dwBufSiz);
DWORD BufRecv(HANDLE h, PVOID pBuf, DWORD dwBufSize, DWORD dwTimeout);
BOOL BufSend(HANDLE h, const void *pBuf, DWORD dwBufSize, DWORD dwTimeout);
BOOL ReadCommandLine(HANDLE hConn, LPSTR lpBuffer, DWORD dwBufferSize,
    HANDLE hEchoDevice, DWORD dwTimeOut);

#define StrSend(h,b,t) ((b)==NULL?BufSend((h),"(null)",6,(t)):\
			BufSend((h),(b),strlen(b),(t))
#define WcsSend(h,b,t) ((b)==NULL?BufSend((h),L"(null)",12,(t)):\
			BufSend((h),(b),wcslen(b)<<1,(t))

#endif // __cplusplus

#endif // _INC_WIO_

