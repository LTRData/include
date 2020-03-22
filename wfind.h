#if !defined(_INC_WFIND_) && defined(__cplusplus)
#define _INC_WFIND_

#include <wtime.h>
#include <winspool.h>

#ifndef _QWORD_DEFINED
typedef DWORDLONG QWORD;
#define _QWORD_DEFINED
#endif

#pragma warning(push)
#pragma warning(disable:4355)

/**
   WFileFinder class. Encapsulation of Windows API functions FindFirstFile(),
   FindNextFile() and FindClose().
   */
class WFileFinder : public WIN32_FIND_DATA
{
   protected:
   /// Open find handle used in internal API calls.
   HANDLE hFind;

   public:
   /// Function to get WFileTime object with creation time.
   WFileTime & CreationTime()
   {
      return *(WFileTime*)&ftCreationTime;
   }

   /// Function to get WFileTime object with last access time.
   WFileTime & LastAccessTime()
   {
      return *(WFileTime*)&ftLastAccessTime;
   }

   /// Function to get WFileTime object with write/modified time.
   WFileTime & LastWriteTime()
   {
      return *(WFileTime*)&ftLastWriteTime;
   }

   /// Returns file size as QWORD.
   QWORD GetFile64BitSize() const
   {
      return ((QWORD)nFileSizeHigh * MAXDWORD) + nFileSizeLow;
   }

   /// Returns compressed file size as QWORD.
   QWORD GetCompressedFile64BitSize() const
   {
      ULARGE_INTEGER uliSize;
      uliSize.u.LowPart =
         GetCompressedFileSize(cFileName, &uliSize.u.HighPart);

      if (uliSize.u.LowPart == MAXDWORD ? GetLastError() != NO_ERROR : false)
	return (QWORD)-1i64;

      return uliSize.QuadPart;
   }

   /// cast to bool returns status of object.
   operator bool() const
   {
      return hFind != INVALID_HANDLE_VALUE;
   }

   /// Starts find with using name pattern.
   WFileFinder(LPCTSTR lpFileName) : hFind(FindFirstFile(lpFileName, this))
   {
   }

   /// Restart find with new file name pattern.
   WFileFinder& operator =(LPCTSTR lpFileName)
   {
      if( hFind != INVALID_HANDLE_VALUE )
         FindClose(hFind);

      hFind = FindFirstFile(lpFileName, this);

      return *this;
   }

   /// Closes the find handle.
   ~WFileFinder()
   {
      if (hFind != INVALID_HANDLE_VALUE)
         FindClose(hFind);
   }

   /// Finds next matching file.
   BOOL Next()
   {
      return FindNextFile(hFind, this);
   }
};

/// WFilteredFileFinder adds filtered search functionality to WFileFinder.
class WFilteredFileFinder : public WFileFinder
{
   protected:
   /// Checks found file against filter.
   BOOL Filter() 
   {
      if( !(dwFileAttributes & dwFileAttrMask) &
         !(~dwFileAttributes & dwFileAttrRequired) &
         (*(QWORD*)&ftCreationTime >= ftCreationTime1) &
         (*(QWORD*)&ftCreationTime <= ftCreationTime2) &
         (*(QWORD*)&ftLastAccessTime >= ftLastAccessTime1) &
         (*(QWORD*)&ftLastAccessTime <= ftLastAccessTime2) &
         (*(QWORD*)&ftLastWriteTime >= ftLastWriteTime1) &
         (*(QWORD*)&ftLastWriteTime <= ftLastWriteTime2) &
         ((((QWORD)nFileSizeHigh * MAXDWORD) + nFileSizeLow) >= qwMinSize) &
         ((((QWORD)nFileSizeHigh * MAXDWORD) + nFileSizeLow) <= qwMaxSize) )
         return true;
      else
      {
         if( !WFileFinder::Next() )  // Find next if this didn't match
            return false;

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
   QWORD qwMinSize;                      // File size interval
   QWORD qwMaxSize;
   WFileTime ftLastWriteTime1;  // Last write time interval
   WFileTime ftLastWriteTime2;
   WFileTime ftLastAccessTime1; // Last access time interval
   WFileTime ftLastAccessTime2;
   WFileTime ftCreationTime1;   // Creation time interval
   WFileTime ftCreationTime2;

   /// Overridden operator=() with filter function.
   WFilteredFileFinder& operator =(LPCTSTR lpFileName)
   {
      WFileFinder::operator=(lpFileName);

      if (hFind == INVALID_HANDLE_VALUE)
         return *this;

      if (!Filter())
         if (hFind != INVALID_HANDLE_VALUE)
         {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
         }
      return *this;
   }

   /// Begin find with file name pattern and initialize filter data.
   WFilteredFileFinder(LPCTSTR lpFileName,
      DWORD dwSetFileAttrMask = 0,     // Specify attributes not to find
      DWORD dwSetFileAttrRequired = 0,   // Specify attributes to find
      QWORD qwSetMinSize = 0,                      // File size interval
      QWORD qwSetMaxSize = (QWORD)-1,
      WFileTime ftSetLastWriteTime1 = 0,  // Last write time interval
      WFileTime ftSetLastWriteTime2 = (QWORD)-1,
      WFileTime ftSetLastAccessTime1 = 0, // Last access time interval
      WFileTime ftSetLastAccessTime2 = (QWORD)-1,
      WFileTime ftSetCreationTime1 = 0,   // Creation time interval
      WFileTime ftSetCreationTime2 = (QWORD)-1
   )
   : WFileFinder(lpFileName), dwFileAttrMask(dwSetFileAttrMask),
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
      if (!Filter())
         if (hFind != INVALID_HANDLE_VALUE)
         {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
         }
   }

   /// Find next matching file.
   BOOL Next()
   {
      if (!WFileFinder::Next())
         return false;

      return Filter();
   }
};

class WFileChangeNotification
{
   protected:
   /// Open find handle used in internal API calls.
   HANDLE hFind;

