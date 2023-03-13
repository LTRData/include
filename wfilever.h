#ifndef _INC_WFILEVER_
#define _INC_WFILEVER_

#include <winstrct.h>

inline
LPVOID
WINAPI
GetModuleVersionResource(HMODULE hmodule)
{
    HRSRC res = FindResource(hmodule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);

    if (res == NULL)
    {
        return NULL;
    }

    HGLOBAL resmem = LoadResource(hmodule, res);

    if (resmem == NULL)
    {
        return NULL;
    }

    return LockResource(resmem);
}

inline
LPVOID
WINAPI
GetRawFileVersionResource(LPVOID FileData, LPDWORD ResourceSize)
{
    PIMAGE_NT_HEADERS header = ImageNtHeader(FileData);

    if (header == NULL || header->Signature != 0x4550 || header->FileHeader.SizeOfOptionalHeader == 0)
    {
        SetLastError(ERROR_RESOURCE_TYPE_NOT_FOUND);
        return NULL;
    }

    PIMAGE_DATA_DIRECTORY resource_header = NULL;

    if (header->FileHeader.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32))
    {
        PIMAGE_OPTIONAL_HEADER32 optional_header = (PIMAGE_OPTIONAL_HEADER32)&header->OptionalHeader;
        resource_header = &optional_header->DataDirectory[2];
    }
    else if (header->FileHeader.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64))
    {
        PIMAGE_OPTIONAL_HEADER64 optional_header = (PIMAGE_OPTIONAL_HEADER64)&header->OptionalHeader;
        resource_header = &optional_header->DataDirectory[2];
    }
    else
    {
        SetLastError(ERROR_RESOURCE_TYPE_NOT_FOUND);
        return NULL;
    }

    PIMAGE_SECTION_HEADER section_table = (PIMAGE_SECTION_HEADER)((LPBYTE)&header->OptionalHeader + header->FileHeader.SizeOfOptionalHeader);
    PIMAGE_SECTION_HEADER section_header = NULL;

    for (int i = 0; i < header->FileHeader.NumberOfSections; i++)
    {
        if (strcmp((const char*)section_table[i].Name, ".rsrc") != 0)
        {
            continue;
        }

        section_header = section_table + i;
        break;
    }

    if (section_header == NULL)
    {
        SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
        return NULL;
    }

    LPBYTE raw = (LPBYTE)FileData + section_header->PointerToRawData;

    PIMAGE_RESOURCE_DIRECTORY resource_section = (PIMAGE_RESOURCE_DIRECTORY)(raw + (resource_header->VirtualAddress - section_header->VirtualAddress));
    PIMAGE_RESOURCE_DIRECTORY_ENTRY resource_dir_entry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(resource_section + 1);

    for (int i = 0; i < resource_section->NumberOfNamedEntries + resource_section->NumberOfIdEntries; i++)
    {
        if (!resource_dir_entry[i].NameIsString &&
            MAKEINTRESOURCE(resource_dir_entry[i].Id) == RT_VERSION &&
            resource_dir_entry[i].DataIsDirectory)
        {
            PIMAGE_RESOURCE_DIRECTORY_ENTRY found_entry = resource_dir_entry + i;
            PIMAGE_RESOURCE_DIRECTORY found_dir = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)resource_section + found_entry->OffsetToDirectory);

            if ((found_dir->NumberOfIdEntries + found_dir->NumberOfNamedEntries) == 0)
            {
                continue;
            }

            PIMAGE_RESOURCE_DIRECTORY_ENTRY found_dir_entry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(found_dir + 1);

            for (int j = 0; j < found_dir->NumberOfNamedEntries + found_dir->NumberOfIdEntries; j++)
            {
                if (!found_dir_entry[j].DataIsDirectory)
                {
                    continue;
                }

                PIMAGE_RESOURCE_DIRECTORY found_subdir = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)resource_section + found_dir_entry->OffsetToDirectory);

                if ((found_subdir->NumberOfIdEntries + found_subdir->NumberOfNamedEntries) == 0)
                {
                    continue;
                }

                PIMAGE_RESOURCE_DIRECTORY_ENTRY found_subdir_entry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(found_subdir + 1);

                if (found_subdir_entry->DataIsDirectory)
                {
                    continue;
                }

                PIMAGE_RESOURCE_DATA_ENTRY found_data_entry = (PIMAGE_RESOURCE_DATA_ENTRY)((LPBYTE)resource_section + found_subdir_entry->OffsetToData);

                typedef struct {
                    WORD             wLength;
                    WORD             wValueLength;
                    WORD             wType;
                    WCHAR            szKey[_countof(L"VS_VERSION_INFO")];
                    WORD             Padding1[1];
                    VS_FIXEDFILEINFO FixedFileInfo;
                } VS_VERSIONINFO, * LPVS_VERSIONINFO;

                LPVS_VERSIONINFO resptr = (LPVS_VERSIONINFO)(raw + (found_data_entry->OffsetToData - section_header->VirtualAddress));

                if (resptr->wType != 0 ||
                    memcmp(resptr->szKey, L"VS_VERSION_INFO", sizeof(resptr->szKey)) != 0 ||
                    resptr->FixedFileInfo.dwSignature != 0xFEEF04BD)
                {
                    SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
                    return NULL;
                }

                if (ResourceSize != NULL)
                {
                    *ResourceSize = found_data_entry->Size;
                }

                return resptr;
            }
        }
    }

    SetLastError(ERROR_RESOURCE_TYPE_NOT_FOUND);
    return NULL;
}

