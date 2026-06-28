/*
 * syscalls.c
 *
 *  Created on: May 13, 2026
 *      Author: vsafonov
 */


#include <sys/stat.h>
#include <errno.h>


#undef errno
extern int errno;

int _close(int fd)                              { return -1; }
int _fstat(int fd, struct stat *st)             { st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)                             { return 1; }
int _lseek(int fd, int offset, int whence)      { return 0; }
int _read(int fd, char *buf, int len)           { return 0; }
int _write(int fd, const char *buf, int len)    { return len; }
int _getpid(void)                               { return 1; }
int _kill(int pid, int sig)                     { errno = EINVAL; return -1; }
void _exit(int status)                          { while(1) {} }
void *_sbrk(int incr)                           { return (void*)-1; } // heap не используем

