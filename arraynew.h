#ifndef HAS_ARRAY_OPERATOR_NEW
#define HAS_ARRAY_OPERATOR_NEW

inline void *operator new[](size_t size)
{
  return operator new(size);
}

inline void operator delete[](void *ptr)
{
  operator delete(ptr);
}

#endif
