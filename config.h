/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* does your system have a working version of mktime? */
#define HAVE_MKTIME 1

/* does your system have a getpagesize call? */
#define HAVE_GETPAGESIZE 1

/* does your system have the ut_host field in utmp? */
#define HAVE_UT_HOST 1

/* Define if your system has the ut_addr field in utmp. */
#define HAVE_UT_ADDR 1

/* Define if your system has the ut_id field in utmp. */
#define HAVE_UT_ID 1

/* Define if your system has the ut_pid field in utmp. */
#define HAVE_UT_PID 1

/* does your system use System V style utmp records? (uses ut_type field) */
#define HAVE_UT_TYPE 1

/* Define if you have the <sys/acct.h> header file. */
#define HAVE_SYS_ACCT_H 1

/* Define if you have the <linux/acct.h> header file. */
/* #undef HAVE_LINUX_ACCT_H */

/* Define if <sys/acct.h> has the AC_UTIME field. */
#define HAVE_ACUTIME 1

/* Define if <sys/acct.h> has the AC_STIME field. */
#define HAVE_ACSTIME 1

/* Define if <sys/acct.h> has the AC_ETIME field. */
#define HAVE_ACETIME 1

/* Define if <sys/acct.h> has the AC_IO field. */
/* #undef HAVE_ACIO */

/* Define if <sys/acct.h> has the AC_MEM field. */
/* #undef HAVE_ACMEM */

/* Define if <sys/acct.h>'s AC_UTIME field is a COMP_T. */
/* #undef ACUTIME_COMPT */

/* Define if <sys/acct.h>'s AC_STIME field is a COMP_T. */
/* #undef ACSTIME_COMPT */

/* Define if <sys/acct.h>'s AC_ETIME field is a COMP_T. */
/* #undef ACETIME_COMPT */

/* Define if <sys/acct.h>'s AC_IO field is a COMP_T. */
/* #undef ACIO_COMPT */

/* Define if <sys/acct.h>'s AC_MEM field is a COMP_T. */
/* #undef ACMEM_COMPT */

/* Define if <sys/acct.h> uses the COMP_T type. */
/* #undef HAVE_COMP_T */

/* Define if your system has the RENAME system call. */
#define HAVE_RENAME 1

/* Define if your system has prototypes. */
#define PROTOTYPES 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1
