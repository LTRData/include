/* winstrct.h
*
* Defines Windows "Unix-like" error number and error message variables and
* functions.
* C++: Encapsulates in C++ objects some Windows API get/set function pairs.
* Written by Olof Lagerkvist 2000-2005.
*/

#ifndef _INC_WINSTRCT_
#define _INC_WINSTRCT_

#ifndef _INC_WINDOWS_
# include <windows.h>
#endif

#ifndef _USERENTRY
#  define _USERENTRY
#endif

// The inlines here uses the stream I/O library functions.
#ifndef _INC_STDIO
#  include <stdio.h>
#endif

#include <tchar.h>

#ifndef EXTERN_C
#  ifdef __cplusplus
#     define EXTERN_C extern "C"
#  else
#     define EXTERN_C
#  endif
#endif

#define WINSOCK_MODULE TEXT("WSOCK32")
#define PDH_MODULE TEXT("PDH")
#define NTDLL_MODULE TEXT("NTDLL")

#ifdef _MSC_VER
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")

#ifndef NO_WINSTRCT_LIB_IMPORT
#pragma comment(lib, "winstrcp.lib")
#pragma comment(lib, "winstrct.lib")
#endif
#endif

// import/export macros
#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)

#ifdef __DLL__
# define DLL_EXTERN EXPORT
# define EXE_EXTERN IMPORT
#else
# define DLL_EXTERN IMPORT
# define EXE_EXTERN EXPORT
#endif

#if __BCPLUSPLUS__ > 0x0520
#pragma option push -vi
#endif

#if defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
#define _UNALIGNED __unaligned
#else
#define _UNALIGNED
#endif

/* _countof helper */
#if !defined(_countof)
#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
    template <typename _CountofType, size_t _SizeOfArray>
    char(*__countof_helper(_UNALIGNED _CountofType(&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
}
#endif
#endif

#ifndef INVALID_HKEY
#define INVALID_HKEY ((HKEY)INVALID_HANDLE_VALUE)
#endif

#ifndef WORD_MIN
#define WORD_MIN ((WORD)0x80000000)
#endif

#ifndef WORD_MAX
#define WORD_MAX ((WORD)0x7FFFFFFF)
#endif

#ifndef DWORD_MAX
#define DWORD_MAX ((DWORD)0xFFFFFFFF)
#endif

#ifndef LONGLONG_MIN
#define LONGLONG_MIN ((DWORDLONG)0x8000000000000000)
#endif

#ifndef LONGLONG_MAX
#define LONGLONG_MAX ((DWORDLONG)0x7FFFFFFFFFFFFFFF)
#endif

#ifndef DWORDLONG_MAX
#define DWORDLONG_MAX ((DWORDLONG)0xFFFFFFFFFFFFFFFF)
#endif

// Windows version macros
#define WinVer_Major       (GetVersion()&0x000000FF)
#define WinVer_Minor       ((GetVersion()&0x0000FF00)>>8)
#define WinVer_Build       ((GetVersion()&0x7FFF0000)>>16)

// Windows platform macros (boolean return values)
#define WinVer_WindowsNT   (!(GetVersion()&0x80000000))
#define WinVer_Windows95   (!!((GetVersion()&0x800000FF) >= 0x80000004))
#define WinVer_Win32s	   (!!(((GetVersion()^0x80000000)&0x800000FF) < 4))
#define WinVer_Win95orNT4  (WinVer_Major >= 4)
#define WinVer_Win32sor95  (!!(GetVersion()&0x80000000))
#define WinVer_Win32sorNT3 (WinVer_Major < 4)
#define WinVer_Win2korXP   (WinVer_Major >= 5)

typedef struct _REPARSE_DATA
{
    DWORD ReparseTag;
    WORD ReparseDataLength;
    WORD Reserved;

    union
    {
        struct _REPARSE_DATA_MOUNT_POINT
        {
            WORD NameOffset;
            WORD NameLength;
            WORD DisplayNameOffset;
            WORD DisplayNameLength;
            BYTE Data[1];
        } MountPoint;

        struct _REPARSE_DATA_SYMLINK
        {
            WORD NameOffset;
            WORD NameLength;
            WORD DisplayNameOffset;
            WORD DisplayNameLength;
            WORD ReservedOffset;
            WORD ReservedLength;
            BYTE Data[1];
        } SymLink;
    };
} REPARSE_DATA, *PREPARSE_DATA;

/// Macros for "human readable" sizes in bytes
#define _1KB  (1ui64<<10)
#define _1MB  (1ui64<<20)
#define _1GB  (1ui64<<30)
#define _1TB  (1ui64<<40)

#define TO_B(n)  ((double)(n))
#define TO_KB(n) ((double)(n)/_1KB)
#define TO_MB(n) ((double)(n)/_1MB)
#define TO_GB(n) ((double)(n)/_1GB)
#define TO_TB(n) ((double)(n)/_1TB)

#define TO_h(n) ((n)>=_1TB ? TO_TB(n) : (n)>=_1GB ? TO_GB(n) :	\
	        (n)>=_1MB ? TO_MB(n) : (n)>=_1KB ? TO_KB(n) : (n))

#define TO_pA(n) ((n)>=_1TB ? "TB" : (n)>=_1GB ? "GB" :			\
	        (n)>=_1MB ? "MB" : (n)>=_1KB ? "KB": (n)==1 ? "byte" : "bytes")

