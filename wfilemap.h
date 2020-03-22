#ifndef _INC_WFILEMAP_
#define _INC_WFILEMAP_
#include <wio.h>

enum FMAP_ACCESS // Access type for file mapping views
{
  FMAP_RO,      // Read only view
  FMAP_RW,      // Read/write view
  FMAP_WC       // Copy to writeable view
};

#ifndef _QWORD_DEFINED
typedef DWORDLONG QWORD;
#define _QWORD_DEFINED
#endif

/// Wrapper for file mapping objects with mapped view.
class WMapping : public WKernelObject
{
   protected:
   LPVOID lpView;          // Pointer to view of mapped file

   void SetHandle(HANDLE handle)
   {
      Close();
      hObject = handle;
   }

   public:
   /// Creates empty object. No mapping is created.
   WMapping()
   : lpView(NULL)
   {
   }

   /// Creates a file mapping to an open file.
   WMapping(
    HANDLE hFile,	// handle to file to map
    DWORD flProtect = PAGE_READWRITE,	// protection for mapping object
    QWORD qwMaximumSize = 0,	// object size
    LPCTSTR lpName = NULL, 	// name of file-mapping object
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes = NULL) // optional sec attr
   : WKernelObject(CreateFileMapping(hFile, lpFileMappingAttributes, flProtect,
				     (DWORD)(qwMaximumSize >> 32),
				     (DWORD)qwMaximumSize, lpName)),
     lpView(NULL)
   {
   }

   /// Creates a memory-only mapping.
   WMapping(
    LPCTSTR lpName, 	// name of file-mapping object
    DWORD flProtect = PAGE_READWRITE,	// protection for mapping object
    QWORD qwMaximumSize = 0,	// object size
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes = NULL) // optional sec attr
   : WKernelObject(CreateFileMapping(INVALID_HANDLE_VALUE,
      lpFileMappingAttributes, flProtect, (DWORD)(qwMaximumSize >> 32),
      (DWORD)qwMaximumSize, lpName)), lpView(NULL)
   {
   }

   /// Opens an existing file mapping object on the system.
   WMapping(LPCTSTR lpMapName, DWORD dwDesiredAccess, BOOL bInheritHandle)
   : WKernelObject(OpenFileMapping(dwDesiredAccess, bInheritHandle,
				   lpMapName)),
      lpView(NULL)
   {
   }

   /**
      Before WKernelObject destructor closes the handle to the file mapping
      object, any view must be unmapped.
      */
   /// Destructor unmaps any existing view.
   ~WMapping()
   {
      Unmap();
   }

   /// Creates a file mapping to an open file.
   bool Create(
    HANDLE hFile = INVALID_HANDLE_VALUE,	// handle to file to map
    DWORD flProtect = PAGE_READWRITE,	// protection for mapping object
    QWORD qwMaximumSize = 0,	// object size
    LPCTSTR lpName = NULL, 	// name of file-mapping object
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes = NULL) // optional sec attr
   {
      Close();
      hObject = CreateFileMapping(hFile, lpFileMappingAttributes, flProtect,
         (DWORD)(qwMaximumSize >> 32), (DWORD)qwMaximumSize, lpName);
      return operator bool();
   }

   /// Creates a memory-only mapping.
   bool Create(
    LPCTSTR lpName, 	// name of file-mapping object
    DWORD flProtect = PAGE_READWRITE,	// protection for mapping object
    QWORD qwMaximumSize = 0,	// object size
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes = NULL) // optional sec attr
   {
      Close();
      hObject = CreateFileMapping(INVALID_HANDLE_VALUE,
				  lpFileMappingAttributes,
				  flProtect, (DWORD)(qwMaximumSize >> 32),
				  (DWORD)qwMaximumSize, lpName);
      return operator bool();
   }

   bool Open(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCTSTR lpMapName)
   {
      Close();
      hObject = OpenFileMapping(dwDesiredAccess, bInheritHandle, lpMapName);
      return operator bool();
   }

   /// Maps the file mapping object into memory of this process.
   bool Map(
    DWORD dwDesiredAccess = FILE_MAP_ALL_ACCESS,	// access mode
    QWORD qwFileOffset = 0,	// offset in file mapping object
    DWORD dwNumberOfBytesToMap = 0,	// number of bytes to map
    LPVOID lpBaseAddress = NULL) // suggested starting address for mapped view
   {
      Unmap();
      lpView = MapViewOfFileEx(hObject, dwDesiredAccess,
         (DWORD)(qwFileOffset >> 32), (DWORD)qwFileOffset,
			       dwNumberOfBytesToMap,
			       lpBaseAddress);
      return lpView != NULL;
   }

