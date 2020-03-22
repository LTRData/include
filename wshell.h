#ifndef _INC_WSHELL_
#define _INC_WSHELL_

#include <windows.h>
#include <shellapi.h>

// Encapsulated NOTIFYICONDATA
struct WNotifyIconData : public NOTIFYICONDATA
{
   BOOL Add()
   {
      return Shell_NotifyIcon(NIM_ADD, this);
   }

   BOOL Delete()
   {
      return Shell_NotifyIcon(NIM_DELETE, this);
   }

   BOOL Modify()
   {
      return Shell_NotifyIcon(NIM_MODIFY, this);
   }

   void SetIcon(HICON __hIcon)
   {
      if (__hIcon)
      {
         hIcon = __hIcon;
         uFlags |= NIF_ICON;
      }
      else
      {
         hIcon = __hIcon;
         uFlags &= ~NIF_ICON;
      }
   }

   void SetMessage(UINT __uCallbackMessage)
   {
      if (__uCallbackMessage)
      {
         uCallbackMessage = __uCallbackMessage;
         uFlags |= NIF_MESSAGE;
      }
      else
      {
         uCallbackMessage = __uCallbackMessage;
         uFlags &= ~NIF_MESSAGE;
      }
   }

   void SetTip(const char *__szTip)
   {
      if (__szTip)
      {
         strncpyn(szTip, __szTip, sizeof szTip);
	 szTip[sizeof(szTip)-1] = 0;
         uFlags |= NIF_TIP;
      }
      else
      {
         szTip[0] = '\x00';
         uFlags &= ~NIF_TIP;
      }
   }

   WNotifyIconData(HWND __hWnd, UINT __uID = 0)
   {
      cbSize = sizeof(NOTIFYICONDATA);
      hWnd = __hWnd;
      uID = __uID;
   }
};
#endif // _INC_WSHELL_
