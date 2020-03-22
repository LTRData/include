#ifndef _INC_WPROCESS_
#define _INC_WPROCESS_

struct WStartupInfo : public STARTUPINFO
{
   void Fill()
   {
      cb = sizeof(STARTUPINFO);
      GetStartupInfo(this);
   }

   WStartupInfo(CONST STARTUPINFO &si)
   {
      *(STARTUPINFO*)this = si;
   }

   WStartupInfo()
   {
      ZeroMemory(this, sizeof *this);
      cb = sizeof(STARTUPINFO);
   }
};
#endif // _INC_WPROCESS_

