#pragma once

#ifdef _NTDEF
EXTERN_C
POBJECT_NAME_INFORMATION
PathCombineWithRoot(LPCWSTR root_dir, LPCWSTR file);
#endif

EXTERN_C
LPWSTR
PathCombineWithRootW(LPCWSTR root_dir, LPCWSTR file);

EXTERN_C
LPSTR
PathCombineWithRootA(LPCSTR root_dir, LPCSTR file);