#define TO_pW(n) ((n)>=_1TB ? L"TB" : (n)>=_1GB ? L"GB" :			\
	        (n)>=_1MB ? L"MB" : (n)>=_1KB ? L"KB": (n)==1 ? L"byte" : L"bytes")

#ifdef UNICODE
#define TO_p(n) TO_pW(n)
#else
#define TO_p(n) TO_pA(n)
#endif

/// Prototype for rundll32.exe callable functions.
typedef void WINAPI
RunDLLFunc(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

#define halloc_seh(n) HeapAlloc(GetProcessHeap(),HEAP_GENERATE_EXCEPTIONS,(n))
#define halloc(n) HeapAlloc(GetProcessHeap(), 0, (n))
#define hrealloc_seh(p, n) ((p) != NULL ? \
                       HeapReAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, (p), (n)) : \
                       HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, (n)))
#define hrealloc(p, n) ((p) != NULL ? \
                       HeapReAlloc(GetProcessHeap(), 0, (p), (n)) : \
                       HeapAlloc(GetProcessHeap(), 0, (n)))
#define hsize(p) HeapSize(GetProcessHeap(), 0, (p))
#define hfree(p) HeapFree(GetProcessHeap(), 0, (p))

#define REPLACE_FIELD(field, freefunc, newvalue) \
        { if ((field) != NULL) { freefunc(field); } (field) = (newvalue); }

#define FREE_FIELD(field, freefunc) \
        REPLACE_FIELD(field, freefunc, NULL)

#if defined(__cplusplus_cli) || !defined(_M_IX86)
#define htonll(n) _byteswap_ulong(n)
#else
__inline long long __declspec(naked)
htonll()
{
    __asm
    {
        mov eax, [esp + 4];
        mov edx, [esp + 8];
        bswap eax;
        bswap edx;
        xchg eax, edx;
        ret;
    }
}
#endif

#if (_MSC_VER >= 1400) && (defined(_M_IX86) || defined(_M_AMD64))
EXTERN_C void __cpuid(int a[4], int b);

#pragma intrinsic(__cpuid, memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

#define MICROSOFT_HV_CPUID "Microsoft Hv"

__forceinline BOOL
IsHyperV()
{
    struct
    {
        int highest_leaf;
        char vendor[12];
    } cpuinfo = { 0 };
    __cpuid((int*)&cpuinfo, 0x40000000);
    return memcmp(cpuinfo.vendor, MICROSOFT_HV_CPUID, sizeof(cpuinfo.vendor)) == 0;
}
#else
#pragma intrinsic(memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

__forceinline BOOL
IsHyperV()
{
    // Obviously not correct, but for now, just to make ported code compile.
    // Might find a way to detect this on ARM CPU etc in the future.
    return TRUE;
}
#endif

__forceinline size_t
StringLength(LPCWSTR string, size_t maxlen)
{
    size_t i = 0;

    while (i < maxlen && string[i] != 0)
    {
        ++i;
    }

    return i;
}

