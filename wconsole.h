#ifndef _INC_WCONSOLE_
#define _INC_WCONSOLE_
#include <windows.h>
#include <wsync.h>

#if _WIN32_WINNT < 0x0500
HWND WINAPI GetConsoleWindow();
#endif

#ifdef __cplusplus
#ifndef EXTERN_C
#  define EXTERN_C extern "C"
#endif

#ifdef _MSC_VER
#  pragma comment(lib, "winstrcp")
#endif

class WConsoleInterface
{
   public:
   virtual HANDLE GetHandle() const = 0;
   virtual HANDLE SetHandle(HANDLE hNewCon) = 0;

   bool SetMode(DWORD dwConMode) const
   {
      DWORD dwOldConMode = GetMode();
      return SetConsoleMode(GetHandle(), dwOldConMode | dwConMode) == TRUE;
   }

   bool UnsetMode(DWORD dwConMode) const
   {
      DWORD dwOldConMode = GetMode();
      return SetConsoleMode(GetHandle(), dwOldConMode & ~dwConMode) == TRUE;
   }

   DWORD GetMode() const
   {
      DWORD dwConMode;
      if (GetConsoleMode(GetHandle(), &dwConMode))
         return dwConMode;
      else
         return 0;
   }

   void ClrEOL()
   {
     CONSOLE_SCREEN_BUFFER_INFO csbi;
     HANDLE hConOut = GetHandle();
     LPSTR cBlankStr;
     DWORD dwWriteLength;

     if (!GetConsoleScreenBufferInfo(hConOut, &csbi))
       return;

     dwWriteLength = csbi.dwSize.X - csbi.dwCursorPosition.X;
     cBlankStr = (LPSTR)HeapAlloc(GetProcessHeap(), 0, dwWriteLength);
     if (cBlankStr == (LPSTR)NULL)
       return;
     FillMemory(cBlankStr, dwWriteLength, ' ');

     WriteConsoleOutputCharacterA(hConOut, cBlankStr, dwWriteLength,
				  csbi.dwCursorPosition, &dwWriteLength);

     HeapFree(GetProcessHeap(), 0, (LPVOID)cBlankStr);
   }  

   void GotoXY(SHORT x, SHORT y)
   {
     COORD coord;
     coord.X = x;
     coord.Y = y;

     SetConsoleCursorPosition(GetHandle(), coord);
   }  
};

class WConsole : public WSynchronizer, public WConsoleInterface
{
   public:
   HANDLE GetHandle() const
   {
      return hObject;
   }

   HANDLE SetHandle(HANDLE hNewCon)
   {
      Close();
      hObject = hNewCon;
      return hObject;
   }

   BOOL Open(LPCTSTR pstrNewCon)
   {
      Close();
      hObject = CreateFile(pstrNewCon, GENERIC_READ|GENERIC_WRITE, 0, NULL,
         OPEN_EXISTING, 0, NULL);
      return hObject != INVALID_HANDLE_VALUE;
   }

   WConsole & operator=(LPCTSTR pstrNewCon)
   {
      Open(pstrNewCon);
      return *this;
   }

   WConsole & operator=(const WConsoleInterface & wcb)
   {
      Close();
      WKernelObject::operator=(wcb.GetHandle());
      return *this;
   }

   explicit WConsole(LPCTSTR pstrNewCon)
   : WSynchronizer(INVALID_HANDLE_VALUE)
   {
      Open(pstrNewCon);
   }

   explicit WConsole(const WConsoleInterface & wc)
   : WSynchronizer(INVALID_HANDLE_VALUE)
   {
      HANDLE hNew;
      if (DuplicateHandle(GetCurrentProcess(), wc.GetHandle(),
         GetCurrentProcess(), &hNew, 0, true, DUPLICATE_SAME_ACCESS))
         SetHandle(hNew);
   }
};

extern const class WConsoleStdIn : public WConsoleInterface
{
   HANDLE GetHandle() const
   {
      return GetStdHandle(STD_INPUT_HANDLE);
   }

   HANDLE SetHandle(HANDLE hCon)
   {
      SetStdHandle(STD_INPUT_HANDLE, hCon);
      return GetStdHandle(STD_INPUT_HANDLE);
   }
} wcStdIn;

extern const class WConsoleStdOut : public WConsoleInterface
{
   HANDLE GetHandle() const
   {
      return GetStdHandle(STD_OUTPUT_HANDLE);
   }

   HANDLE SetHandle(HANDLE hCon)
   {
      SetStdHandle(STD_OUTPUT_HANDLE, hCon);
      return GetStdHandle(STD_OUTPUT_HANDLE);
   }
} wcStdOut;

