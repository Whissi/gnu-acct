/* al_share.cpp
 *
 * code shared between ac & last
 *
 * to get the code for last, #define BACKWARDS before #including this
 * file. */


/* This is the loop that handles the parsing of entries.  Since it's
   virtually identical for ac & last, I figured it was better to have
   only one copy of it to reduce the possiblity of errors. */

void
parse_entries ()
{
  struct utmp *rec;		/* the current record */
  int expecting_clock = 0;	/* set to a nonzero value if the next
				   entry is expected to be the second
				   half of a clock pair */
  time_t clock_before = 0;	/* the clock before the date cmd */
  short first_time = 1;

  /* loop while there are still entries to read... */

  while ((rec = utmp_get_entry ()) != NULL)
    {	  
      /* Munge the record for those systems that have a bsd/sysv mix
         of records. */

      if (debugging_enabled)
	{
	  fprintf (stddebug, "---------------------------------------------------------------------------\nCURRENT REC: ");
	  print_utmp_record (rec, stddebug);
	}

#if HAVE_UT_TYPE
      if (fix_ut_type_field (rec) && debugging_enabled)
	{
	  fprintf (stddebug, " MUNGED REC: ");
	  print_utmp_record (rec, stddebug);
	}
#endif

      /* Throw away obviously bad records. */

      if (bad_utmp_record (rec))
	{
	  if (print_file_problems)
	    {
	      utmp_print_file_and_line (stddebug);
	      fprintf (stddebug, ": problem: bad record\n");
	    }
	  continue;		/* skip it */
	}

      if (first_time)
	{
	  first_time = 0;

#ifdef BACKWARDS

	  /* last: The first time through, we need to set up the
	     LAST_TIME variable correctly.  We also set up the signal
	     handler here (rather than somewhere earlier) so we're
	     guaranteed to get a good value for LAST_TIME if the
	     program gets interrupted. */

	  last_time = rec->ut_time + 1;
	  (void) signal (SIGINT, (RETSIGTYPE (*)) handler);
#else
	  /* ac: if this is the first time, we need to set up
	     next_midnight correctly, so that do_totals will function
	     correctly.  */

	  next_midnight = midnight_after_me (rec->ut_time);
#endif
	}

#ifdef BACKWARDS

      /* last: if the time of this record occurs AFTER the time of the
	 last record, there's something wrong.  To deal with it, log
	 everyone in at the time of the record.  Unfortunately, this
	 means that statistics cannot be collected for the users on
	 these ttys...  Also, reset last_time so THIS check can be
	 made correctly.  Similarly, if we see a huge jump forward in
	 time, there's something wrong.  If we're being nasty, go
	 ahead and assign the time to the user.  Otherwise, clear out
	 the list of logins.  NOTE: the '|' entry is a special case,
	 so ignore it.  */

      if ((rec->RECORD_TYPE != NEW_TIME)
	  && ((rec->ut_time - time_warp_leniency > last_time)
	      || (rec->ut_time < last_time - time_warp_suspicious)))

#else

      /* ac: if the time of this record occurs BEFORE the time of the
	 last record, there's something wrong.  To deal with it, log
	 everyone out at the next midnight, doing the totals at that
	 time.  Then reset NEXT_MIDNIGHT so the next do_totals call is
	 correct.  Also, reset LAST_TIME so this check can be made
	 correctly.  Similarly, if we see a huge jump forward in time,
	 there's something wrong.  If we're being nasty, go ahead and
	 assign the time to the user.  Otherwise, clear out the list
	 of logins.  NOTE: the '{' entry is a special case, so ignore
	 it. */

      if ((rec->RECORD_TYPE != NEW_TIME)
	  && ((rec->ut_time + time_warp_leniency < last_time)
	      || (rec->ut_time > last_time + time_warp_suspicious)))

#endif
	{
	  if (print_file_problems)
	    {
	      utmp_print_file_and_line (stddebug);
	      fprintf (stddebug, ": problem: time warp (%-24.24s",
		       ctime (&last_time));
	      fprintf (stddebug, " -> %-24.24s)\n",
		       ctime ((time_t *) &(rec->ut_time)));
	    }

#ifdef BACKWARDS
	  /* last: update the event stuff */

	  last_time = rec->ut_time;
	  last_event = "warp";
	  log_everyone_in (rec->ut_time);
#else
	  /* ac: force an update to occur by passing two times which
             are one second apart -- DO_TOTALS does it's thing if the
             first argument is less than the second. */

	  do_totals (&next_midnight, next_midnight + 1,
		     nasty_time_warp, FALSE, "time warp");

	  /* Fix up NEXT_MIDNIGHT to reflect our new time base. */
	  
	  next_midnight = midnight_after_me (rec->ut_time);
#endif
	}

      last_time = rec->ut_time; /* so we're ready to run next time */

#ifndef BACKWARDS
      /* ac: if we're doing totals every midnight, we have to check
	 and see if the current record occurs past the next midnight.
	 If it does, print out the totals for the evening and
	 increment the NEXT_MIDNIGHT variable.  Continue until the
	 record is before our midnight.  */

      if (rec->RECORD_TYPE != NEW_TIME)
	do_totals (&next_midnight, (time_t) rec->ut_time,
		   TRUE, TRUE, "midnight logout");
#endif

      /* Process each record, depending on its type. */

      switch (rec->RECORD_TYPE)
	{
#ifdef BACKWARDS
	case OLD_TIME:
#else
	case NEW_TIME:
#endif

	  /* Change the time if we've already seen the other half of
             the date change command. */

	  if (expecting_clock)
	    {
#ifdef BACKWARDS
#ifdef HAVE_UT_TYPE
	      if (print_which_recs >= 1)
#endif
		print_record (rec, 0, NULL, "old time", "");
#endif
	      update_system_time (rec->ut_time - clock_before);
	      expecting_clock = 0;
#ifndef BACKWARDS
	      next_midnight = midnight_after_me (rec->ut_time);
#endif
	    }
	  else if (print_file_problems)
	    {
	      utmp_print_file_and_line (stddebug);
	      fprintf (stddebug,
		       ": problem: unpaired time record (ignored)\n");
#ifdef BACKWARDS
	      print_record (rec, 0, NULL, "old time", "");
#endif
	    }

	  break;

#ifdef BACKWARDS
	case NEW_TIME:
#else
	case OLD_TIME:
#endif

	  /* This is the first half of a time change that we should
             see. */

#ifdef BACKWARDS
#ifdef HAVE_UT_TYPE
	  if (print_which_recs >= 1)
#endif
	    print_record (rec, 0, NULL, "new time", "");
#endif
	  expecting_clock = 1;
	  clock_before = rec->ut_time;

	  break;

	case BOOT_TIME:

	  if (strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
	    {
#if defined (HAVE_UT_TYPE) && (defined (RUN_LVL) || defined (BACKWARDS))
	    do_shutdown:
#endif

#ifdef BACKWARDS
#ifdef HAVE_UT_TYPE
	      if (print_which_recs >= 1)
#endif
		print_record (rec, 0, NULL, "system down", "");

#if defined (HAVE_UT_TYPE) && defined (RUN_LVL) && defined (__linux__)
	    do_shutdown_part_2:
#endif
	      last_event = "down";
	      last_event_time = rec->ut_time;
	      log_everyone_in (rec->ut_time);

#else /* ! BACKWARDS */

	      log_everyone_out (rec->ut_time, TRUE, FALSE, "shutdown");

#endif
	    }
	  else
	    {
#ifdef BACKWARDS
	      print_record (rec, 0, NULL, "system boot", "");
	      last_event = "crash";
	      last_event_time = rec->ut_time;
	      log_everyone_in (rec->ut_time);
#else
	      /* The system was rebooted at the given time, so we
		 don't know when it really went down.  Let's be nice
		 to the users, and don't count the time against their
		 total time.  Just clear the current logins!  The
		 NASTY_REBOOT flag is put in for compatibility...  */

	      log_everyone_out (rec->ut_time, nasty_reboot, FALSE, "reboot");
#endif
	    }

	  break;
	  
	  /* Handle those types which simply print things out, rather
	     than linking up logins/logouts. */

#ifdef HAVE_UT_TYPE		/* just in case */

#ifdef RUN_LVL

	case RUN_LVL:

	  /* This should be either "shutdown" (a special case) or a
             runlevel change record. */
	  
#ifdef BACKWARDS

	  if (strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
	    goto do_shutdown;
	  else
	    {
#ifdef __linux__
	      /* The init program by Miquel van Smoorenburg
                 <miquels@drinkel.ow.org> writes its runlevel to the
                 UT_PID field. */

	      char s[12], lvl = rec->ut_pid & 255;
	      sprintf (s, "(to lvl %c)", lvl);
	      
	      /* Note that this will print if we specify the `-x' flag
                 to be compatible with Debian. */

	      if (print_which_recs >= 1)
		print_record (rec, 0, NULL, s, "");

	      if ((lvl == '0') || (lvl == '6'))
		goto do_shutdown_part_2;
#else /* ! __linux__ */
	      if (print_which_recs >= 2)
		print_record (rec, 0, NULL, NULL, "");
#endif /* ! __linux__ */

	    }

#else /* ! BACKWARDS */

	  if (strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
	    goto do_shutdown;

#ifdef __linux__
	  /* The init program by Miquel van Smoorenburg
	     <miquels@drinkel.ow.org> writes its runlevel to the
	     UT_PID field. */
	  
	  {
	    char lvl = rec->ut_pid & 255;
	      
	    if ((lvl == '0') || (lvl == '6'))
	      goto do_shutdown;
	  }
#endif /* __linux__ */

	  /* Otherwise ignore this. */

#endif /* ! BACKWARDS */

	  break;
#endif

#ifdef LOGIN_PROCESS
	  /* This is a record written by the "login" program as it
	     waits for input on a particular tty. */

	case LOGIN_PROCESS:
#ifdef BACKWARDS
	  if (print_which_recs >= 2)
	    print_record (rec, 0, "(login)", NULL, "");
#endif
	  break;
#endif

#ifdef INIT_PROCESS
	  /* This is a record written by the "init" program for things
             other than run level changes. */

	case INIT_PROCESS:
#ifdef BACKWARDS
	  if (print_which_recs >= 2)
	    print_record (rec, 0, "(init)", NULL, "");
#endif
	  break;
#endif

#ifdef ACCOUNTING
        case ACCOUNTING:
#ifdef BACKWARDS
	  if (print_which_recs >= 2)
	    print_record (rec, 0, "(acct)", NULL, "");
#endif
          break;
#endif

#ifdef EMPTY
	case EMPTY:
#ifdef BACKWARDS
	  if (print_which_recs >= 2)
	    print_record (rec, 0, "(empty)", NULL, "");
#endif
	  break;	
#endif

	  /* Handle those types which are counted as logins/logouts. */

#ifdef USER_PROCESS
	  /* A record written for a regular login.  If the system
             doesn't have DEAD_PROCESS, we might see a logout here, so
             check UT_NAME. */

	case USER_PROCESS:
	  if (rec->ut_name[0] == '\0')
#ifdef BACKWARDS
	    log_out (rec, FALSE);
#else
	    log_out (rec);
#endif
	  else
	    log_in (rec);
	  break;
#endif

#ifdef DEAD_PROCESS
	  /* A record written when a process exits -- ftpd, login, and
	     other things spawned by init.  If the record has a tty,
	     try to log it out.  Otherwise, it's a background
	     process and should be ignored. */
	  
	case DEAD_PROCESS:
	  if (rec->ut_line[0] != '\0')
#ifdef BACKWARDS
	    log_out (rec, FALSE);
#else
	    log_out (rec);
#endif
	  break;
#endif

#endif /* HAVE_UT_TYPE */
	  
	  /* FIXME: here we assume that unknown types are things that
	     are to be treated as logins/logouts in the BSD sense.  If
	     this is incorrect behavior on some systems, #ifdefs
	     should be added above to handle the new values for the
	     UT_TYPE field. */

	default:

	  if (rec->ut_line[0] == '\0')
	    {
	      if (print_file_problems)
		{
		  utmp_print_file_and_line (stddebug);
		  fprintf (stddebug, ": problem: empty record\n");
		}
	    }
	  else
	    {
	      if (rec->ut_name[0] == '\0')
#ifdef BACKWARDS
		log_out (rec, FALSE);
#else
		log_out (rec);
#endif
	      else
		log_in (rec);
	    }

	  break;
	}

      if (debugging_enabled)
	{
	  struct hashtab_order ho;
	  struct hashtab_elem *he;
	  
	  for (he = hashtab_first (login_table, &ho);
	       he != NULL;
	       he = hashtab_next (&ho))
	    {
	      char *ttyname = hashtab_get_key (he);
	      struct login_data *l = hashtab_get_value (he);

#ifdef BACKWARDS
	      fprintf (stddebug, "**\t%-*.*s %s %s",
		       TTY_LEN, TTY_LEN, ttyname,
		       l->fake_entry ? "FAKE" : "real",
		       ctime (&(l->time)));
#else
	      fprintf (stddebug, "**\t%-*.*s %-*.*s %s",
		       TTY_LEN, TTY_LEN, ttyname,
		       NAME_LEN, NAME_LEN, l->ut_name,
		       ctime (&(l->time)));
#endif
	    }
	}
    }
}