// Complement to the lstr*** API functions
__forceinline int
lstrncmpi(LPCTSTR lpString1, LPCTSTR lpString2, int iMaxLength)
{
    int iLength1 = lstrlen(lpString1);
    int iLength2 = lstrlen(lpString2);
    int iCompare = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE |
        SORT_STRINGSORT, lpString1,
        iLength1<iMaxLength ? iLength1 : iMaxLength,
        lpString2,
        iLength2<iMaxLength ? iLength2 : iMaxLength);

    return iCompare - 2;
}

/// <summary>
/// Sets a bit to 1 in a bit field.
/// </summary>
/// <param name="data">Bit field</param>
/// <param name="bitnumber">Bit number to set to 1</param>
__forceinline void
SetBit(PBYTE data, int bitnumber)
{
    data[bitnumber >> 3] |= 1 << ((~bitnumber) & 7);
}

/// <summary>
/// Sets a bit to 0 in a bit field.
/// </summary>
/// <param name="data">Bit field</param>
/// <param name="bitnumber">Bit number to set to 0</param>
__forceinline void
ClearBit(PBYTE data, int bitnumber)
{
    data[bitnumber >> 3] &= ~(1 << ((~bitnumber) & 7));
}

/// <summary>
/// Gets a bit from a bit field.
/// </summary>
/// <param name="data">Bit field</param>
/// <param name="bitnumber">Bit number to get/param>
/// <returns>True if value of specified bit is 1, false if 0.</returns>
__forceinline BOOL
GetBit(PBYTE data, int bitnumber)
{
    return (data[bitnumber >> 3] & (1 << ((~bitnumber) & 7))) != 0;
}

/// <summary>
 /// Finds greatest common divisor for two values.
 /// </summary>
 /// <param name="a">First value</param>
 /// <param name="b">Second value</param>
 /// <returns>Greatest common divisor for values a and b.</returns>
__forceinline __int64
GreatestCommonDivisor(__int64 a, __int64 b)
{
    while (b != 0)
    {
        __int64 temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/// <summary>
/// Finds greatest common divisor for a sequence of values.
/// </summary>
/// <param name="values">Sequence of values</param>
/// <returns>Greatest common divisor for values.</returns>
__forceinline __int64
GreatestCommonDivisorFromSequence(PLONGLONG values, int count)
{
    __int64 result;
    int i;

    if (count <= 0)
    {
        return 0;
    }
    if (count == 1)
    {
        return values[0];
    }

    result = values[0];

    for (i = 1; i < count; i++)
    {
        result = GreatestCommonDivisor(result, values[i]);
    }

    return result;
}

/// <summary>
/// Finds least common multiple for two values.
/// </summary>
/// <param name="a">First value</param>
/// <param name="b">Second value</param>
/// <returns>Least common multiple for values a and b.</returns>
__forceinline __int64
LeastCommonMultiple(__int64 a, __int64 b)
{
    return (a / GreatestCommonDivisor(a, b)) * b;
}

/// <summary>
/// Finds least common multiple for a sequence of values.
/// </summary>
/// <param name="values">Sequence of values</param>
/// <returns>Least common multiple for values.</returns>
__forceinline __int64
LeastCommonMultipleFromSequence(PLONGLONG values, int count)
{
    __int64 result;
    int i;

    if (count <= 0)
    {
        return 0;
    }
    if (count == 1)
    {
        return values[0];
    }

    result = values[0];

    for (i = 1; i < count; i++)
    {
        result = LeastCommonMultiple(result, values[i]);
    }

    return result;
}

/// <summary>
/// Returns a sequence of prime factors for a value.
/// </summary>
/// <param name="value">Value</param>
/// <returns>Sequence of prime factors</returns>
__forceinline void
PrimeFactors(__int64 value, void (*action)(__int64 prime))
{
    __int64 z = 2;

    while (z * z <= value)
    {
        if (value % z == 0)
        {
            action(z);
            value /= z;
        }
        else
        {
            z++;
        }
    }

    if (value > 1)
    {
        action(value);
    }
}

#ifdef  __cplusplus
extern "C" {
#endif
    extern
        int
        oem_printf_line_length;

    __forceinline
        void
        SetOemPrintFLineLength(HANDLE hConsole)
    {
        CONSOLE_SCREEN_BUFFER_INFO con_info;
        if (GetConsoleScreenBufferInfo(hConsole, &con_info))
        {
            oem_printf_line_length =
                con_info.srWindow.Right - con_info.srWindow.Left - 2;

            if (oem_printf_line_length > 250)
            {
                oem_printf_line_length = 250;
            }
        }
        else
        {
            oem_printf_line_length = 0;
        }
    }
#ifdef  __cplusplus
}
#endif

#ifdef _M_ARM

#ifdef CharToOemA
#undef CharToOemA
#endif

#define CharToOemA(s,t)

#ifdef MessageBoxA
#undef MessageBoxA
#endif
#ifdef MessageBoxW
#undef MessageBoxW
#endif

#define MessageBoxA ConsoleMessageA
#define MessageBoxW ConsoleMessageW

#endif

EXTERN_C
HMODULE
WINAPI
LoadRemoteLibrary(
    HANDLE process,
    LPCWSTR dllpath);

EXTERN_C
int
WINAPI
ConsoleMessageA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType);

EXTERN_C
int
WINAPI
ConsoleMessageW(
    HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType);

EXTERN_C
BOOL
oem_printf(FILE *stream, LPCSTR lpMessage, ...);

/* win_error
*/
__forceinline LPSTR
win_errmsgA(DWORD dwErrNo)
{
    LPSTR errmsg = NULL;
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwErrNo,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, "Error %1!#x!%n", 0, 0,
        (LPSTR)&errmsg, 0, (va_list*)&dwErrNo))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
