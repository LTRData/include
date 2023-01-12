#ifndef _INC_WTIME_
#define _INC_WTIME_

#include <windows.h>
#include <stdio.h>

// 1/10 µs between 1 Jan 1601 and 1 Jan 1900.
#define FILETIME_19000101 (94354848000000000)

#ifdef _M_ARM

#ifdef CharToOemA
#undef CharToOemA
#endif

#define CharToOemA(s,t)

#endif

#ifdef __cplusplus

// Enhanced FILETIME structure with encapsulated API functions,
// compare operators and type conversions
struct WFileTime : public FILETIME
{
    // Get current time function
    void GetUTC()
    {
        GetSystemTimeAsFileTime(this);
    }

    // Time compare operators
    bool operator ==(CONST WFileTime &wft) const
    {
        return !CompareFileTime(this, &wft);
    }

    bool operator >(CONST WFileTime &wft) const
    {
        return CompareFileTime(this, &wft) > 0;
    }

    bool operator >=(CONST WFileTime &wft) const
    {
        return CompareFileTime(this, &wft) >= 0;
    }

    bool operator <(CONST WFileTime &wft) const
    {
        return CompareFileTime(this, &wft) < 0;
    }

    bool operator <=(CONST WFileTime& wft) const
    {
        return CompareFileTime(this, &wft) <= 0;
    }

    WFileTime operator +(CONST WFileTime& wft)
    {
        return *(LONGLONG*)this + *(LONGLONG*)&wft;
    }

    WFileTime operator -(CONST WFileTime& wft)
    {
        return *(LONGLONG*)this - *(LONGLONG*)&wft;
    }

    WFileTime& operator +=(CONST WFileTime& wft)
    {
        *(LONGLONG*)this += *(LONGLONG*)&wft;
        return *this;
    }

    WFileTime& operator -=(CONST WFileTime& wft)
    {
        *(LONGLONG*)this -= *(LONGLONG*)&wft;
        return *this;
    }

    // Convert to DOS FAT date and time
    BOOL ToDOS(LPWORD lpFatDate, LPWORD lpFatTime) const
    {
        return FileTimeToDosDateTime(this, lpFatDate, lpFatTime);
    }

    BOOL ToLocal(LPFILETIME lpft) const
    {
        return FileTimeToLocalFileTime(this, lpft);
    }

    BOOL ToUTC(LPFILETIME lpft) const
    {
        return LocalFileTimeToFileTime(this, lpft);
    }

    // Type conversions
    operator DWORDLONG() const
    {
        return *(DWORDLONG*)this;
    }

    // Type conversions
    operator LONGLONG() const
    {
        return *(LONGLONG*)this;
    }

    // Constructors
    WFileTime(CONST FILETIME &ft)
    {
        *(FILETIME*)this = ft;
    }

    WFileTime(CONST SYSTEMTIME &st)
    {
        SystemTimeToFileTime(&st, this);
    }

    // Construct from QWORD value
    WFileTime(DWORDLONG qw)
    {
        *(DWORDLONG*)this = qw;
    }

    // Construct from QWORD value
    WFileTime(LONGLONG qw)
    {
        *(LONGLONG*)this = qw;
    }

    bool IsZero() const
    {
        return (dwLowDateTime == 0) && (dwHighDateTime == 0);
    }

    void PrintAbsoluteTime()
    {
        DWORDLONG qwTime = *(DWORDLONG*)this;
        double dblMilliseconds;
        DWORD dwSeconds;
        DWORD dwMinutes;
        DWORD dwHours;
        DWORD dwDays;
        DWORD dwWeeks;

        dwWeeks = (DWORD)(qwTime / 6048000000000); qwTime %= 6048000000000;
        dwDays = (DWORD)(qwTime / 864000000000); qwTime %= 864000000000;
        dwHours = (DWORD)(qwTime / 36000000000); qwTime %= 36000000000;
        dwMinutes = (DWORD)(qwTime / 600000000); qwTime %= 600000000;
        dwSeconds = (DWORD)(qwTime / 10000000); qwTime %= 10000000;
        dblMilliseconds = (double)qwTime / 10000;

        if (dwWeeks)
            printf("%u weeks, ", dwWeeks);

        if (dwDays)
            printf("%u days, ", dwDays);

        printf("%u:%.2u:%.2u %f",
            dwHours, dwMinutes, dwSeconds, dblMilliseconds);
    }

