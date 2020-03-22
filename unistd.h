#include <stdlib.h>
#include <io.h>
#include <process.h>

int msleep(unsigned long microseconds);

#define usleep(usec)   msleep(usec/1000)
#define sleep(sec)     ((unsigned long)msleep(sec*1000))
#define ftruncate      chsize
