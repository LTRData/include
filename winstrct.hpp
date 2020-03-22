/* winstrct.h
*
* Defines Windows "Unix-like" error number and error message variables and
* functions.
* C++: Encapsulates in C++ objects some Windows API get/set function pairs.
* Written by Olof Lagerkvist 2000-2005.
*/

#ifndef _INC_WINSTRCT_HPP_
#define _INC_WINSTRCT_HPP_

#ifndef _INC_WINSTRCT_
#include <winstrct.h>
#endif

#if __BCPLUSPLUS__ > 0x0520
#pragma option push -Ve -Vx
#endif

/*
WPreserveLastError

An object of this class preserves value of Win32 GetLastError(). Constructor
calls GetLastError() and saves returned value. Destructor calls SetLastError()
with saved value.
*/
class WPreserveLastError
{
public:
    DWORD Value;

    WPreserveLastError()
    {
        Value = GetLastError();
    }

    ~WPreserveLastError()
    {
        SetLastError(Value);
    }
};

struct WSystemInfo : public SYSTEM_INFO
{
    WSystemInfo()
    {
        GetSystemInfo(this);
    }
};

#if _WIN32_WINNT >= 0x0501
struct WNativeSystemInfo : public SYSTEM_INFO
{
    WNativeSystemInfo()
    {
        GetNativeSystemInfo(this);
    }
};
#endif

struct WOSVersionInfo : public OSVERSIONINFO
{
    WOSVersionInfo()
    {
        dwOSVersionInfoSize = sizeof(*this);
        
#if _MSC_VER >= 1500
#pragma warning(suppress: 28159)
#pragma warning(suppress: 4996)
        GetVersionEx(this);
#else
        GetVersionEx(this);
#endif
    }
};

struct WOSVersionInfoEx : public OSVERSIONINFOEX
{
    WOSVersionInfoEx()
    {
        dwOSVersionInfoSize = sizeof(*this);

#if _MSC_VER >= 1500
#pragma warning(suppress: 28159)
#pragma warning(suppress: 4996)
        GetVersionEx((LPOSVERSIONINFO)this);
#else
        GetVersionEx((LPOSVERSIONINFO)this);
#endif
    }
};

template<typename T> class WMemHolder
{
protected:
    T *ptr;

public:
    operator bool() const
    {
        return ptr != NULL;
    }

    bool operator!() const
    {
        return ptr == NULL;
    }

    operator T*() const
    {
        return ptr;
    }

    T* operator ->() const
    {
        return ptr;
    }

    T* operator+(char i) const
    {
        return ptr + i;
    }

    T* operator-(char i) const
    {
        return ptr - i;
    }

    T* operator+(unsigned char i) const
    {
        return ptr + i;
    }

    T* operator-(unsigned char i) const
    {
        return ptr - i;
    }

    T* operator+(int i) const
    {
        return ptr + i;
    }

    T* operator-(int i) const
    {
        return ptr - i;
    }

    T* operator+(unsigned int i) const
    {
        return ptr + i;
    }

    T* operator-(unsigned int i) const
    {
        return ptr - i;
    }

    T* operator+(long i) const
    {
        return ptr + i;
    }

    T* operator-(long i) const
    {
        return ptr - i;
    }

    T* operator+(unsigned long i) const
    {
        return ptr + i;
    }

    T* operator-(unsigned long i) const
    {
        return ptr - i;
    }

    T* operator+(long long i) const
    {
        return ptr + i;
    }

    T* operator-(long long i) const
    {
        return ptr - i;
    }

    T* operator+(unsigned long long i) const
    {
        return ptr + i;
    }

    T* operator-(unsigned long long i) const
    {
        return ptr - i;
    }

    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    T* GetPtr()
    {
        return ptr;
    }

    T* GetPtr(LONG_PTR offset)
    {
        return ptr + offset;
    }

    template<typename TC> TC* GetPtr()
    {
        return (TC*)(void*)ptr;
    }

    template<typename TC> TC* GetPtr(LONG_PTR offset)
    {
        return ((TC*)(void*)ptr) + offset;
    }

    T* Abandon()
    {
        T* ab_ptr = ptr;
        ptr = NULL;
        return ab_ptr;
    }

    WMemHolder()
        : ptr(NULL) { }

    explicit WMemHolder(T *pBlk)
        : ptr(pBlk)
    {
    }
};

class WEnvironmentStrings : public WMemHolder<TCHAR>
{
public:
    WEnvironmentStrings()
        : WMemHolder<TCHAR>(GetEnvironmentStrings())
    {
    }

