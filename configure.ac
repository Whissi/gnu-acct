dnl Process this file with autoconf to produce a configure script.

dnl init autoconf
AC_REQ(2.63)
AC_INIT([acct], [6.5], [Markus Gothe <nietzsche@lysator.liu.se])
AC_CONFIG_SRCDIR(ac.c)
AC_CANONICAL_TARGET
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

dnl macros for automake
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
gl_EARLY
gl_INIT
if test "x${ac_cv_prog_gcc}" = "xyes"; then
 CFLAGS="${CFLAGS} -Wall -Wmissing-prototypes"
fi
AC_AIX
AC_MINIX
AC_HEADER_STDC
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_CC_C89
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_RANLIB

AC_ISC_POSIX

AC_C_PROTOTYPES
AC_C_CONST
AC_C_RESTRICT
AC_C_VOLATILE
AC_SYS_POSIX_TERMIOS

AC_FUNC_ALLOCA
AC_TYPE_SIGNAL
AC_CHECK_FUNC(getopt, AC_DEFINE(HAVE_GETOPT, ,[Does your system have getopt?]))
AC_CHECK_FUNC(getopt_long, AC_DEFINE(HAVE_GETOPT_LONG, ,[Does your system have getopt_long?]))
dnl AC_HEADER_MAJOR
AC_FUNC_MKTIME
dnl AC_CHECK_FUNC(mktime,
dnl	      AC_DEFINE(HAVE_MKTIME, ,
dnl			[Does your system have a working version of mktime?])
dnl              MKTIME_C="" MKTIME_O="" )
AC_CHECK_FUNC(getpagesize, AC_DEFINE(HAVE_GETPAGESIZE, ,
				     [Does your system have a getpagesize call?]))
AC_CHECK_FUNC(rename, AC_DEFINE(HAVE_RENAME, ,
				[Define if your system has the RENAME system call.]))
AC_CHECK_HEADERS(limits.h stdlib.h unistd.h string.h)
dnl
dnl crunky time stuff
dnl
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/time.h)
dnl
dnl find out where various programs reside on this system
dnl
AC_PATH_PROGS(SYS_LAST, last, echo, /bin /etc /sbin /usr/bin /usr/etc /usr/sbin /usr/ucb /usr/local/bin /usr/local/etc /usr/local/sbin )
AC_PATH_PROGS(SYS_LASTCOMM, lastcomm, echo, /bin /etc /sbin /usr/bin /usr/etc /usr/sbin /usr/ucb /usr/local/bin /usr/local/etc /usr/local/sbin )
AC_PATH_PROGS(SYS_AC, ac, echo, /bin /etc /sbin /usr/bin /usr/etc /usr/sbin /usr/ucb /usr/local/bin /usr/local/etc /usr/local/sbin )
AC_PATH_PROGS(SYS_SA, sa, echo, /bin /etc /sbin /usr/bin /usr/etc /usr/sbin /usr/ucb /usr/local/bin /usr/local/etc /usr/local/sbin )
dnl
dnl find out what fields are valid in utmp.h
dnl
AC_EGREP_HEADER(ut_id, utmp.h, AC_DEFINE(HAVE_UT_ID, ,
					 [Define if your system has the ut_id field in utmp.]), )
AC_EGREP_HEADER(ut_pid, utmp.h, AC_DEFINE(HAVE_UT_PID, ,
					  [Define if your system has the ut_pid field in utmp.]), )
AC_EGREP_HEADER(ut_host, utmp.h, AC_DEFINE(HAVE_UT_HOST, ,
					   [Does your system have the ut_host field in utmp?]), )
AC_EGREP_HEADER(ut_addr, utmp.h, AC_DEFINE(HAVE_UT_ADDR, ,
					   [Define if your system has the ut_addr field in utmp.]), )
AC_EGREP_HEADER(ut_type, utmp.h, AC_DEFINE(HAVE_UT_TYPE, ,
					   [Does your system use System V style utmp records? (uses ut_type field)]), )
