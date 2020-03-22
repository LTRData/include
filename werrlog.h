#ifndef _INC_WERRLOG_
#define _INC_WERRLOG_

EXTERN_C void LogMsg(LPCSTR szMsgStr, ...);
EXTERN_C void LogErr(LPCSTR szMsgStr, ...);
EXTERN_C void LogWinErr(LPCSTR szMsgStr, ...);
EXTERN_C void LogSockErr(LPCSTR szMsgStr, ...);

#endif // _INC_WERRLOG_
