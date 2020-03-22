#ifndef _INC_WSTRING_H
#define _INC_WSTRING_H

#ifndef EXTERN_C
#  ifdef _cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C
#  endif
#endif

#ifdef _MSC_VER
#  pragma comment(lib, "winstrct.lib")
#endif

#define KEY_HOME        0x4700
#define KEY_END         0x4F00
#define KEY_PGUP        0x4900
#define KEY_PGDN        0x5100
#define KEY_LEFT        0x4B00
#define KEY_RIGHT       0x4D00
#define KEY_UP          0x4800
#define KEY_DOWN        0x5000
#define KEY_CTRLENTER   0x000A
#define KEY_ENTER       0x000D
#define KEY_ESCAPE      0x001B
#define KEY_INSERT      0x5200
#define KEY_DELETE      0x5300
#define KEY_BACKSP      0x0008
#define KEY_CTRLINSERT  0x9200
#define KEY_CTRLDELETE  0x9300
#define KEY_CTRLZ       0x001A
#define KEY_ALTX        0x2D00
#define KEY_ALTBACKSP   0x0E00

EXTERN_C char *strltrm(char *);
EXTERN_C char *strrtrm(char *);
EXTERN_C char *strtrm(char *);
EXTERN_C char *memstr(char *, const char *, size_t);
EXTERN_C LPWSTR memwcs(LPWSTR, LPCWSTR, size_t);

#endif // _INC_WSTRING_