dnl
dnl
dnl
AC_ARG_ENABLE([linux-multiformat],
	      AC_HELP_STRING([--enable-linux-multiformat],
			     [support various predefined linux acct file formats ]
			     [instead of the one defined in <sys/acct.h>]),
	      [],
	      [
		case $target in
		*-linux*)
			enable_linux_multiformat=yes ;;
		*)
			enable_linux_multiformat=no ;;
		esac
	      ]
	      )
AS_IF([test $enable_linux_multiformat = yes],
      AC_DEFINE(LINUX_MULTIFORMAT,1,
		[Define to use "linux-acct.h" instead of <sys/acct.h>])
      DUMP_ACCT_PROG=dump-acct
      ACCTON_PROG=accton
      LASTCOMM_PROG=lastcomm
      SA_PROG=sa
      ACCTON_MAN=accton.8
      LASTCOMM_MAN=lastcomm.1
      SA_MAN=sa.8
      AC_DEFINE(HAVE_ACUTIME, ,
	        [Define if <sys/acct.h> has the AC_UTIME field.])
      AC_DEFINE(ACUTIME_COMPT, ,
		[Define if <sys/acct.h>'s AC_UTIME field is a COMP_T.])
      AC_DEFINE(HAVE_ACSTIME, ,
		[Define if <sys/acct.h> has the AC_STIME field.])
      AC_DEFINE(ACSTIME_COMPT, ,
		[Define if <sys/acct.h>'s AC_STIME field is a COMP_T.])
      AC_DEFINE(HAVE_ACETIME, ,
		[Define if <sys/acct.h> has the AC_ETIME field.])
      AC_DEFINE(HAVE_ACIO, ,
		[Define if <sys/acct.h> has the AC_IO field.])
      AC_DEFINE(ACIO_COMPT, ,
		[Define if <sys/acct.h>'s AC_IO field is a COMP_T.])
      AC_DEFINE(HAVE_ACMEM, ,
		[Define if <sys/acct.h> has the AC_MEM field.])
      AC_DEFINE(ACMEM_COMPT, ,
		[Define if <sys/acct.h>'s AC_MEM field is a COMP_T.])
      AC_DEFINE(HAVE_PAGING, ,
		[Define if <sys/acct.h> has the AC_MINFLT, AC_MAJFLT and AC_SWAPS fields.])
      AC_DEFINE(ACMINFLT_COMPT, ,
		[Define if <sys/acct.h>'s AC_MINFLT field is a COMP_T.])
      AC_DEFINE(ACMAJFLT_COMPT, ,
		[Define if <sys/acct.h>'s AC_MAJFLT field is a COMP_T.])
      AC_DEFINE(ACSWAPS_COMPT, ,
		[Define if <sys/acct.h>'s AC_SWAPS field is a COMP_T.])
      AC_DEFINE(HAVE_COMP_T, ,
		[Define if <sys/acct.h> uses the COMP_T type.])
      , [
dnl
dnl figure out where acct.h lives
dnl and whether fields are int/comp_t
dnl
AC_CHECK_HEADER(sys/acct.h,
		AC_DEFINE(HAVE_SYS_ACCT_H, ,
			  [Define if you have the <sys/acct.h> header file.])
		DUMP_ACCT_PROG=dump-acct
		ACCTON_PROG=accton
		LASTCOMM_PROG=lastcomm
		SA_PROG=sa
		ACCTON_MAN=accton.8
		LASTCOMM_MAN=lastcomm.1
		SA_MAN=sa.8
		AC_HEADER_EGREP(ac_utime, sys/acct.h,
				AC_DEFINE(HAVE_ACUTIME, ,
					  [Define if <sys/acct.h> has the AC_UTIME field.])
				AC_HEADER_EGREP(comp_t.*ac_utime, sys/acct.h,
						AC_DEFINE(ACUTIME_COMPT, ,
							  [Define if <sys/acct.h>'s AC_UTIME field is a COMP_T.]))
		)
		AC_HEADER_EGREP(ac_stime, sys/acct.h,
				AC_DEFINE(HAVE_ACSTIME, ,
					  [Define if <sys/acct.h> has the AC_STIME field.])
				AC_HEADER_EGREP(comp_t.*ac_stime, sys/acct.h,
						AC_DEFINE(ACSTIME_COMPT, ,
							  [Define if <sys/acct.h>'s AC_STIME field is a COMP_T.]))
		)
		AC_HEADER_EGREP(ac_etime, sys/acct.h,
				AC_DEFINE(HAVE_ACETIME, ,
					  [Define if <sys/acct.h> has the AC_ETIME field.])
				AC_HEADER_EGREP(comp_t.*ac_etime, sys/acct.h,
						AC_DEFINE(ACETIME_COMPT, ,
							  [Define if <sys/acct.h>'s AC_ETIME field is a COMP_T.]))
		)
		AC_HEADER_EGREP(ac_io,    sys/acct.h,
				AC_DEFINE(HAVE_ACIO, ,
					  [Define if <sys/acct.h> has the AC_IO field.])
				AC_HEADER_EGREP(comp_t.*ac_io,    sys/acct.h,
						AC_DEFINE(ACIO_COMPT, ,
							  [Define if <sys/acct.h>'s AC_IO field is a COMP_T.]))
		)
		AC_HEADER_EGREP(ac_mem,   sys/acct.h,
				AC_DEFINE(HAVE_ACMEM, ,
					  [Define if <sys/acct.h> has the AC_MEM field.])
				AC_HEADER_EGREP(comp_t.*ac_mem,   sys/acct.h,
						AC_DEFINE(ACMEM_COMPT, ,
							  [Define if <sys/acct.h>'s AC_MEM field is a COMP_T.]))
		)
		AC_HEADER_EGREP(ac_minflt,   sys/acct.h,
				AC_HEADER_EGREP(ac_majflt,   sys/acct.h,
						AC_HEADER_EGREP(ac_swaps,   sys/acct.h,
								AC_DEFINE(HAVE_PAGING, ,
									  [Define if <sys/acct.h> has the AC_MINFLT, AC_MAJFLT and AC_SWAPS fields.])
								AC_HEADER_EGREP(comp_t.*ac_minflt, sys/acct.h,
										AC_DEFINE(ACMINFLT_COMPT, ,
											  [Define if <sys/acct.h>'s AC_MINFLT field is a COMP_T.]))
								AC_HEADER_EGREP(comp_t.*ac_mayflt, sys/acct.h,
										AC_DEFINE(ACMAJFLT_COMPT, ,
											  [Define if <sys/acct.h>'s AC_MAJFLT field is a COMP_T.]))
								AC_HEADER_EGREP(comp_t.*ac_swaps, sys/acct.h,
										AC_DEFINE(ACSWAPS_COMPT, ,
											  [Define if <sys/acct.h>'s AC_SWAPS field is a COMP_T.]))
						)
				)
		)
		AC_HEADER_EGREP(comp_t,   sys/acct.h, AC_DEFINE(HAVE_COMP_T, ,
								[Define if <sys/acct.h> uses the COMP_T type.]))
  ) ]
)
dnl
dnl find out where utmp/pacct are stored
dnl
AC_TRY_RUN(
[
#include <stdio.h>
#include <sys/types.h>
#include <sys/acct.h>
#include <utmp.h>

#ifndef WTMP_FILE
#  if defined(__FreeBSD__) || defined (__NetBSD__) || defined(__linux__)
#    define WTMP_FILE "/var/log/wtmp"
#  else
#    if defined(sun) || defined(AMIX)
#      define WTMP_FILE "/var/adm/wtmp"
#    else
#      if defined(sgi) || defined(SVR4)
#        define WTMP_FILE "/usr/adm/wtmp"
#      else
#        define WTMP_FILE "/usr/adm/wtmp"
#      endif
#    endif
#  endif
#endif

#ifndef ACCT_FILE
#  if defined(__FreeBSD__) || defined(__linux__)
#    define ACCT_FILE "/var/log/account/pacct"
#  else
#    if defined(__NetBSD__)
#      define ACCT_FILE "/var/log/account/acct"
#    else
#      if defined(sun) || defined(AMIX)
#        define ACCT_FILE "/var/adm/pacct"
#      else
#        if defined(sgi) || defined(SVR4) || defined(M_XENIX)
#          define ACCT_FILE "/usr/adm/pacct"
#        else
#          define ACCT_FILE "/usr/adm/acct"
#        endif
#      endif
#    endif
#  endif
#endif

#ifndef SAVACCT_FILE
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__linux__)
#    define SAVACCT_FILE "/var/log/account/savacct"
#  else
#    if defined(sun) || defined(AMIX)
#      define SAVACCT_FILE "/var/adm/savacct"
#    else
#      if defined(sgi) || defined(SVR4)
#        define SAVACCT_FILE "/usr/adm/savacct"
#      else
#        define SAVACCT_FILE "/usr/adm/savacct"
#      endif
#    endif
#  endif
#endif

#ifndef USRACCT_FILE
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__linux__)
#    define USRACCT_FILE "/var/log/account/usracct"
#  else
#    if defined(sun) || defined(AMIX)
#      define USRACCT_FILE "/var/adm/usracct"
#    else
#      if defined(sgi) || defined(SVR4)
#        define USRACCT_FILE "/usr/adm/usracct"
#      else
#        define USRACCT_FILE "/usr/adm/usracct"
#      endif
#    endif
#  endif
#endif

main ()
{
  FILE *fp;
  fp = fopen ("locs", "w");
  fprintf (fp, "WTMP_FILE_LOC=%s\n", WTMP_FILE);
  fprintf (fp, "ACCT_FILE_LOC=%s\n", ACCT_FILE);
  fprintf (fp, "SAVACCT_FILE_LOC=%s\n", SAVACCT_FILE);
  fprintf (fp, "USRACCT_FILE_LOC=%s\n", USRACCT_FILE);
  fclose (fp);
  exit (0);
}
], . ./locs; rm locs,
   echo "Error -- could not locate your wtmp and acct files."; exit 1,
   echo "Sorry -- you cannot cross-compile this package (FIXME)."; exit 1 )
AC_TYPE_PID_T dnl for sys/acct.h
AC_TYPE_UID_T dnl same as above
AC_TYPE_SIZE_T
AC_HEADER_DIRENT
AC_C_BIGENDIAN(AC_DEFINE(BYTEORDER,0x80,
			 [0x80 for big-endian, 0x00 for little-endian.]),
               AC_DEFINE(BYTEORDER,0x00,
			 [0x80 for big-endian, 0x00 for little-endian.]),)
echo checking for kitchen sink... You find a ring in the sink!

dnl Substitute program names for automake
AC_SUBST(DUMP_ACCT_PROG)
AC_SUBST(ACCTON_PROG)
AC_SUBST(LASTCOMM_PROG)
AC_SUBST(SA_PROG)

dnl Substitute manual page names for automake
AC_SUBST(ACCTON_MAN)
AC_SUBST(LASTCOMM_MAN)
AC_SUBST(SA_MAN)

dnl Substitutions for mktime
AC_SUBST(MKTIME_C)
AC_SUBST(MKTIME_O)

dnl Substitutions for file locations
AC_SUBST(WTMP_FILE_LOC)
AC_SUBST(ACCT_FILE_LOC)
AC_SUBST(SAVACCT_FILE_LOC)
AC_SUBST(USRACCT_FILE_LOC)

dnl Dump the makefiles and etc.
AC_OUTPUT(Makefile lib/Makefile files.h version.h, [test -z "$CONFIG_HEADERS" || echo timestamp > stamp-h])
