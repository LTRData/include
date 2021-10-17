#ifndef _INC_WPDH_
#define _INC_WPDH_

#include <windows.h>

#ifdef __cplusplus
#include <wbase.h>

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

/**
Base class for all kernel object wrapping classes.
Uses CloseHandle() on the embedded handle when object is destroyed.
*/
class WPDHQuery : public WObjectHandle
{
public:
    explicit WPDHQuery(PDH_HLOG hDataSource,
        DWORD_PTR dwUserData = 0)
    {
        PDH_STATUS status = PdhOpenQueryH(hDataSource, dwUserData, &hObject);
        if (status != NO_ERROR)
        {
            SetLastError(status);
            hObject = NULL;
        }
    }

    explicit WPDHQuery(LPCWSTR szDataSource = NULL,
        DWORD_PTR dwUserData = 0)
    {
        PDH_STATUS status = PdhOpenQueryW(szDataSource, dwUserData, &hObject);
        if (status != NO_ERROR)
        {
            SetLastError(status);
            hObject = NULL;
        }
    }

    explicit WPDHQuery(LPCSTR szDataSource,
        DWORD_PTR dwUserData = 0)
    {
        PDH_STATUS status = PdhOpenQueryA(szDataSource, dwUserData, &hObject);
        if (status != NO_ERROR)
        {
            SetLastError(status);
            hObject = NULL;
        }
    }

    /// Init with existing handle.
    explicit WPDHQuery(PDH_HQUERY handle)
        : WObjectHandle(handle)
    {
    }

    /// Init with handle from other object.
    explicit WPDHQuery(const WPDHQuery& obj)
        : WObjectHandle(obj.Handle())
    {
    }

    /// Closes existing object and init with handle from other object.
    WPDHQuery& operator=(const WPDHQuery& obj)
    {
        SetHandle(obj.Handle());
        return *this;
    }

    /// Closes existing object and init with supplied handle.
    WPDHQuery& operator=(PDH_HQUERY handle)
    {
        SetHandle(handle);

        return *this;
    }

    /// Closes existing object and init with supplied handle.
    void SetHandle(PDH_HQUERY handle)
    {
        Close();

        hObject = handle;
    }

    /// Closes the handle.
    ~WPDHQuery()
    {
        Close();
    }

    /// Closes the embedded handle if it is valid.
    bool Close()
    {
        if (!IsValidHandle(hObject))
            return false;

        if (PdhCloseQuery(hObject) == NO_ERROR)
        {
            hObject = INVALID_HANDLE_VALUE;
            return true;
        }
        else
            return false;
    }

};

#endif // __cplusplus

#endif // _INC_WIO_

