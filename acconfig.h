/* acconfig.h
 *
 * miscellaneous definitions for autoheader, so that I don't have to
 * create the damn config.h.in file by hand EVERY TIME I CHANGE
 * SOMETHING.  Leave the ^L in on the next line. */


/* does your system have a working version of mktime? */
#undef HAVE_MKTIME

/* does your system have a getpagesize call? */
#undef HAVE_GETPAGESIZE

/* does your system have the ut_host field in utmp? */
#undef HAVE_UT_HOST

/* Define if your system has the ut_addr field in utmp. */
#undef HAVE_UT_ADDR

/* Define if your system has the ut_id field in utmp. */
#undef HAVE_UT_ID

/* Define if your system has the ut_pid field in utmp. */
#undef HAVE_UT_PID

/* does your system use System V style utmp records? (uses ut_type field) */
#undef HAVE_UT_TYPE

/* Define if you have the <sys/acct.h> header file. */
#undef HAVE_SYS_ACCT_H

/* Define if you have the <linux/acct.h> header file. */
#undef HAVE_LINUX_ACCT_H

/* Define if <sys/acct.h> has the AC_UTIME field. */
#undef HAVE_ACUTIME

/* Define if <sys/acct.h> has the AC_STIME field. */
#undef HAVE_ACSTIME

/* Define if <sys/acct.h> has the AC_ETIME field. */
#undef HAVE_ACETIME

/* Define if <sys/acct.h> has the AC_IO field. */
#undef HAVE_ACIO

/* Define if <sys/acct.h> has the AC_MEM field. */
#undef HAVE_ACMEM

/* Define if <sys/acct.h>'s AC_UTIME field is a COMP_T. */
#undef ACUTIME_COMPT

/* Define if <sys/acct.h>'s AC_STIME field is a COMP_T. */
#undef ACSTIME_COMPT

/* Define if <sys/acct.h>'s AC_ETIME field is a COMP_T. */
#undef ACETIME_COMPT

/* Define if <sys/acct.h>'s AC_IO field is a COMP_T. */
#undef ACIO_COMPT

/* Define if <sys/acct.h>'s AC_MEM field is a COMP_T. */
#undef ACMEM_COMPT

/* Define if <sys/acct.h> uses the COMP_T type. */
#undef HAVE_COMP_T

/* Define if your system has the RENAME system call. */
#undef HAVE_RENAME

/* Define if your system has prototypes. */
#undef PROTOTYPES


/* Hey!  Hey YOU!  Leave that ^L in on the previous line */
