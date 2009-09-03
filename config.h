/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if <sys/acct.h>'s AC_ETIME field is a COMP_T. */
/* #undef ACETIME_COMPT */

/* Define if <sys/acct.h>'s AC_IO field is a COMP_T. */
#define ACIO_COMPT /**/

/* Define if <sys/acct.h>'s AC_MAJFLT field is a COMP_T. */
#define ACMAJFLT_COMPT /**/

/* Define if <sys/acct.h>'s AC_MEM field is a COMP_T. */
#define ACMEM_COMPT /**/

/* Define if <sys/acct.h>'s AC_MINFLT field is a COMP_T. */
#define ACMINFLT_COMPT /**/

/* Define if <sys/acct.h>'s AC_STIME field is a COMP_T. */
#define ACSTIME_COMPT /**/

/* Define if <sys/acct.h>'s AC_SWAPS field is a COMP_T. */
#define ACSWAPS_COMPT /**/

/* Define if <sys/acct.h>'s AC_UTIME field is a COMP_T. */
#define ACUTIME_COMPT /**/

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* 0x80 for big-endian, 0x00 for little-endian. */
#define BYTEORDER 0x00

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define if <sys/acct.h> has the AC_ETIME field. */
#define HAVE_ACETIME /**/

/* Define if <sys/acct.h> has the AC_IO field. */
#define HAVE_ACIO /**/

/* Define if <sys/acct.h> has the AC_MEM field. */
#define HAVE_ACMEM /**/

/* Define if <sys/acct.h> has the AC_STIME field. */
#define HAVE_ACSTIME /**/

/* Define if <sys/acct.h> has the AC_UTIME field. */
#define HAVE_ACUTIME /**/

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define if <sys/acct.h> uses the COMP_T type. */
#define HAVE_COMP_T /**/

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Does your system have getopt? */
#define HAVE_GETOPT /**/

/* Does your system have getopt_long? */
#define HAVE_GETOPT_LONG /**/

/* Does your system have a getpagesize call? */
#define HAVE_GETPAGESIZE /**/

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define if <sys/acct.h> has the AC_MINFLT, AC_MAJFLT and AC_SWAPS fields. */
#define HAVE_PAGING /**/

/* Define if your system has the RENAME system call. */
#define HAVE_RENAME /**/

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the <sys/acct.h> header file. */
/* #undef HAVE_SYS_ACCT_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define if your system has the ut_addr field in utmp. */
#define HAVE_UT_ADDR /**/

/* Does your system have the ut_host field in utmp? */
#define HAVE_UT_HOST /**/

/* Define if your system has the ut_id field in utmp. */
#define HAVE_UT_ID /**/

/* Define if your system has the ut_pid field in utmp. */
#define HAVE_UT_PID /**/

/* Does your system use System V style utmp records? (uses ut_type field) */
#define HAVE_UT_TYPE /**/

/* Define to use "linux-acct.h" instead of <sys/acct.h> */
#define LINUX_MULTIFORMAT 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "acct"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "Markus Gothe <nietzsche@lysator.liu.se"

/* Define to the full name of this package. */
#define PACKAGE_NAME "acct"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "acct 6.5"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "acct"

/* Define to the version of this package. */
#define PACKAGE_VERSION "6.5"

/* Define to 1 if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "6.5"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Work around a bug in Apple GCC 4.0.1 build 5465: In C99 mode, it supports
   the ISO C 99 semantics of 'extern inline' (unlike the GNU C semantics of
   earlier versions), but does not display it by setting __GNUC_STDC_INLINE__.
   __APPLE__ && __MACH__ test for MacOS X.
   __APPLE_CC__ tests for the Apple compiler and its version.
   __STDC_VERSION__ tests for the C99 mode.  */
#if defined __APPLE__ && defined __MACH__ && __APPLE_CC__ >= 5465 && !defined __cplusplus && __STDC_VERSION__ >= 199901L && !defined __GNUC_STDC_INLINE__
# define __GNUC_STDC_INLINE__ 1
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#define restrict __restrict
/* Work around a bug in Sun C++: it does not support _Restrict, even
   though the corresponding Sun C compiler does, which causes
   "#define restrict _Restrict" in the previous line.  Perhaps some future
   version of Sun C++ will work with _Restrict; if so, it'll probably
   define __RESTRICT, just as Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define as a marker that can be attached to function parameter declarations
   for parameters that are not used.  This helps to reduce warnings, such as
   from GCC -Wunused-parameter.  */
#if __GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
# define _UNUSED_PARAMETER_ __attribute__ ((__unused__))
#else
# define _UNUSED_PARAMETER_
#endif


/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
