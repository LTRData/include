
#ifdef _M_IX86

extern void(__stdcall *YieldSingleProcessor)();

#else

#define YieldSingleProcessor()

#endif