inline
VS_FIXEDFILEINFO*
WINAPI
GetFixedVersionInfo(LPVOID VersionResource)
{
    typedef struct {
        WORD             wLength;
        WORD             wValueLength;
        WORD             wType;
        WCHAR            szKey[_countof(L"VS_VERSION_INFO")];
        WORD             Padding1[1];
        VS_FIXEDFILEINFO FixedFileInfo;
    } VS_VERSIONINFO, * LPVS_VERSIONINFO;

    LPVS_VERSIONINFO resptr = (LPVS_VERSIONINFO)VersionResource;

    if (resptr == NULL ||
        resptr->wType != 0 ||
        memcmp(resptr->szKey, L"VS_VERSION_INFO", sizeof(resptr->szKey)) != 0 ||
        resptr->FixedFileInfo.dwSignature != 0xFEEF04BD)
    {
        SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
        return NULL;
    }

    return &resptr->FixedFileInfo;
}

inline
VS_FIXEDFILEINFO*
WINAPI
GetModuleVersionInfo(HMODULE hmodule)
{
    LPVOID resource = GetModuleVersionResource(hmodule);

    if (resource == NULL)
    {
        return NULL;
    }

    return GetFixedVersionInfo(resource);
}

class WFileVerInfo : public WHeapMem<VOID>
{
public:
    explicit WFileVerInfo(LPCSTR lpstrFileName)
    {
        DWORD dwVerFileHandle;
        DWORD dwVerInfoSize = GetFileVersionInfoSizeA(lpstrFileName,
            &dwVerFileHandle);

        if (!dwVerInfoSize)
            return;

        ReAlloc(dwVerInfoSize);

        if (!*this)
            return;

        if (GetFileVersionInfoA(lpstrFileName, dwVerFileHandle, dwVerInfoSize,
            ptr))
        {
            return;
        }

        Free();

        return;
    }

    explicit WFileVerInfo(LPCWSTR lpstrFileName)
    {
        DWORD dwVerFileHandle;
        DWORD dwVerInfoSize = GetFileVersionInfoSizeW(lpstrFileName,
            &dwVerFileHandle);

        if (!dwVerInfoSize)
            return;

        ReAlloc(dwVerInfoSize);

        if (!*this)
            return;

        if (GetFileVersionInfoW(lpstrFileName, dwVerFileHandle, dwVerInfoSize,
            ptr))
        {
            return;
        }

        Free();

        return;
    }

    WFileVerInfo()
    {
        TCHAR lpstrFileName[MAX_PATH];
        if (!GetModuleFileName(NULL, lpstrFileName,
            sizeof(lpstrFileName) / sizeof(*lpstrFileName)))
            return;

        DWORD dwVerFileHandle;
        DWORD dwVerInfoSize = GetFileVersionInfoSize(lpstrFileName,
            &dwVerFileHandle);
        if (!dwVerInfoSize)
            return;

        ReAlloc(dwVerInfoSize);

        if (!*this)
            return;

        if (GetFileVersionInfo(lpstrFileName, dwVerFileHandle, dwVerInfoSize,
            ptr))
        {
            return;
        }

        Free();

        return;
    }

    LPTSTR QueryValue() const
    {
        return QueryValue((LPCTSTR)NULL);
    }

