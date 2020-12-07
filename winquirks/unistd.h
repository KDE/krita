/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QUIRK_UNISTD_H
#define QUIRK_UNISTD_H

#ifndef _MSC_VER
#include <unistd.h>
#else

#pragma message("winquirk: no unistd.h!")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include <windef.h>
#include <winbase.h>
#include <sys/types.h>
#include <sys/stat.h>

#define getpid _getpid
#define popen _popen
#define pclose _pclose
#define ssize_t SSIZE_T

#if 0
static int gethostname(char *__name, size_t __len) {
	DWORD len = __len;
	if (0==GetComputerNameA(__name, &len))
		return -1;
	return 0;
}
#endif


#define environ _environ

/*
#define _IFMT  0170000 // type of file
#define _IFDIR 0040000 // directory
#define S_ISDIR(m) (((m)&_IFMT) == _IFDIR)
*/

#define S_ISUID 0004000
#define S_IRWXU 0000700
#define S_IRWXG 0000070
#define S_IRWXO 0000007
#define S_ISGID 0002000
#define S_ISVTX 0001000

#define S_IRUSR 0000400
#define S_IWUSR 0000200

#define S_IRGRP 0000040
#define S_IWGRP 0000020

#define S_IROTH 0000004
#define S_IWOTH 0000002

#define PATH_SEPARATOR '\\'

#if defined(_MSC_VER)  &&  !defined(S_IREAD)
#   define S_IFMT   _S_IFMT                     /* File type mask */
#   define S_IFDIR  _S_IFDIR                    /* Directory */
#   define S_IFCHR  _S_IFCHR                    /* Character device */
#   define S_IFFIFO _S_IFFIFO                   /* Pipe */
#   define S_IFREG  _S_IFREG                    /* Regular file */
#   define S_IREAD  _S_IREAD                    /* Read permission */
#   define S_IWRITE _S_IWRITE                   /* Write permission */
#   define S_IEXEC  _S_IEXEC                    /* Execute permission */
#endif

#define S_IFFIFO 0
#define S_IFLNK 0
#define S_IFBLK   0
#define S_IFSOCK  0
#define S_IXUSR 0
#define S_IXGRP 0
#define S_IXOTH 0

#if 0
#define S_ISLNK(x) 0
#define S_ISBLK(x) 0
#define S_ISCHR(x) 0
#define S_ISFIFO(x) 0
#define S_ISSOCK(x) 0
#else
#define	S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFFIFO)
#define	S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define	S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define	S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define	S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#define	S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define	S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#endif


#define	F_OK	0
#define	R_OK	4
#define	W_OK	2
#define	X_OK	1

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#if defined _MSC_VER
#if _MSC_VER < 1600
#define ENOTSUP       ENOSYS
#endif
#endif
typedef unsigned int gid_t;
typedef unsigned int uid_t;

#if defined(__MINGW64__) && defined(_WIN64)
/* MINGW_EXTENSION */
typedef long long int pid_t;
/* "long long int" is an alternative to __int64 */
#else
typedef int mode_t;
typedef int pid_t;
#endif


#define getgroups(x,y) 0

static uid_t geteuid() {
	return -2;
}

static uid_t getuid() {
	return -2;
}

static int readlink(const char *__path, char *__buf, int __buflen)
{
    if (!__path) {
      errno = EINVAL;
      return -1;
    }
    if ( (__buflen < 0) || ((int)strlen(__path)>(__buflen-1)) )
    {
      errno = ENAMETOOLONG;
      return -1;
    }
    if (access(__path, R_OK) == 0) {
      /* ok, copy to buf */
      strncpy(__buf,__path,__buflen);
      errno = 0;
      return 0;
    }
    errno = ENOENT;
    return -1;
}


#ifdef __cplusplus
extern "C" {

#if 0
int setenv(const char *name, const char *value, int overwrite);
int unsetenv (const char *name);
#endif

}
#endif  /* __cplusplus */


static int sleep(unsigned int sec) {
	Sleep(sec*1000);
	return 0;
}

#endif
#endif
