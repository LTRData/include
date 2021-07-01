#if !defined(_INC_WNTFIND_) && defined(__cplusplus)
#define _INC_WNTFIND_

#include <wtime.h>

#pragma warning(push)
#pragma warning(disable:4355)

/**
   WNativeFileFinder class. Encapsulation of NT API NtQueryDirectoryFile to
   enumerate files in a directory.
   */
class WNativeFileFinder : public FILE_ID_BOTH_DIR_INFORMATION
{
private:
    WCHAR FileNameBuffer[UNICODE_STRING_MAX_CHARS];

    /// Starts find with using name pattern.
    bool First(LPCWSTR Directory, LPCWSTR Pattern)
    {
        hFind = CreateFile(
            Directory,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        UNICODE_STRING pattern;
        RtlInitUnicodeString(&pattern, Pattern);

        IO_STATUS_BLOCK io_status;

        NTSTATUS status = NtQueryDirectoryFile(hFind, NULL, NULL,
            NULL, &io_status, this,
            FIELD_OFFSET(WNativeFileFinder, hFind),
            FileIdBothDirectoryInformation, TRUE,
            Pattern != NULL ? &pattern : NULL, TRUE);

        if (!NT_SUCCESS(status))
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
            SetLastError(RtlNtStatusToDosError(status));
            return FALSE;
        }

        FileName[FileNameLength >> 1] = 0;

        return true;
    }

protected:
    /// Open find handle used in internal API calls.
    HANDLE hFind;

public:
    /// cast to bool returns status of object.
    operator bool() const
    {
        return hFind != INVALID_HANDLE_VALUE;
    }

    /// Starts find with using name pattern.
    WNativeFileFinder(
        LPCWSTR Directory,
        LPCWSTR Pattern    // Specify file name pattern, or NULL for any names
    )
    {
        First(Directory, Pattern);
    }

    /// Restart find with new file name pattern.
    WNativeFileFinder& operator =(LPCTSTR lpFileName)
    {
        if (hFind != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }

        First(lpFileName, NULL);

        return *this;
    }

    /// Closes the find handle.
    ~WNativeFileFinder()
    {
        if (hFind != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

    /// Finds next matching file.
    bool Next()
    {
        IO_STATUS_BLOCK io_status;

        NTSTATUS status = NtQueryDirectoryFile(hFind, NULL, NULL,
            NULL, &io_status, this,
            FIELD_OFFSET(WNativeFileFinder, hFind),
            FileIdBothDirectoryInformation, TRUE, NULL, FALSE);

        if (!NT_SUCCESS(status))
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
            SetLastError(RtlNtStatusToDosError(status));
            return false;
        }

        FileName[FileNameLength >> 1] = 0;

        return true;
    }
};

/// WFilteredNativeFileFinder adds filtered search functionality to WNativeFileFinder.
class WFilteredNativeFileFinder : public WNativeFileFinder
{
protected:
    /// Checks found file against filter.
    bool Filter()
    {
        if (!(FileAttributes & dwFileAttrMask) &&
            !(~FileAttributes & dwFileAttrRequired) &&
            (CreationTime.QuadPart >= ftCreationTime1) &&
            (CreationTime.QuadPart <= ftCreationTime2) &&
            (LastAccessTime.QuadPart >= ftLastAccessTime1) &&
            (LastAccessTime.QuadPart <= ftLastAccessTime2) &&
            (LastWriteTime.QuadPart >= ftLastWriteTime1) &&
            (LastWriteTime.QuadPart <= ftLastWriteTime2) &&
            (EndOfFile.QuadPart >= qwMinSize) &&
            (EndOfFile.QuadPart <= qwMaxSize))
        {
            return true;
        }
        else
        {
            if (!WNativeFileFinder::Next())  // Find next if this didn't match
            {
                return false;
            }

#ifdef RECURSE_SLEEP
            RECURSE_SLEEP;
#endif
            return Filter();
        }
    }

public:
    /// Specifies attributes files must not have.
    DWORD dwFileAttrMask;
    /// Specify attributes files must have
    DWORD dwFileAttrRequired;
    LONGLONG qwMinSize;                      // File size interval
    LONGLONG qwMaxSize;
    LONGLONG ftLastWriteTime1;  // Last write time interval
    LONGLONG ftLastWriteTime2;
    LONGLONG ftLastAccessTime1; // Last access time interval
    LONGLONG ftLastAccessTime2;
    LONGLONG ftCreationTime1;   // Creation time interval
    LONGLONG ftCreationTime2;

    /// Overridden operator=() with filter function.
    WFilteredNativeFileFinder& operator =(LPCTSTR lpFileName)
    {
        WNativeFileFinder::operator=(lpFileName);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            return *this;
        }

        if (!Filter() && hFind != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }

        return *this;
    }

    /// Begin find with file name pattern and initialize filter data.
    WFilteredNativeFileFinder(
        LPCWSTR Directory,
        LPCWSTR Pattern,    // Specify file name pattern, or NULL for any names
        DWORD dwSetFileAttrMask = 0,     // Specify attributes not to find
        DWORD dwSetFileAttrRequired = 0,   // Specify attributes to find
        LONGLONG qwSetMinSize = 0,                      // File size interval
        LONGLONG qwSetMaxSize = LONGLONG_MAX,
        WFileTime ftSetLastWriteTime1 = 0LL,  // Last write time interval
        WFileTime ftSetLastWriteTime2 = LONGLONG_MAX,
        WFileTime ftSetLastAccessTime1 = 0LL, // Last access time interval
        WFileTime ftSetLastAccessTime2 = LONGLONG_MAX,
        WFileTime ftSetCreationTime1 = 0LL,   // Creation time interval
        WFileTime ftSetCreationTime2 = LONGLONG_MAX
    )
        : WNativeFileFinder(Directory, Pattern), dwFileAttrMask(dwSetFileAttrMask),
        dwFileAttrRequired(dwSetFileAttrRequired), qwMinSize(qwSetMinSize),
        qwMaxSize(qwSetMaxSize), ftLastWriteTime1(ftSetLastWriteTime1),
        ftLastWriteTime2(ftSetLastWriteTime2),
        ftLastAccessTime1(ftSetLastAccessTime1),
        ftLastAccessTime2(ftSetLastAccessTime2),
        ftCreationTime1(ftSetCreationTime1), ftCreationTime2(ftSetCreationTime2)
    {
        if (hFind == INVALID_HANDLE_VALUE)
            return;

        // The only way we can report error condition is by closing handle and
        // set hFind invalid. This makes bool operator return false, so caller
        // can check if anything found.
        if (!Filter() && hFind != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

    /// Find next matching file.
    BOOL Next()
    {
        return WNativeFileFinder::Next() && Filter();
    }
};

#pragma warning(pop)

#endif // _INC_WNTFIND_