__forceinline LPWSTR
win_errmsgW(DWORD dwErrNo)
{
    LPWSTR errmsg = NULL;
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwErrNo,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, L"Error %1!#x!%n", 0,
        0, (LPWSTR)&errmsg, 0, (va_list*)&dwErrNo))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
#ifdef UNICODE
#define win_errmsg win_errmsgW
#define win_error win_errmsgW(GetLastError())
#define h_error win_errmsgW(WSAGetLastError())
#else
#define win_errmsg win_errmsgA
#define win_error win_errmsgA(GetLastError())
#define h_error win_errmsgA(WSAGetLastError())
#endif

#define win_errorA win_errmsgA(GetLastError())
#define win_errorW win_errmsgW(GetLastError())
#define h_errorA win_errmsgA(WSAGetLastError())
#define h_errorW win_errmsgW(WSAGetLastError())

/* pdh_error
*/
__forceinline LPSTR pdh_errmsgA(DWORD dwErrorCode)
{
    LPSTR errmsg = NULL;
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        GetModuleHandle(PDH_MODULE), dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, "Error %1!#x!%n", 0, 0,
        (LPSTR)&errmsg, 0, (va_list*)&dwErrorCode))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
__forceinline LPWSTR pdh_errmsgW(DWORD dwErrorCode)
{
    LPWSTR errmsg = NULL;
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        GetModuleHandle(PDH_MODULE), dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, L"Error 0x%1!x!%n", 0,
        0, (LPWSTR)&errmsg, 0, (va_list*)&dwErrorCode))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
#ifdef UNICODE
#define pdh_errmsg pdh_errmsgW
#define pdh_error pdh_errmsgW(GetLastError())
#else
#define pdh_errmsg pdh_errmsgA
#define pdh_error pdh_errmsgA(GetLastError())
#endif

#define pdh_errorA pdh_errmsgA(GetLastError())
#define pdh_errorW pdh_errmsgW(GetLastError())

#ifdef _NTDDK_
/* nt_error
*/
__forceinline LPSTR nt_errmsgA(NTSTATUS dwErrorCode)
{
    LPSTR errmsg = NULL;
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        GetModuleHandle(NTDLL_MODULE), dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, "Error %1!#x!%n", 0, 0,
        (LPSTR)&errmsg, 0, (va_list*)&dwErrorCode))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
__forceinline LPWSTR nt_errmsgW(NTSTATUS dwErrorCode)
{
    LPWSTR errmsg = NULL;
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        GetModuleHandle(NTDLL_MODULE), dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errmsg, 0, NULL))
    {
        return errmsg;
    }
    else if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY, L"Error 0x%1!x!%n", 0,
        0, (LPWSTR)&errmsg, 0, (va_list*)&dwErrorCode))
    {
        return errmsg;
    }
    else
    {
        return NULL;
    }
}
#ifdef UNICODE
#define nt_errmsg nt_errmsgW
#else
#define nt_errmsg nt_errmsgA
#endif
#endif // _NTDDK_

