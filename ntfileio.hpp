#ifndef NtFileFinderBaseClassId
typedef FILE_BOTH_DIR_INFORMATION NtFileFinderBaseClass;
#define NtFileFinderBaseClassId FileBothDirectoryInformation
#endif

inline
VOID
InitCountedUnicodeString(PUNICODE_STRING Destination,
PWSTR Source,
USHORT SourceSize)
{
    Destination->Length = SourceSize;
    Destination->MaximumLength = SourceSize;
    Destination->Buffer = Source;
}

inline
HANDLE
NativeOpenFile(IN HANDLE RootDirectory,
IN PUNICODE_STRING Name,
IN ACCESS_MASK DesiredAccess,
IN ULONG ObjectOpenAttributes,
IN ULONG ShareAccess,
IN ULONG OpenOptions)
{
    // If "File" is current directory marker ".", we need to pass just
    // root directory and an empty name to directory create routines
    UNICODE_STRING name = { 0 };
    if ((Name->Length != 2) || (Name->Buffer[0] != L'.'))
        name = *Name;

    OBJECT_ATTRIBUTES object_attributes;

    InitializeObjectAttributes(&object_attributes,
        &name,
        ObjectOpenAttributes,
        RootDirectory,
        NULL);

    IO_STATUS_BLOCK io_status;
    HANDLE handle;

    NTSTATUS status =
        NtOpenFile(&handle,
        DesiredAccess | SYNCHRONIZE,
        &object_attributes,
        &io_status,
        ShareAccess,
        OpenOptions |
        FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status))
    {
        SetLastError(RtlNtStatusToDosError(status));
        return INVALID_HANDLE_VALUE;
    }

    return handle;
}

inline
BOOL
NativeSetFileAttributes(IN HANDLE FileHandle,
IN ULONG FileAttributes)
{
    FILE_BASIC_INFORMATION basic_info = { 0 };
    IO_STATUS_BLOCK io_status;
    NTSTATUS status;

    basic_info.FileAttributes = (FileAttributes & 0x23127) | 0x80;

    status =
        NtSetInformationFile(FileHandle,
        &io_status,
        &basic_info,
        sizeof basic_info,
        FileBasicInformation);

    if (NT_SUCCESS(status))
        return TRUE;

    SetLastError(RtlNtStatusToDosError(status));
    return FALSE;
}

inline
BOOL
NativeSetFileAttributes(IN HANDLE RootDirectory,
IN PUNICODE_STRING File,
IN ULONG FileAttributes)
{
    FILE_BASIC_INFORMATION basic_info = { 0 };
    IO_STATUS_BLOCK io_status;
    NTSTATUS status;

    HANDLE hFile =
        NativeOpenFile(RootDirectory,
        File,
        FILE_WRITE_ATTRIBUTES,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE |
        FILE_SHARE_DELETE,
        FILE_OPEN_FOR_BACKUP_INTENT);
    
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    basic_info.FileAttributes = (FileAttributes & 0x23127) | 0x80;

    status =
        NtSetInformationFile(hFile,
        &io_status,
        &basic_info,
        sizeof basic_info,
        FileBasicInformation);

    NtClose(hFile);

    if (NT_SUCCESS(status))
        return TRUE;

    SetLastError(RtlNtStatusToDosError(status));
    return FALSE;
}

// This function creates the specified directory, optionally by creating any
// directories necessary above it in the tree. It works much like
// MakeSureDirectoryPathExists() in the image helper library but this is a
// Unicode function using native API.
HANDLE
NativeCreateDirectory(IN HANDLE RootDirectory,
IN PUNICODE_STRING Name,
IN ACCESS_MASK DesiredAccess,
IN ULONG ObjectOpenAttributes,
OUT PULONG_PTR CreationResult OPTIONAL,
IN ULONG FileAttributes,
IN ULONG ShareAccess,
IN ULONG CreateOptions,
IN BOOL CreateParentDirectories);

HANDLE
NativeCreateFile(IN HANDLE RootDirectory,
IN PUNICODE_STRING Name,
IN ACCESS_MASK DesiredAccess,
IN ULONG ObjectOpenAttributes,
OUT PULONG_PTR CreationResult OPTIONAL,
IN PLARGE_INTEGER AllocationSize OPTIONAL,
IN ULONG FileAttributes,
IN ULONG ShareAccess,
IN ULONG CreateDisposition,
IN ULONG CreateOptions,
IN BOOL CreateParentDirectories);

inline
BOOL
NativeDeleteFile(HANDLE Handle)
{
    IO_STATUS_BLOCK io_status;

    FILE_DISPOSITION_INFORMATION info = { TRUE };

    NTSTATUS status =
        NtSetInformationFile(Handle,
        &io_status,
        &info,
        sizeof(info),
        FileDispositionInformation);

    if (!NT_SUCCESS(status))
    {
        SetLastError(RtlNtStatusToDosError(status));
        return FALSE;
    }

    return TRUE;
}

