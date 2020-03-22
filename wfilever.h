#ifndef _INC_WFILEVER_
#define _INC_WFILEVER_

#include <winstrct.h>

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

        LPSTR lpVerBuf;
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

        LPWSTR lpVerBuf;
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

    bool PrintFileVerRecord(LPSTR strRecordName = NULL,
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

    bool PrintFileVerRecord(LPWSTR strRecordName = NULL,
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
            printf("%ws: %ws\n",
                (strRecordDescription ? strRecordDescription :
                L"File description"), lpVerBuf);
            return true;
        }
        else
            return false;
    }

    bool PrintCommonFileVerRecords() const
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return false;
        }

        LPDWORD lpdwTranslationCode =
            (LPDWORD)QueryValue(TEXT("\\VarFileInfo\\Translation"));

        DWORD dwTranslationCode;
        if (lpdwTranslationCode)
        {
            dwTranslationCode = *lpdwTranslationCode;
            TCHAR tcLanguageName[128];
            if (VerLanguageName(dwTranslationCode & 0x0000FFFF, tcLanguageName,
                _countof(tcLanguageName)))
            {
#ifndef UNICODE
                CharToOem(tcLanguageName, tcLanguageName);
#endif
                printf("Language: (0x%X) %s\n", dwTranslationCode,
                    tcLanguageName);
            }
        }
        else
            dwTranslationCode = 0x04E40409;

        PrintFileVerRecord(TEXT("CompanyName"), TEXT("Company name"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("FileDescription"), TEXT("File description"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("FileVersion"), TEXT("File version"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("InternalName"), TEXT("Internal name"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("LegalCopyright"), TEXT("Legal copyright"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("OriginalFilename"), TEXT("Original filename"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("ProductName"), TEXT("Product name"),
            dwTranslationCode);
        PrintFileVerRecord(TEXT("ProductVersion"), TEXT("Product version"),
            dwTranslationCode);

        return true;
    }

    DWORD GetLanguageName(LPTSTR szLang, DWORD nSize)
    {
        if (!*this)
        {
            SetLastError(ERROR_NO_MORE_ITEMS);
            return 0;
        }

        LPWORD lpwTranslationCode =
            (LPWORD)QueryValue(TEXT("\\VarFileInfo\\Translation"));

        if (!lpwTranslationCode)
            return 0;

        return VerLanguageName(*lpwTranslationCode, szLang, nSize);
    }
};
#endif // _INC_WFILEVER_
