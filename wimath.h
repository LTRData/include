#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif

#ifndef CDECL
#ifdef _WIN32
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

EXTERN_C int CDECL gcd(int, int);
EXTERN_C int CDECL getprimefactors(int, void *, int (CDECL *f)(void *,int));
