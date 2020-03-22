#ifndef _WCOMM_H
#define _WCOMM_H

#include <windows.h>

/// Enhanced DCB stucture with encapsulated API functions.
struct WDCB : public DCB
{
   /// Initialize data members using device state.
   BOOL Get(HANDLE hComm)
   {
      return GetCommState(hComm, this);
   }

   /// Initialize device using data members.
   BOOL Set(HANDLE hComm)
   {
      return SetCommState(hComm, this);
   }

   /// Convert pointer to DCB.
   WDCB(DCB &dcb)
   {
      *(DCB*)this = dcb;
   }

   /// Fills data members using MODE command line style parameter string.
   BOOL Build(LPCTSTR lpDef)
   {
      ZeroMemory(this, sizeof *this);
      DCBlength = sizeof(DCB);   // Init length member
      return BuildCommDCB(lpDef, this);
   }

   /// Empty stucture constructor.
   WDCB()
   {
      ZeroMemory(this, sizeof *this);
      DCBlength = sizeof(DCB);   // Init length member
   }

   /// Construct or assign from MODE command line style parameter string.
   WDCB(LPCTSTR lpDef)
   {
      Build(lpDef);
   }

   /// Construct or assign from handle to open device.
   WDCB(HANDLE hComm)
   {
      Get(hComm);
   }
};

/// Enhanced COMMPROP stucture with encapsulated API functions.
struct WCommProp : public COMMPROP
{
   /// Initialize data members using device properties.
   BOOL Get(HANDLE hComm)
   {
      ZeroMemory(this, sizeof *this);
      return GetCommProperties(hComm, this);
   }

   /// Converts pointer to COMMPROP.
   WCommProp(COMMPROP &cp)
   {
      *(COMMPROP*)this = cp;
   }

   /// Empty stucture constructor.
   WCommProp()
   {
      ZeroMemory(this, sizeof *this);
   }

   /// Construct or assign from handle to open device.
   WCommProp(HANDLE hComm)
   {
      Get(hComm);
   }
};

/// Enhanced COMMTIMEOUTS stucture with encapsulated API functions.
struct WCommTimeouts : public COMMTIMEOUTS
{
   /// Initialize data members using device state.
   BOOL Get(HANDLE hComm)
   {
      return GetCommTimeouts(hComm, this);
   }

   /// Initialize device using data members.
   BOOL Set(HANDLE hComm)
   {
      return SetCommTimeouts(hComm, this);
   }

   /// Constructs from COMMCONFIG.
   WCommTimeouts(COMMTIMEOUTS &ct)
   {
      *(COMMTIMEOUTS*)this = ct;
   }

   /// Constructs or assigns from file handle.
   WCommTimeouts(HANDLE hComm)
   {
      Get(hComm);
   }

   /// Empty constructor.
   WCommTimeouts()
   {
      ZeroMemory(this, sizeof *this);
   }
};

/// Enhanced COMMCONFIG stucture with encapsulated API functions.
struct WCommConfig : public COMMCONFIG
{
   /// Initialize data members using device state.
   BOOL Get(HANDLE hComm)
   {
      ZeroMemory(this, sizeof *this);
      dwSize = sizeof(COMMCONFIG);
      return GetCommConfig(hComm, this, &dwSize);
   }

   /// Initialize device using data members.
   BOOL Set(HANDLE hComm)
   {
      return SetCommConfig(hComm, this, sizeof(COMMCONFIG));
   }

   /// Initialize data members using default settings of device name.
   BOOL Get(LPCSTR lpszName)
   {
      ZeroMemory(this, sizeof *this);
      dwSize = sizeof(COMMCONFIG);
      return GetDefaultCommConfig(lpszName, this, &dwSize);
   }

   /// Initialize device defaults (by name) using data members.
   BOOL Set(LPCSTR lpszName)
   {
      return SetDefaultCommConfig(lpszName, this, sizeof(COMMCONFIG));
   }

   /// Construct from COMMCONFIG
   WCommConfig(COMMCONFIG &cc)
   {
      *(COMMCONFIG*)this = cc;
   }

   /// Empty stucture constructor
   WCommConfig()
   {
      ZeroMemory(this, sizeof *this);
   }

   /// Construct or assign from handle to open device.
   WCommConfig(HANDLE hComm)
   {
      Get(hComm);
   }

   /// Construct or assign from name of device.
   WCommConfig(LPCSTR lpszName)
   {
      Get(lpszName);
   }
};
#endif