   void Unmap()
   {
      if (lpView == NULL)
         return;

      UnmapViewOfFile(lpView);
      lpView = NULL;
   }

   void Close()
   {
      Unmap();
      WKernelObject::Close();
   }

   LPVOID GetView() const
   {
      return lpView;
   }

   bool Flush(DWORD dwNumberOfBytesToFlush = 0) const
   {
      if (lpView)
         return FlushViewOfFile(lpView, dwNumberOfBytesToFlush) == TRUE;
      else
         return false;
   }
};

/**
   Handles open/close on a file and creates a WMapping object representing the
   file.
   */
/// Open file-Create mapping-Map view using one class.
class WMappedFile : public WFile
{
   protected:
   WMapping map;           // File mapping object
   LPCTSTR lpMapName;      // Name of file mapping object
   QWORD  qwFileSize;      // New size of file
   QWORD  qwMapOffset;     // File offset for data mapped into view
   DWORD  dwBytesMapped;   // File bytes mapped into view
   FMAP_ACCESS Access;     // Access type
   LPSECURITY_ATTRIBUTES lpSecurityAttributes;  // Security Attributes object

   /// Function to open an existing file mapping.
   bool OpenMapAndView()
   {
   	// Evaluate the file access parameter from our Access parameter
      DWORD dwDesiredAccess = 0;
   	if (Access == FMAP_RO)
      	dwDesiredAccess |= FILE_MAP_READ;
      else if (Access == FMAP_RW)
	  	   dwDesiredAccess |= FILE_MAP_ALL_ACCESS;
      else if (Access == FMAP_WC)
      	dwDesiredAccess |= FILE_MAP_COPY;

	   // Open an existing file mapping object
   	if (!map.Open(dwDesiredAccess, lpSecurityAttributes ?
         lpSecurityAttributes->bInheritHandle : false, lpMapName))
      {
         dwLastError = GetLastError();
         return false;
      }

      // Now make the view
      if (MapView())
         return true;

      map.Close();
      return false;
   }

   /// Function to create the file mapping and view.
   bool CreateMapAndView()
   {
      // Evaluate the file access parameter from our Access parameter
      DWORD flProtect = SEC_COMMIT;
      if (Access == FMAP_RO)
         flProtect |= PAGE_READONLY;
      else if (Access == FMAP_RW)
         flProtect |= PAGE_READWRITE;
      else if (Access == FMAP_WC)
         flProtect |= PAGE_WRITECOPY;

	   // Create a file mapping object
      map.Create(hObject, flProtect, qwFileSize, lpMapName,
         lpSecurityAttributes);
      if (!map)
      {
         dwLastError = GetLastError();
         return false;
      }

      // Now make the view
      if (MapView())
         return true;

      map.Close();
      return false;
   }

   /// Function to create just the view.
   bool MapView() 
   {
	   DWORD dwDesiredAccess = 0;
      if (Access == FMAP_RO)
         dwDesiredAccess |= FILE_MAP_READ;
      else if (Access == FMAP_RW)
	  	   dwDesiredAccess |= FILE_MAP_ALL_ACCESS;
      else if (Access == FMAP_WC)
         dwDesiredAccess |= FILE_MAP_COPY;

      // Map view into memory.
      if (!map.Map(dwDesiredAccess, qwMapOffset, dwBytesMapped))
      {
         dwLastError = GetLastError();
         return false;
      }

      if (!qwFileSize)
      {
         DWORD dwHiDWord = 0;
         qwFileSize = GetFileSize(hObject, &dwHiDWord);
         qwFileSize |= (QWORD)dwHiDWord << 32;
      }

      if (!dwBytesMapped)
         dwBytesMapped = (DWORD)(qwFileSize - qwMapOffset);

      return true;
   }

   /// Function to free the view, close the mapping.
   void CloseMap()
   {
      map.Close();
   }

   /// Just unmap the view.
   void UnmapView()
   {
      map.Unmap();
	}

   /// Free all resources referenced by this object.
   void Close() 
   {
      map.Close();
      WKernelObject::Close();

      if (lpSecurityAttributes)
      {
         delete lpSecurityAttributes;
         lpSecurityAttributes = NULL;
      }
   }