    BOOL Free()
    {
        if (ptr != NULL)
        {
            return FreeEnvironmentStrings(ptr);
        }
        else
        {
            return TRUE;
        }
    }

    ~WEnvironmentStrings()
    {
        Free();
    }
};

template<typename T> class WMem : public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    DWORD_PTR Count() const
    {
        return GetSize() / sizeof(T);
    }

    DWORD_PTR GetSize() const
    {
        if (ptr == NULL)
            return 0;
        else
            return LocalSize(ptr);
    }

    /* WMem::ReAlloc()
    *
    * Note that this function uses LocalReAlloc() which makes it lose the
    * data if the block must be moved to increase.
    */
    T* ReAlloc(DWORD dwAllocSize)
    {
        T *newblock = (T*)LocalReAlloc(ptr, dwAllocSize, LMEM_ZEROINIT);
        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    T* Free()
    {
        if (ptr == NULL)
            return NULL;
        else
            return ptr = (T*)LocalFree(ptr);
    }

    WMem()
    {
    }

    explicit WMem(DWORD dwAllocSize)
        : WMemHolder<T>((T*)LocalAlloc(LPTR, dwAllocSize))
    {
    }

    explicit WMem(T *pBlk)
        : WMemHolder<T>(pBlk)
    {
    }

    ~WMem()
    {
        Free();
    }
};

#ifdef _INC_MALLOC
template<typename T> class WCRTMem : public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    size_t Count() const
    {
        return GetSize() / sizeof(T);
    }

    size_t GetSize() const
    {
        if (ptr == NULL)
            return 0;
        else
            return _msize(ptr);
    }

    /* WHeapMem::ReAlloc()
    *
    * This function uses realloc() which makes it preserve the data if the
    * block must be moved to increase.
    */
    T* ReAlloc(size_t dwAllocSize)
    {
        T *newblock = (T*)realloc(ptr, dwAllocSize);
        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    void Free()
    {
        if (ptr != NULL)
        {
            free(ptr);
            ptr = NULL;
        }
    }

    WCRTMem()
    {
    }

    explicit WCRTMem(size_t dwAllocSize)
        : WMemHolder<T>((T*)malloc(dwAllocSize)) { }

    explicit WCRTMem(T *pBlk)
        : WMemHolder<T>(pBlk) { }

    ~WCRTMem()
    {
        Free();
    }
};
#endif

template<typename T> class WHeapMem :public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    SIZE_T Count() const
    {
        return GetSize() / sizeof(T);
    }

    SIZE_T GetSize(DWORD dwFlags = 0) const
    {
        if (ptr == NULL)
            return 0;
        else
            return HeapSize(GetProcessHeap(), dwFlags, ptr);
    }

    /* WHeapMem::ReAlloc()
    *
    * This function uses HeapReAlloc() which makes it preserve the data if the
    * block must be moved to increase.
    */
    T* ReAlloc(SIZE_T AllocSize, DWORD dwFlags = 0)
    {
        if (ptr == NULL)
        {
            return ptr =
                (T*)HeapAlloc(GetProcessHeap(), dwFlags, AllocSize);
        }

        T *newblock = (T *)HeapReAlloc(GetProcessHeap(), dwFlags, ptr, AllocSize);

        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    T *Free(DWORD dwFlags = 0)
    {
        if ((this == NULL) || (ptr == NULL))
            return NULL;
        else if (HeapFree(GetProcessHeap(), dwFlags, ptr))
            return ptr = NULL;
        else
#if _MSC_VER >= 1500
#pragma warning(suppress: 6001)
#endif
            return ptr;
    }

    WHeapMem()
    {
    }

    explicit WHeapMem(SIZE_T dwAllocSize, DWORD dwFlags = 0)
        : WMemHolder<T>((T*)HeapAlloc(GetProcessHeap(), dwFlags, dwAllocSize))
    {
    }

    explicit WHeapMem(T *pBlk)
        : WMemHolder<T>(pBlk)
    {
    }

    ~WHeapMem()
    {
        Free();
    }
};

/* WErrMsg objects are auto-initialized with a buffer to win_error message or
* any error message.
*
* The buffer is automatically freed by destructor.
*/
class WErrMsgA : public WMem<CHAR>
{
public:
    LPSTR operator =(DWORD dwErrno)
    {
        Free();
        return ptr = win_errmsgA(dwErrno);
    }

    WErrMsgA() : WMem<CHAR>(win_errmsgA(GetLastError())) { }