   public:
   BOOL Next() const
   {
      return FindNextChangeNotification(hFind);
   }

   DWORD Wait(DWORD dwMilliseconds = INFINITE) const
   {
      return WaitForSingleObject(hFind, dwMilliseconds);
   }

   BOOL SyncNext(DWORD dwMilliseconds = INFINITE) const
   {
      if (Wait(dwMilliseconds) != WAIT_OBJECT_0)
         return false;
      else
         return Next();
   }

   /// Returns the waitable handle to this change notification.
   HANDLE Synchronizer() const
   {
      return hFind;
   }

   explicit WFileChangeNotification(
      LPCTSTR lpPathName,	// pointer to name of directory to watch
      BOOL bWatchSubtree = false, // flag for monitoring directory subtree
      DWORD dwNotifyFilter = -1) 	// filter conditions to watch for
   : hFind(FindFirstChangeNotification(lpPathName, bWatchSubtree,
      dwNotifyFilter))
   { }

   WFileChangeNotification & operator=(LPCTSTR lpPathName)
   {
      if (hFind != INVALID_HANDLE_VALUE)
         FindCloseChangeNotification(hFind);
      hFind = FindFirstChangeNotification(lpPathName, false, (DWORD)-1);
      return *this;
   }

   ~WFileChangeNotification()
   {
      if (hFind != INVALID_HANDLE_VALUE)
         FindCloseChangeNotification(hFind);
   }
};

/*
class WPrinterChangeNotification
{
   protected:
   /// Open find handle used in internal API calls.
   HANDLE hFind;
   const PRINTER_NOTIFY_INFO *info;
   DWORD dwFlags;

   public:
   BOOL Next() const
   {
      return FindNextPrinterChangeNotification(hFind);
   }

   DWORD Wait(DWORD dwMilliseconds = INFINITE) const
   {
      return WaitForSingleObject(hFind, dwMilliseconds);
   }

   BOOL SyncNext(DWORD dwMilliseconds = INFINITE) const
   {
      if (Wait(dwMilliseconds) != WAIT_OBJECT_0)
         return false;
      else
         return Next();
   }

   /// Returns the waitable handle to this change notification.
   HANDLE Synchronizer() const
   {
      return hFind;
   }

   const PRINTER_NOTIFY_INFO *Info() const
   {
      return info;
   }

   explicit WPrinterChangeNotification(
      HANDLE hPrinter,	// handle to printer or print server to monitor
      DWORD fdwFlags = PRINTER_CHANGE_ALL,   // conditions to monitor
      DWORD fdwOptions = 0)	// printer information to monitor
    : hFind(FindFirstPrinterChangeNotification(hPrinter, fdwFlags, fdwOptions,
      &info)), dwFlags(fdwFlags)
   { }

   WPrinterChangeNotification & operator=(HANDLE hPrinter)
   {
      if (hFind != INVALID_HANDLE_VALUE)
         FindClosePrinterChangeNotification(hFind);
      hFind = FindFirstPrinterChangeNotification(hPrinter, );
   }

   ~WPrinterChangeNotification()
   {
      if (hFind != INVALID_HANDLE_VALUE)
         FindClosePrinterChangeNotification(hFind);
   }
};
*/

#pragma warning(pop)

#endif // _INC_WFIND_