// Use Borland compiler's internal functions
#ifdef __BORLANDC__

# define lstrcmpA   __strcmp__
# define lstrcmpW   __wcscmp__

# define lstrcpyA   __strcpy__
# define lstrcpyW   __wcscpy__

#endif

EXTERN_C LPSTR
CDECL
mprintfA(LPCSTR lpMsg, ...);

EXTERN_C LPWSTR
CDECL
mprintfW(LPCWSTR lpMsg, ...);

#ifdef UNICODE
#define mprintf mprintfW
#else
#define mprintf mprintfA
#endif

EXTERN_C LPSTR
WideToOemAlloc(LPCWSTR lpSrc);

EXTERN_C LPSTR
WideToByteAlloc(LPCWSTR lpSrc);

EXTERN_C LPWSTR
OemToWideAlloc(LPCSTR lpSrc);

EXTERN_C LPWSTR
ByteToWideAlloc(LPCSTR lpSrc);

__forceinline int
wcscnt(LPCWSTR lp, WCHAR c)
{
    int o = 0;
    while ((lp = wcschr(lp, c)) != NULL)
    {
        lp++;
        o++;
    }
    return o;
}

// This function enables backup and restore privileges for this process.
__forceinline
BOOL
EnableBackupPrivileges()
{
    LPCTSTR privileges[] = {
        SE_BACKUP_NAME,
        SE_RESTORE_NAME,
        SE_SECURITY_NAME,
        SE_MANAGE_VOLUME_NAME,
        SE_DEBUG_NAME
    };

    BYTE buf[FIELD_OFFSET(TOKEN_PRIVILEGES, Privileges) +
        _countof(privileges) * sizeof(LUID_AND_ATTRIBUTES)] = { 0 };

    PTOKEN_PRIVILEGES tkp = (PTOKEN_PRIVILEGES)buf;

    HANDLE hToken;

    BOOL result;

    int i;

    for (i = 0; i < _countof(privileges); i++)
    {
        if (!LookupPrivilegeValue(NULL, privileges[i], &tkp->Privileges[i].Luid))
        {
            return FALSE;
        }

        tkp->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

    tkp->PrivilegeCount = _countof(privileges);

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    result = AdjustTokenPrivileges(hToken, FALSE, tkp, sizeof buf, NULL, NULL);

    CloseHandle(hToken);

    return result;
}

// Win32 version of kbhit()
#define lkbhit()           (WaitForSingleObject(hStdIn,0)==WAIT_OBJECT_0?1:0)

#ifdef __cplusplus

#include <winstrct.hpp>

#else /*_cplusplus*/

// Do the best possible if not C++

/* win_perror()
*
* Used similar to perror() in Unix environments.
*
* Used to print to stderr the error message associated with the error code of
* the most recently non-sucessful call to a Windows API function. The error
* message buffer is only used internally and is automatically freed by this
* function.
*/
void win_perrorA(LPCSTR __errmsg);
void win_perrorW(LPCWSTR __errmsg);
#ifdef UNICODE
#define win_perror win_perrorW
#else
#define win_perror win_perrorA
#endif

/* h_perror()
*
*/
void h_perrorA(LPCSTR __errmsg);
void h_perrorW(LPCWSTR __errmsg);
#ifdef UNICODE
#define h_perror h_perrorW
#else
#define h_perror h_perrorA
#endif

/* pdh_perror()
*
*/
void pdh_perrorA(LPCSTR __errmsg);
void pdh_perrorW(LPCWSTR __errmsg);
#ifdef UNICODE
#define pdh_perror pdh_perrorW
#else
#define pdh_perror pdh_perrorA
#endif

#ifdef _NTDDK_
/* nt_perror()
*
*/
void nt_perrorA(NTSTATUS status, LPCSTR __errmsg);
void nt_perrorW(NTSTATUS status, LPCWSTR __errmsg);
#ifdef UNICODE
#define nt_perror nt_perrorW
#else
#define nt_perror nt_perrorA
#endif
#endif // _NTDDK_

#endif /*_cplusplus*/

VOID WINAPI s_perror(LPCSTR szMsg, int iErrNo);

#endif //  !_INC_WINSTRCT_
