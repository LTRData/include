#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#include <sys/types.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define MAP_VARIABLE	 0x0000	/* map anywhere (antonym for MAP_FIXED) */
#define	MAP_FIXED	 0x0010	/* map addr must be exactly as requested */
#define	MAP_RENAME	 0x0020	/* Sun: rename private pages to file */
#define	MAP_NORESERVE	 0x0040	/* Sun: don't reserve needed swap area */
#define	MAP_INHERIT	 0x0080	/* region is retained after exec */
#define	MAP_NOEXTEND	 0x0100	/* for MAP_FILE, don't change file size */
#define	MAP_HASSEMAPHORE 0x0200	/* region may contain semaphores */

#define	PROT_NONE	0x00	/* no access allowed */
#define	PROT_READ	0x01	/* pages can be read */
#define	PROT_WRITE	0x02	/* pages can be written */
#define	PROT_EXEC	0x04	/* pages can be executed */

#define	MAP_SHARED	0x0001	/* share changes */
#define	MAP_PRIVATE	0x0002	/* changes are private */
#define	MAP_COPY	0x0004	/* "copy" region at mmap time */
#define MAP_FAILED  ((void *)(((char *) 0) - 1))

#define MS_SYNC	    	0x01
#define MS_ASYNC    	0x02
#define MS_INVALIDATE	0x04

void * mmap(void *, size_t, int, int, int, off_t);
int    munmap(void *, size_t);

#if defined(__cplusplus)
}
#endif

#endif