    explicit WErrMsgA(DWORD dwErrno) : WMem<CHAR>(win_errmsgA(dwErrno)) { }
};
class WErrMsgW : public WMem<WCHAR>
{
public:
    LPWSTR operator =(DWORD dwErrno)
    {
        Free();
        return ptr = win_errmsgW(dwErrno);
    }

    WErrMsgW() : WMem<WCHAR>(win_errmsgW(GetLastError())) { }

    explicit WErrMsgW(DWORD dwErrno) : WMem<WCHAR>(win_errmsgW(dwErrno)) { }
};
#ifdef UNICODE
#define WErrMsg WErrMsgW
#else
#define WErrMsg WErrMsgA
#endif

#ifdef _WINSOCKAPI_
/* WSockErrMsg completes the WErrMsg class with error messages from Windows
* Socket 32-bit system.
*/
class WSockErrMsg : public WMem<TCHAR>
{
public:
    LPTSTR operator =(DWORD dwErrno)
    {
        Free();
        return ptr = win_errmsg(dwErrno);
    }

    WSockErrMsg() : WMem<TCHAR>((LPTSTR)h_error) { }

    explicit WSockErrMsg(DWORD dwErrno) : WMem<TCHAR>(win_errmsg(dwErrno)) { }
};
#endif

/* WPDHErrMsg completes the WErrMsg class with error messages from Performance
* Data Helper module.
*/
class WPDHErrMsg : public WMem<TCHAR>
{
public:
    LPTSTR operator =(DWORD dwErrno)
    {
        Free();
        return ptr = pdh_errmsg(dwErrno);
    }

    WPDHErrMsg() : WMem<TCHAR>((LPTSTR)pdh_error) { }

    explicit WPDHErrMsg(DWORD dwErrno) : WMem<TCHAR>(pdh_errmsg(dwErrno)) { }
};

#ifdef _NTDDK_
/* WNTErrMsg completes the WErrMsg class with error messages from Windows NT
* status codes.
*/
class WNTErrMsgA : public WMem<CHAR>
{
public:
    LPSTR operator =(NTSTATUS status)
    {
        Free();
        return ptr = nt_errmsgA(status);
    }

    explicit WNTErrMsgA(NTSTATUS status) : WMem<CHAR>(nt_errmsgA(status)) { }
};
class WNTErrMsgW : public WMem<WCHAR>
{
public:
    LPWSTR operator =(NTSTATUS status)
    {
        Free();
        return ptr = nt_errmsgW(status);
    }

    explicit WNTErrMsgW(NTSTATUS status) : WMem<WCHAR>(nt_errmsgW(status)) { }
};
#ifdef UNICODE
#define WNTErrMsg WNTErrMsgW
#else
#define WNTErrMsg WNTErrMsgA
#endif
#endif

#ifdef UNICODE
class WMsgOEM : public WHeapMem<CHAR>
{
public:
    explicit WMsgOEM(WMem<WCHAR> &mem);
};
#endif

/* win_perror()
*
* Used similar to perror() in Unix environments.
*
* Used to print to stderr the error message associated with the error code of
* the most recently non-sucessful call to a Windows API function. The error
* message buffer is only used internally and is automatically freed by this
* function.
*/
EXTERN_C void win_perrorA(LPCSTR __errmsg = NULL);
EXTERN_C void win_perrorW(LPCWSTR __errmsg = NULL);
#ifdef UNICODE
#define win_perror win_perrorW
#else
#define win_perror win_perrorA
#endif

/* h_perror()
*
*/
EXTERN_C void h_perrorA(LPCSTR __errmsg = NULL);
EXTERN_C void h_perrorW(LPCWSTR __errmsg = NULL);
#ifdef UNICODE
#define h_perror h_perrorW
#else
#define h_perror h_perrorA
#endif

/* pdh_perror()
*
*/
EXTERN_C void pdh_perrorA(LPCSTR __errmsg = NULL);
EXTERN_C void pdh_perrorW(LPCWSTR __errmsg = NULL);
#ifdef UNICODE
#define pdh_perror pdh_perrorW
#else
#define pdh_perror pdh_perrorA
#endif

#ifdef _NTDDK_
/* nt_perror()
*
*/
EXTERN_C void nt_perrorA(NTSTATUS status, LPCSTR __errmsg = NULL);
EXTERN_C void nt_perrorW(NTSTATUS status, LPCWSTR __errmsg = NULL);
#ifdef UNICODE
#define nt_perror nt_perrorW
#else
#define nt_perror nt_perrorA
#endif
#endif // _NTDDK_

#if __BCPLUSPLUS__ > 0x0520
#pragma option pop
#pragma option pop
#endif

#endif  // _INC_WINSTRCT_HPP_
