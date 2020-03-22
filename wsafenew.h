#if !defined(_INC_WSAFENEW_) && !defined(_M_ARM64)
#define _INC_WSAFENEW_

inline void * operator new[](size_t siz)
{
  void *memblk = malloc(siz);
  if (memblk != NULL)
    return memblk;

  abort();

#if _MSC_VER < 1400
  return NULL;
#endif
}

inline void * operator new(size_t siz)
{
  void *memblk = malloc(siz);
  if (memblk != NULL)
    return memblk;

  abort();

#if _MSC_VER < 1400
  return NULL;
#endif
}

inline void operator delete[](void *memblk)
{
  free(memblk);
}

inline void operator delete(void *memblk)
{
  free(memblk);
}

#endif // _INC_WSAFENEW_