    LPSTR QueryValue(LPCSTR lpSubBlock) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return NULL;
        }

        if (!lpSubBlock)
            lpSubBlock = "\\StringFileInfo\\040904E4\\FileDescription";

        LPSTR lpVerBuf = NULL;
        UINT uiVerBufSiz;
        if (!VerQueryValueA(ptr, lpSubBlock, (LPVOID*)&lpVerBuf,
            &uiVerBufSiz))
        {
            return NULL;
        }
        else
        {
            return lpVerBuf;
        }
    }

    LPWSTR QueryValue(LPCWSTR lpSubBlock) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return NULL;
        }

        if (!lpSubBlock)
            lpSubBlock = L"\\StringFileInfo\\040904E4\\FileDescription";

        LPWSTR lpVerBuf = NULL;
        UINT uiVerBufSiz;
        if (!VerQueryValueW(ptr, lpSubBlock, (LPVOID*)&lpVerBuf,
            &uiVerBufSiz))
        {
            return NULL;
        }
        else
        {
            return lpVerBuf;
        }
    }

    LPSTR QueryValueWithTranslation(LPCSTR strRecordName = NULL,
        LPCSTR strRecordDescription = NULL,
        DWORD dwTranslationCode = (DWORD)-1) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return NULL;
        }

        if (!strRecordDescription)
            strRecordDescription = strRecordName;

        DWORD dwDefaultTranslationCode = 0x04E40409;
        LPWORD lpwTranslationCode = (LPWORD)&dwTranslationCode;
        if (dwTranslationCode == (DWORD)-1)
        {
            lpwTranslationCode =
                (LPWORD)QueryValue("\\VarFileInfo\\Translation");
            if (!lpwTranslationCode)
                lpwTranslationCode = (LPWORD)& dwDefaultTranslationCode;
        }

        CHAR strSubBlock[128];
        _snprintf(strSubBlock, sizeof(strSubBlock) / sizeof(strSubBlock[0]),
            "\\StringFileInfo\\%04X%04X\\%s",
            lpwTranslationCode[0], lpwTranslationCode[1],
            strRecordName ? strRecordName : "FileDescription");
        strSubBlock[sizeof(strSubBlock) / sizeof(strSubBlock[0]) - 1] = 0;

        return QueryValue(strSubBlock);
    }

    LPWSTR QueryValueWithTranslation(LPWSTR strRecordName = NULL,
        LPCWSTR strRecordDescription = NULL,
        DWORD dwTranslationCode = (DWORD)-1) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return NULL;
        }

        if (!strRecordDescription)
            strRecordDescription = strRecordName;

        DWORD dwDefaultTranslationCode = 0x04E40409;
        LPWORD lpwTranslationCode = (LPWORD)&dwTranslationCode;
        if (dwTranslationCode == (DWORD)-1)
        {
            lpwTranslationCode =
                (LPWORD)QueryValue(L"\\VarFileInfo\\Translation");
            if (!lpwTranslationCode)
                lpwTranslationCode = (LPWORD)& dwDefaultTranslationCode;
        }

        WCHAR strSubBlock[128];
        _snwprintf(strSubBlock, sizeof(strSubBlock) / sizeof(strSubBlock[0]),
            L"\\StringFileInfo\\%04X%04X\\%s",
            lpwTranslationCode[0], lpwTranslationCode[1],
            strRecordName ? strRecordName : L"FileDescription");
        strSubBlock[sizeof(strSubBlock) / sizeof(strSubBlock[0]) - 1] = 0;

        return QueryValue(strSubBlock);
    }

    bool PrintFileVerRecord(LPCSTR strRecordName = NULL,
        LPCSTR strRecordDescription = NULL,
        DWORD dwTranslationCode = (DWORD)-1) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return false;
        }

        if (!strRecordDescription)
            strRecordDescription = strRecordName;

        DWORD dwDefaultTranslationCode = 0x04E40409;
        LPWORD lpwTranslationCode = (LPWORD)&dwTranslationCode;
        if (dwTranslationCode == (DWORD)-1)
        {
            lpwTranslationCode =
                (LPWORD)QueryValue("\\VarFileInfo\\Translation");
            if (!lpwTranslationCode)
                lpwTranslationCode = (LPWORD)&dwDefaultTranslationCode;
        }

        CHAR strSubBlock[128];
        _snprintf(strSubBlock, sizeof(strSubBlock) / sizeof(strSubBlock[0]),
            "\\StringFileInfo\\%04X%04X\\%s",
            lpwTranslationCode[0], lpwTranslationCode[1],
            strRecordName ? strRecordName : "FileDescription");
        strSubBlock[sizeof(strSubBlock) / sizeof(strSubBlock[0]) - 1] = 0;

        LPSTR lpVerBuf = QueryValue(strSubBlock);
        if (lpVerBuf)
        {
            CharToOemA(lpVerBuf, lpVerBuf);

            printf("%s: %s\n",
                (strRecordDescription ? strRecordDescription :
                "File description"), lpVerBuf);
            return true;
        }
        else
            return false;
    }

    bool PrintFileVerRecord(LPCWSTR strRecordName = NULL,
        LPCWSTR strRecordDescription = NULL,
        DWORD dwTranslationCode = (DWORD)-1) const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return false;
        }

        if (!strRecordDescription)
            strRecordDescription = strRecordName;

        DWORD dwDefaultTranslationCode = 0x04E40409;
        LPWORD lpwTranslationCode = (LPWORD)&dwTranslationCode;
        if (dwTranslationCode == (DWORD)-1)
        {
            lpwTranslationCode =
                (LPWORD)QueryValue(L"\\VarFileInfo\\Translation");
            if (!lpwTranslationCode)
                lpwTranslationCode = (LPWORD)&dwDefaultTranslationCode;
        }

        WCHAR strSubBlock[128];
        _snwprintf(strSubBlock, sizeof(strSubBlock) / sizeof(strSubBlock[0]),
            L"\\StringFileInfo\\%04X%04X\\%s",
            lpwTranslationCode[0], lpwTranslationCode[1],
            strRecordName ? strRecordName : L"FileDescription");
        strSubBlock[sizeof(strSubBlock) / sizeof(strSubBlock[0]) - 1] = 0;

        LPWSTR lpVerBuf = QueryValue(strSubBlock);
        if (lpVerBuf)
        {
            oem_printf(stdout, "%1!ws!: %2!ws!%%n",
                (strRecordDescription ? strRecordDescription :
                L"File description"), lpVerBuf);
            return true;
        }
        else
            return false;
    }

    bool PrintCommonFileVerRecordsA() const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return false;
        }

        LPDWORD lpdwTranslationCode =
            (LPDWORD)QueryValue("\\VarFileInfo\\Translation");

        DWORD dwTranslationCode;
        if (lpdwTranslationCode)
        {
            dwTranslationCode = *lpdwTranslationCode;
            CHAR tcLanguageName[128];
            if (VerLanguageNameA(dwTranslationCode & 0x0000FFFF, tcLanguageName,
                _countof(tcLanguageName)))
            {
                CharToOemA(tcLanguageName, tcLanguageName);
                printf("Language: (0x%X) %s\n", dwTranslationCode,
                    tcLanguageName);
            }
        }
        else
        {
            dwTranslationCode = 0x04E40409;
        }

        PrintFileVerRecord("CompanyName", "Company name",
            dwTranslationCode);
        PrintFileVerRecord("FileDescription", "File description",
            dwTranslationCode);
        PrintFileVerRecord("FileVersion", "File version",
            dwTranslationCode);
        PrintFileVerRecord("InternalName", "Internal name",
            dwTranslationCode);
        PrintFileVerRecord("LegalCopyright", "Legal copyright",
            dwTranslationCode);
        PrintFileVerRecord("OriginalFilename", "Original filename",
            dwTranslationCode);
        PrintFileVerRecord("ProductName", "Product name",
            dwTranslationCode);
        PrintFileVerRecord("ProductVersion", "Product version",
            dwTranslationCode);

        return true;
    }

    bool PrintCommonFileVerRecordsW() const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return false;
        }

        LPDWORD lpdwTranslationCode =
            (LPDWORD)QueryValue(L"\\VarFileInfo\\Translation");

        DWORD dwTranslationCode;
        if (lpdwTranslationCode)
        {
            dwTranslationCode = *lpdwTranslationCode;
            WCHAR tcLanguageName[128];
            if (VerLanguageNameW(dwTranslationCode & 0x0000FFFF, tcLanguageName,
                _countof(tcLanguageName)))
            {
                oem_printf(stdout, "Language: (0x%1!X!) %2!ws!%%n", dwTranslationCode,
                    tcLanguageName);
            }
        }
        else
        {
            dwTranslationCode = 0x04E40409;
        }

        PrintFileVerRecord(L"CompanyName", L"Company name",
            dwTranslationCode);
        PrintFileVerRecord(L"FileDescription", L"File description",
            dwTranslationCode);
        PrintFileVerRecord(L"FileVersion", L"File version",
            dwTranslationCode);
        PrintFileVerRecord(L"InternalName", L"Internal name",
            dwTranslationCode);
        PrintFileVerRecord(L"LegalCopyright", L"Legal copyright",
            dwTranslationCode);
        PrintFileVerRecord(L"OriginalFilename", L"Original filename",
            dwTranslationCode);
        PrintFileVerRecord(L"ProductName", L"Product name",
            dwTranslationCode);
        PrintFileVerRecord(L"ProductVersion", L"Product version",
            dwTranslationCode);

        return true;
    }

    DWORD GetLanguageName(LPSTR szLang, DWORD nSize)
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return 0;
        }

        LPWORD lpwTranslationCode =
            (LPWORD)QueryValue("\\VarFileInfo\\Translation");

        if (!lpwTranslationCode)
            return 0;

        return VerLanguageNameA(*lpwTranslationCode, szLang, nSize);
    }

    DWORD GetLanguageName(LPWSTR szLang, DWORD nSize)
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return 0;
        }

        LPWORD lpwTranslationCode =
            (LPWORD)QueryValue(L"\\VarFileInfo\\Translation");

        if (!lpwTranslationCode)
            return 0;

        return VerLanguageNameW(*lpwTranslationCode, szLang, nSize);
    }
};
#endif // _INC_WFILEVER_