    // Construct from DOS FAT date and time
    WFileTime(WORD wFatDate, WORD wFatTime)
    {
        DosDateTimeToFileTime(wFatDate, wFatTime, this);
    }

    // Empty constructor
    WFileTime()
    {
        *(DWORDLONG*)this = 0;
    }
};

// Enhanced SYSTEMTIME structure with encapsulated API functions and type
// conversions
struct WSystemTime : public SYSTEMTIME
{
    // Get/set current time functions
    void GetLocal()
    {
        GetLocalTime(this);
    }

    void GetUTC()
    {
        GetSystemTime(this);
    }

    BOOL SetLocal() const
    {
        return SetLocalTime(this);
    }

    BOOL SetUTC() const
    {
        return SetSystemTime(this);
    }

    // Special for Windows NT
    BOOL GetUTCAsTzSpecificLocal(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
        LPSYSTEMTIME lpUniversalTime)
    {
        return SystemTimeToTzSpecificLocalTime(lpTimeZoneInformation,
            lpUniversalTime, this);
    }

    VOID CDECL PrintShortDate(FILE *strm = stdout)
    {
        char cBuf[160];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, this, NULL, cBuf, sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fputs(cBuf, strm);
        }
    }

    VOID CDECL PrintLongDate(FILE *strm = stdout)
    {
        char cBuf[160];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, this, NULL, cBuf,
            sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fputs(cBuf, strm);
        }
    }

    VOID CDECL PrintTime(FILE *strm = stdout)
    {
        char cBuf[160];
        if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, this, NULL, cBuf, sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fputs(cBuf, strm);
        }
    }

    VOID CDECL PrintShortDateAndTime(FILE *strm = stdout)
    {
        char cBuf[160];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, this, NULL, cBuf, sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fprintf(strm, "%s ", cBuf);
        }

        if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, this, NULL, cBuf, sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fputs(cBuf, strm);
        }
    }

    VOID CDECL PrintLongDateAndTime(FILE *strm = stdout)
    {
        char cBuf[160];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, this, NULL, cBuf,
            sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fprintf(strm, "%s ", cBuf);
        }

        if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, this, NULL, cBuf, sizeof cBuf))
        {
            CharToOemA(cBuf, cBuf);
            fputs(cBuf, strm);
        }
    }

    // Constructors
    WSystemTime(CONST SYSTEMTIME &st)
    {
        *(SYSTEMTIME*)this = st;
    }

    WSystemTime(CONST FILETIME &ft)
    {
        FileTimeToSystemTime(&ft, this);
    }

    WSystemTime()
    {
        ZeroMemory(this, sizeof *this);
    }
};

// Enhanced TIME_ZONE_INFORMATION structure with encapsulated API functions
struct WTimeZoneInformation : public TIME_ZONE_INFORMATION
{
    operator TIME_ZONE_INFORMATION() const
    {
        return *(TIME_ZONE_INFORMATION*)this;
    }

    DWORD Get()
    {
        return GetTimeZoneInformation(this);
    }

    BOOL Set() const
    {
        return SetTimeZoneInformation(this);
    }
};

#else /* _cplusplus */

EXTERN_C VOID CDECL PrintShortDate(SYSTEMTIME*);
EXTERN_C VOID CDECL PrintLongDate(SYSTEMTIME*);
EXTERN_C VOID CDECL PrintTime(SYSTEMTIME*);
EXTERN_C VOID CDECL PrintShortDateAndTime(SYSTEMTIME*);
EXTERN_C VOID CDECL PrintLongDateAndTime(SYSTEMTIME*);
EXTERN_C VOID CDECL PrintAbsoluteTime(DWORDLONG);

#endif

#endif