extern const class WConsoleStdErr : public WConsoleInterface
{
   HANDLE GetHandle() const
   {
      return GetStdHandle(STD_ERROR_HANDLE);
   }

   HANDLE SetHandle(HANDLE hCon)
   {
      SetStdHandle(STD_ERROR_HANDLE, hCon);
      return GetStdHandle(STD_ERROR_HANDLE);
   }
} wcStdErr;

struct WConsoleCursorInfo : public CONSOLE_CURSOR_INFO
{
   BOOL Get(HANDLE hConsoleOutput)
   {
      return GetConsoleCursorInfo(hConsoleOutput, this);
   }

   BOOL Get(const WConsoleInterface *wcbConsoleOutput)
   {
      return GetConsoleCursorInfo(wcbConsoleOutput->GetHandle(), this);
   }

   BOOL Get()
   {
      HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsoleOutput == INVALID_HANDLE_VALUE)
         return false;
      else
         return GetConsoleCursorInfo(hConsoleOutput, this);
   }

   BOOL Set(HANDLE hConsoleOutput) const
   {
      return SetConsoleCursorInfo(hConsoleOutput, this);
   }

   BOOL Set(const WConsoleInterface *wcbConsoleOutput)
   {
      return SetConsoleCursorInfo(wcbConsoleOutput->GetHandle(), this);
   }

   BOOL Set() const
   {
      HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsoleOutput == INVALID_HANDLE_VALUE)
         return false;
      else
         return SetConsoleCursorInfo(hConsoleOutput, this);
   }

   WConsoleCursorInfo(CONST CONSOLE_CURSOR_INFO &cci)
   {
      *(CONSOLE_CURSOR_INFO*)this = cci;
   }

   WConsoleCursorInfo()
   {
      if (!Get())
         ZeroMemory(this, sizeof *this);
   }

   WConsoleCursorInfo(HANDLE hConsoleOutput)
   {
      if (!Get(hConsoleOutput))
         ZeroMemory(this, sizeof *this);
   }

   WConsoleCursorInfo(const WConsoleInterface *wcbConsoleOutput)
   {
      if (!Get(wcbConsoleOutput))
         ZeroMemory(this, sizeof *this);
   }
};

struct WConsoleScreenBufferInfo : public CONSOLE_SCREEN_BUFFER_INFO
{
   BOOL Get(HANDLE hConsoleOutput)
   {
      return GetConsoleScreenBufferInfo(hConsoleOutput, this);
   }

   BOOL Get(const WConsoleInterface *wcbConsoleOutput)
   {
      return GetConsoleScreenBufferInfo(wcbConsoleOutput->GetHandle(), this);
   }

   BOOL Get()
   {
      HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsoleOutput == INVALID_HANDLE_VALUE)
         return false;
      else
         return GetConsoleScreenBufferInfo(hConsoleOutput, this);
   }

  /*
   BOOL Set(HANDLE hConsoleOutput) const
   {
      return SetConsoleScreenBufferInfo(hConsoleOutput, this);
   }

   BOOL Set(const WConsoleInterface *wcbConsoleOutput)
   {
      return SetConsoleScreenBufferInfo(wcbConsoleOutput->GetHandle(), this);
   }

   BOOL Set() const
   {
      HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsoleOutput == INVALID_HANDLE_VALUE)
         return false;
      else
         return SetConsoleScreenBufferInfo(hConsoleOutput, this);
   }
  */

   WConsoleScreenBufferInfo(CONST CONSOLE_SCREEN_BUFFER_INFO &cci)
   {
      *(CONSOLE_SCREEN_BUFFER_INFO*)this = cci;
   }

   WConsoleScreenBufferInfo()
   {
      if (!Get())
         ZeroMemory(this, sizeof *this);
   }

   WConsoleScreenBufferInfo(HANDLE hConsoleOutput)
   {
      if (!Get(hConsoleOutput))
         ZeroMemory(this, sizeof *this);
   }

   WConsoleScreenBufferInfo(const WConsoleInterface *wcbConsoleOutput)
   {
      if (!Get(wcbConsoleOutput))
         ZeroMemory(this, sizeof *this);
   }
};
#else // __cplusplus

#pragma comment(lib, "winstrct")

#ifndef EXTERN_C
#  define EXTERN_C
#endif

#endif // __cplusplus

EXTERN_C void clreol();
EXTERN_C void clrscr();
EXTERN_C void gotoxy(SHORT x, SHORT y);

#endif // _INC_WCONSOLE_