inline
void
SplitPath(const PUNICODE_STRING Path,
PUNICODE_STRING Directory,
PUNICODE_STRING File OPTIONAL)
{
    for (*Directory = *Path;
        Directory->Length >= 2;
        Directory->Length -= 2)
        if (Directory->Buffer[(Directory->Length >> 1) - 1] == L'\\')
            break;

    if (File != NULL)
    {
        File->Length = (USHORT)(Path->Length - Directory->Length);
        File->MaximumLength = (USHORT)(Path->MaximumLength - Directory->Length);
        File->Buffer = (PWSTR)(((PUCHAR)Path->Buffer) + Directory->Length);
    }
}

class NtFileFinder
{

public:
    
    union
    {
        NtFileFinderBaseClass Base;
        struct
        {
            BYTE Padding[FIELD_OFFSET(NtFileFinderBaseClass, FileName)];
            WCHAR FileName[UNICODE_STRING_MAX_CHARS];
        };
    };

private:

    NTSTATUS LastStatus;

public:

    /// cast to bool returns status of object.
    operator bool() const
    {
        return NT_SUCCESS(LastStatus);
    }

    DWORD GetLastWin32Error() const
    {
        return RtlNtStatusToDosError(LastStatus);
    }

    NTSTATUS GetLastStatus() const
    {
        return LastStatus;
    }

    /// Finds first matching file.
    bool FindFirst(HANDLE DirHandle)
    {
        return FindFirst(DirHandle, NULL);
    }

    /// Finds first matching file.
    bool FindFirst(HANDLE DirHandle, PUNICODE_STRING FilePattern)
    {
        IO_STATUS_BLOCK IoStatusBlock;

        LastStatus =
            NtQueryDirectoryFile(DirHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                this,
                FIELD_OFFSET(NtFileFinderBaseClass, FileName) +
                sizeof(FileName),
                NtFileFinderBaseClassId,
                TRUE,
                FilePattern,
                TRUE);

        return NT_SUCCESS(LastStatus);
    }

    /// Finds next matching file.
    bool FindNext(HANDLE DirHandle)
    {
        IO_STATUS_BLOCK IoStatusBlock;

        LastStatus =
            NtQueryDirectoryFile(DirHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                this,
                FIELD_OFFSET(NtFileFinderBaseClass, FileName) +
                sizeof(FileName),
                NtFileFinderBaseClassId,
                TRUE,
                NULL,
                FALSE);

        return NT_SUCCESS(LastStatus);
    }
};

/// NtFilteredFileFinder adds filtered search functionality to NtFileFinder.
class NtFilteredFileFinder : public NtFileFinder
{
protected:
    /// Checks found file against filter.
    bool Filter(HANDLE DirHandle)
    {
        for (;;)
            if (!(Base.FileAttributes & dwFileAttrMask) &
                !(~Base.FileAttributes & dwFileAttrRequired) &
                (Base.CreationTime.QuadPart >= ftCreationTime1) &
                (Base.CreationTime.QuadPart <= ftCreationTime2) &
                (Base.LastAccessTime.QuadPart >= ftLastAccessTime1) &
                (Base.LastAccessTime.QuadPart <= ftLastAccessTime2) &
                (Base.LastWriteTime.QuadPart >= ftLastWriteTime1) &
                (Base.LastWriteTime.QuadPart <= ftLastWriteTime2) &
                (Base.EndOfFile.QuadPart >= qwMinSize) &
                (Base.EndOfFile.QuadPart <= qwMaxSize))
                return true;
            else
                // Find next if this didn't match
                if (!NtFileFinder::FindNext(DirHandle))
                    return false;
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

    /// Initialize filter data.
    NtFilteredFileFinder
        (DWORD dwSetFileAttrMask = 0, // Specify attributes not to find
        DWORD dwSetFileAttrRequired = 0, // Specify attributes to find
        LONGLONG qwSetMinSize = 0,            // File size interval
        LONGLONG qwSetMaxSize = LONGLONG_MAX,
        LONGLONG ftSetLastWriteTime1 = 0, // Last write time interval
        LONGLONG ftSetLastWriteTime2 = LONGLONG_MAX,
        LONGLONG ftSetLastAccessTime1 = 0, // Last access time intvl
        LONGLONG ftSetLastAccessTime2 = LONGLONG_MAX,
        LONGLONG ftSetCreationTime1 = 0,   // Creation time interval
        LONGLONG ftSetCreationTime2 = LONGLONG_MAX)
        : dwFileAttrMask(dwSetFileAttrMask),
        dwFileAttrRequired(dwSetFileAttrRequired),
        qwMinSize(qwSetMinSize),
        qwMaxSize(qwSetMaxSize),
        ftLastWriteTime1(ftSetLastWriteTime1),
        ftLastWriteTime2(ftSetLastWriteTime2),
        ftLastAccessTime1(ftSetLastAccessTime1),
        ftLastAccessTime2(ftSetLastAccessTime2),
        ftCreationTime1(ftSetCreationTime1),
        ftCreationTime2(ftSetCreationTime2)
    {
    }

    /// Begin find with file name pattern
    bool FindFirst(HANDLE DirHandle,
        PUNICODE_STRING FilePattern)
    {
        if (!NtFileFinder::FindFirst(DirHandle, FilePattern))
            return false;

        return Filter(DirHandle);
    }

    /// Find next matching file.
    bool FindNext(HANDLE DirHandle)
    {
        if (!NtFileFinder::FindNext(DirHandle))
            return false;

        return Filter(DirHandle);
    }
};