   public:
   DWORD dwLastError;    // Last error code
   DWORD dwRecLen;       // Record length used by index operator

   // Cast to pointer (or handle) returns pointer (handle) to the view
   operator LPVOID() const
   {
      if (this)
         return map.GetView();
      else
         return NULL;
   }

   // Cast to LPSTR returns pointer to the view
   operator LPSTR() const
   {
      if (this)
         return (LPSTR)map.GetView();
      else
         return NULL;
   }

   // Cast to LPWSTR pointer returns pointer to the view
   operator LPWSTR() const
   {
      if (this)
         return (LPWSTR)map.GetView();
      else
         return NULL;
   }

   // Index operator. Used to get a record from a set of fixed-length record
   // in a mapped file. Record length is set in the dwRecLen public data
   // member.
   LPVOID operator[](DWORD dwRecNr) const
   {
      if (!this)
         return NULL;

      if (dwBytesMapped < (dwRecNr*dwRecLen)+dwRecLen)
         return NULL;

   	return (LPVOID)(((LPSTR)map.GetView())+dwRecNr*dwRecLen);
   }

   // Returns void pointer to the view
   LPVOID GetView() const
   {
      if (this)
         return map.GetView();
      else
         return NULL;
   }

   bool operator!() const
   {
      return map.GetView() == NULL;
   }

   operator bool() const
   {
      return map.GetView() != NULL;
   }

   // Get size of the file mapping object
   QWORD GetSizeOfMappedFile() const
   {
      if (this)
         return qwFileSize;
      else
         return 0;
   }

   // Get number of bytes that was read from physical file into the view
   DWORD GetBytesMapped() const
   {
      if (this)
         return dwBytesMapped;
      else
         return 0;
   }

   // Get the byte offset in the file, where the view byte read started
   QWORD GetMapOffset() const
   {
      if (this)
         return qwMapOffset;
      else
	return (QWORD)-1;
   }

   // Make a new view with different offset and length from the mapped file.
   void Resize(DWORD dwBytesToMap = 0, // Length of new view. 0 = Entire file
               QWORD qwNewMapOffset = 0, // File offset for new view to begin
               QWORD qwNewFileSize = 0) // New size of file. 0 = Current size.
   {
      if (!this)
         return;

   	// Initialize data members with new values
      qwMapOffset = qwNewMapOffset;
      dwBytesMapped = dwBytesToMap;

      // If file size should be changed, we must close the file mapping object
      if (qwNewFileSize)
      {
		   qwFileSize = qwNewFileSize;
         map.Close();
         CreateMapAndView();
      }
      else
      {
	      // No file size change, just remake the view
   	   UnmapView();
         MapView();
      }
   }

   // Flush number of bytes of the view to the physical file. 0 = Entire view.
   BOOL Flush(DWORD dwNumberOfBytesToFlush = 0) const
   {
      if (this)
         return map.Flush(dwNumberOfBytesToFlush);
      else
         return false;
   }

   // Constructor by file name
	explicit WMappedFile(LPCTSTR lpFileName,   // Name of file to map
      FMAP_ACCESS NewAccess = FMAP_RW,  // Access to map and view
      DWORD dwShareMode = 0,   // Share mode for open file. FILE_SHARE_READ
                                    // and/or FILE_SHARE_WRITE is valid.
      QWORD qwNewFileSize = 0,  // New size of file set by mapping object
      QWORD qwNewMapOffset = 0, // Byte offset of view in physical file
      DWORD dwBytesToMap = 0,  // Bytes to map from file

      // Thise is used in CreateFile() call
      DWORD dwCreationDistribution = OPEN_EXISTING,

      // Name of file mapping object used in CreateFileMapping() call
      LPCTSTR lpNewMapName = NULL,

      // These are used in CreateFile() call
      DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
      const LPSECURITY_ATTRIBUTES lpNewSecurityAttributes = NULL, // Also used by CreateFileMapping()
      HANDLE hTemplateFile = NULL)
   	// Construct data members
      : Access(NewAccess), qwFileSize(qwNewFileSize), qwMapOffset(qwNewMapOffset),
      dwBytesMapped(dwBytesToMap), lpMapName(lpNewMapName),
      lpSecurityAttributes(NULL), dwLastError(NO_ERROR), dwRecLen(1)
	{
		// Evaluate the file access parameter from our Access parameter
		DWORD dwDesiredAccess = 0;

      // Copy security attributes
      if (lpNewSecurityAttributes)
         lpSecurityAttributes =
            new SECURITY_ATTRIBUTES(*lpNewSecurityAttributes);

      if (lpMapName)   // If an object name was supplied
         OpenMapAndView();  // First attempt to open an existing object

      if (map.GetView() != NULL)
         return;

      // Open the file
     	if (Access == FMAP_RO)
  	      dwDesiredAccess = GENERIC_READ;
      else if ((Access == FMAP_RW) | (Access == FMAP_WC))
     	   dwDesiredAccess = GENERIC_READ|GENERIC_WRITE;

		if ((hObject = CreateFile(lpFileName, dwDesiredAccess, dwShareMode,
   	   lpSecurityAttributes, dwCreationDistribution, dwFileAttributes,
   	   hTemplateFile)) == INVALID_HANDLE_VALUE)
	   {
         dwLastError = GetLastError();
         Close();
	   }
      else
      	// Create the mapping and view using this file handle
	      CreateMapAndView();
	}

