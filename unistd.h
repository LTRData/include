#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <process.h>
#include <sys/_types.h>

int msleep(unsigned long microseconds);

#define usleep(usec)    msleep(usec/1000)
#define sleep(sec)      ((unsigned long)msleep(sec*1000))
#define ftruncate       chsize

typedef intptr_t        ssize_t;
typedef int             mode_t;

#define S_IRUSR         _S_IREAD
#define S_IWUSR         _S_IWRITE

#define F_OK            0

#define O_NONBLOCK      0

__forceinline
int pread(int d, void* buf, int nbytes, __int64 offset)
{
    int result = 0;

    if (_lseeki64(d, offset, SEEK_SET) != offset)
    {
        return -1;
    }

    while (nbytes > 0)
    {
        int rc = _read(d, buf, nbytes);

        if (rc < 0)
        {
            return rc;
        }

        if (rc == 0)
        {
            break;
        }

        nbytes -= rc;
        result += rc;
        buf = (char*)buf + rc;
    }

    return result;
}

__forceinline
int pwrite(int d, const void* buf, int nbytes, __int64 offset)
{
    int rc;

    if (_lseeki64(d, offset, SEEK_SET) != offset)
    {
        return -1;
    }

    rc = _write(d, buf, nbytes);

    if ((rc >= 0) || (errno != 28))
    {
        return rc;
    }

    fprintf(stderr, "Extending file to %I64u bytes.\n", offset + nbytes);

#if defined(_M_IX86)
    {
        HANDLE h = (HANDLE)_get_osfhandle(d);
        LARGE_INTEGER eof;
        eof.QuadPart = offset + nbytes;

        if (SetFilePointer(h, eof.LowPart, &eof.HighPart, FILE_BEGIN) ==
            INVALID_SET_FILE_POINTER &&
            GetLastError() != NO_ERROR)
        {
            errno = 28;
            return -1;
        }

        if (!SetEndOfFile(h))
        {
            errno = 28;
            return -1;
        }
    }
#elif defined(_M_X64)
    {
        HANDLE h = (HANDLE)_get_osfhandle(d);
        LARGE_INTEGER eof;
        eof.QuadPart = offset + nbytes;

        if (!SetFilePointerEx(h, eof, NULL, FILE_BEGIN))
        {
            errno = 28;
            return -1;
        }

        if (!SetEndOfFile(h))
        {
            errno = 28;
            return -1;
        }
    }
#else
    errno = _chsize_s(d, offset + nbytes);

    if (errno != 0)
    {
        return -1;
    }
#endif

    if (_lseeki64(d, offset, SEEK_SET) != offset)
    {
        return -1;
    }

    return _write(d, buf, nbytes);
}
