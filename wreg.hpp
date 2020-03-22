#if !defined(_INC_WREG_HPP_) && defined(__cplusplus)
#define _INC_WREG_HPP_

#ifndef INVALID_HKEY
#define INVALID_HKEY ((HKEY)INVALID_HANDLE_VALUE)
#endif

#include <wbase.hpp>

class WRegKey : public WObjectHandle
{
private:
    static bool SetError(LONG lRegError)
    {
        if (lRegError == NO_ERROR)
        {
            return true;
        }
        else
        {
            SetLastError(lRegError);
            return false;
        }
    }

public:
    /// Init with existing handle.
    explicit WRegKey(HKEY handle = INVALID_HKEY)
        : WObjectHandle((HANDLE)handle)
    {
    }

    /// Init with handle from other object.
    explicit WRegKey(const WRegKey &obj)
        : WObjectHandle(obj.Handle())
    {
    }

    explicit WRegKey(LPCSTR lpSubKey, HKEY hOpenKey = HKEY_CURRENT_USER,
        bool bCreate = false)
        : WObjectHandle(INVALID_HANDLE_VALUE)
    {
        LONG lRegError = (bCreate ? RegCreateKeyA : RegOpenKeyA)
            (hOpenKey, lpSubKey, (PHKEY)&hObject);
        if (lRegError == ERROR_SUCCESS)
            return;

        SetLastError(lRegError);
    }

    explicit WRegKey(LPCWSTR lpSubKey, HKEY hOpenKey = HKEY_CURRENT_USER,
        bool bCreate = false)
        : WObjectHandle(INVALID_HANDLE_VALUE)
    {
        LONG lRegError = (bCreate ? RegCreateKeyW : RegOpenKeyW)
            (hOpenKey, lpSubKey, (PHKEY)&hObject);
        if (lRegError == ERROR_SUCCESS)
            return;

        SetLastError(lRegError);
    }

    explicit WRegKey(LPCSTR lpSubKey, WRegKey& OpenKey, bool bCreate = false)
        : WObjectHandle(INVALID_HANDLE_VALUE)
    {
        LONG lRegError = (bCreate ? RegCreateKeyA : RegOpenKeyA)
            (OpenKey.hKey(), lpSubKey, (PHKEY)&hObject);
        if (lRegError == ERROR_SUCCESS)
            return;

        SetLastError(lRegError);
    }

    explicit WRegKey(LPCWSTR lpSubKey, WRegKey& OpenKey, bool bCreate = false)
        : WObjectHandle(INVALID_HANDLE_VALUE)
    {
        LONG lRegError = (bCreate ? RegCreateKeyW : RegOpenKeyW)
            (OpenKey.hKey(), lpSubKey, (PHKEY)&hObject);
        if (lRegError == ERROR_SUCCESS)
            return;

        SetLastError(lRegError);
    }

    /// Closes existing object and init with handle from other object.
    WRegKey& operator=(const WRegKey &obj)
    {
        Close();
        hObject = obj.Handle();
        return *this;
    }

    /// Closes existing object and init with duplicate of handle.
    WRegKey& operator=(HKEY handle)
    {
        Close();

        hObject = (HANDLE)handle;

        return *this;
    }

    /// Closes the handle.
    ~WRegKey()
    {
        Close();
    }

    /// Returns the handle to the embedded kernel object.
    HKEY hKey() const
    {
        return (HKEY)hObject;
    }

    /// Closes the embedded handle if it is valid.
    bool Close()
    {
        if (!IsValidHandle())
            return false;

        LONG lRegError = RegCloseKey(hKey());
        if (lRegError == ERROR_SUCCESS)
        {
            hObject = INVALID_HANDLE_VALUE;
            return true;
        }
        else
        {
            SetLastError(lRegError);
            return false;
        }
    }

    bool QueryKey(LPCSTR lpSubKey, LPSTR lpData, PLONG lpSize)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegQueryValueA(hKey(), lpSubKey, lpData, lpSize));
    }

    bool QueryKey(LPCWSTR lpSubKey, LPWSTR lpData, PLONG lpSize)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegQueryValueW(hKey(), lpSubKey, lpData, lpSize));
    }

    bool SetKey(LPCSTR lpSubKey, LPCSTR lpData, DWORD dwSize)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegSetValueA(hKey(), lpSubKey, REG_SZ, lpData, dwSize));
    }

    bool SetKey(LPCWSTR lpSubKey, LPCWSTR lpData, DWORD dwSize)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegSetValueW(hKey(), lpSubKey, REG_SZ, lpData, dwSize));
    }

    bool QueryValue(LPCSTR lpValueName = NULL, LPVOID lpData = NULL,
        LPDWORD lpSize = 0, LPDWORD lpType = NULL)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegQueryValueExA(hKey(), lpValueName, NULL, lpType, (LPBYTE)lpData,
            lpSize));
    }

    bool QueryValue(LPCWSTR lpValueName = NULL, LPVOID lpData = NULL,
        LPDWORD lpSize = 0, LPDWORD lpType = NULL)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegQueryValueExW(hKey(), lpValueName, NULL, lpType, (LPBYTE)lpData,
            lpSize));
    }

    bool SetValue(LPCSTR lpValueName = NULL, const LPVOID lpData = NULL,
        DWORD dwSize = 0, DWORD dwType = REG_BINARY)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegSetValueExA(hKey(), lpValueName, NULL, dwType,
            (CONST LPBYTE)lpData, dwSize));
    }

    bool SetValue(LPCWSTR lpValueName = NULL, const LPVOID lpData = NULL,
        DWORD dwSize = 0, DWORD dwType = REG_BINARY)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegSetValueExW(hKey(), lpValueName, NULL, dwType,
            (CONST LPBYTE)lpData, dwSize));
    }

    bool DeleteValue(LPCSTR lpValueName)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegDeleteValueA(hKey(), lpValueName));
    }

    bool DeleteValue(LPCWSTR lpValueName)
        const
    {
        if (!IsValidHandle())
            return SetError(ERROR_INVALID_HANDLE);

        return SetError(RegDeleteValueW(hKey(), lpValueName));
    }
};
#endif // _INC_WREG_HPP_