   // Constructor by already open file handle
	explicit WMappedFile(HANDLE hOpenFile,  // Open file handle
   		  FMAP_ACCESS NewAccess = FMAP_RW,  // Access to map and view
           QWORD qwNewFileSize = 0, // New size of file set by mapping object
           QWORD qwNewMapOffset = 0,  // Byte offset of view in physical file
           DWORD dwBytesToMap = 0,  // Bytes to map from file

           // This is used by call to CreateFileMapping().
           LPCTSTR lpNewMapName = NULL,
           const LPSECURITY_ATTRIBUTES lpNewSecurityAttributes = NULL
          )
      // Construct data members
      : Access(NewAccess), qwFileSize(qwNewFileSize),
         qwMapOffset(qwNewMapOffset), dwBytesMapped(dwBytesToMap),
         lpMapName(lpNewMapName),
         lpSecurityAttributes(NULL), dwLastError(NO_ERROR), dwRecLen(1)
   {
      // Copy security attributes
      if (lpNewSecurityAttributes)
         lpSecurityAttributes =
            new SECURITY_ATTRIBUTES(*lpNewSecurityAttributes);

   	// Duplicate the incoming file handle
      if (hOpenFile != INVALID_HANDLE_VALUE)
         if (!DuplicateHandle(GetCurrentProcess(), hOpenFile,
            GetCurrentProcess(), &hObject, 0,
            lpSecurityAttributes ? lpSecurityAttributes->bInheritHandle : false,
         	DUPLICATE_SAME_ACCESS) )
      	   dwLastError = GetLastError();

   	// Create the mapping and view using this file handle
      if ((hObject != INVALID_HANDLE_VALUE) |
         (hOpenFile == INVALID_HANDLE_VALUE))
	      CreateMapAndView();
   }

   // Copy constructor. Duplicates existing object's file handle and creates
   // a new mapping and view for the new object. Data members are copied into
   // the new object.
   explicit WMappedFile(const WMappedFile& filemap)
      // Construct data members by copy the other object's data
   	: lpMapName(filemap.lpMapName), Access(filemap.Access),
         qwFileSize(filemap.qwFileSize), qwMapOffset(filemap.qwMapOffset),
         dwBytesMapped(filemap.dwBytesMapped), lpSecurityAttributes(NULL),
         dwLastError(NO_ERROR), dwRecLen(1)
   {
      // Copy security attributes into new struct
      if (filemap.lpSecurityAttributes)
         lpSecurityAttributes =
            new SECURITY_ATTRIBUTES(*filemap.lpSecurityAttributes);

      if (filemap.Handle() == INVALID_HANDLE_VALUE)
      {
         CreateMapAndView();
         return;
      }

	   // Duplicate the source object's file handle
      hObject = filemap.NewHandle(DUPLICATE_SAME_ACCESS,
         lpSecurityAttributes ? lpSecurityAttributes->bInheritHandle : false);

      if (hObject != INVALID_HANDLE_VALUE)
      	// Create the mapping and view using this file handle
         CreateMapAndView();
      else
         dwLastError = GetLastError();
   }

   ~WMappedFile()
   {
      Close();
   }
};
#endif // _INC_WFILEMAP_

