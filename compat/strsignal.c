
/*
 *  strsignal is free software.
 *  
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *  
 *  strsignal is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with strsignal.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *  
 *  As a special exception, Bruce Korb gives permission for additional
 *  uses of the text contained in the release of strsignal.
 *  
 *  The exception is that, if you link the strsignal library with other
 *  files to produce an executable, this does not by itself cause the
 *  resulting executable to be covered by the GNU General Public License.
 *  Your use of that executable is in no way restricted on account of
 *  linking the strsignal library code into it.
 *  
 *  This exception does not however invalidate any other reasons why
 *  the executable file might be covered by the GNU General Public License.
 *  
 *  This exception applies only to the code released by Bruce Korb under
 *  the name strsignal.  If you copy code from other sources under the
 *  General Public License into a copy of strsignal, as the General Public
 *  License permits, the exception does not apply to the code that you add
 *  in this way.  To avoid misleading anyone as to the status of such
 *  modified files, you must delete this exception notice from them.
 *  
 *  If you write modifications of your own for strsignal, it is your choice
 *  whether to permit this exception to apply to your modifications.
 *  If you do not wish that, delete this exception notice.
 *
 *  $Id: strsignal.c,v 3.2 2002/06/15 18:24:59 bkorb Exp $
 */

#include "compat.h"

/*  Routines imported from standard C runtime libraries. */

#if ! defined( HAVE_STRSIGNAL ) || ! defined( HAVE_PSIGNAL )

#ifdef __STDC__
# include <stddef.h>
#else	/* !__STDC__ */
#  ifndef const
#    define const
#  endif
#endif	/* __STDC__ */

#ifdef HAVE_SYS_SIGLIST
#  include <signal.h>
#endif

/*
 *  Import the generated tables
 */
#include "strsignal.h"
#endif

#ifndef HAVE_STRSIGNAL
/*

NAME

	signo_max -- return the max signo value

SYNOPSIS

	int signo_max ();

DESCRIPTION

	Returns the maximum signo value for which a corresponding symbolic
	name or message is available.  Note that in the case where
	we use the sys_siglist supplied by the system, it is possible for
	there to be more symbolic names than messages, or vice versa.
	In fact, the manual page for psignal(3b) explicitly warns that one
	should check the size of the table (NSIG) before indexing it,
	since new signal codes may be added to the system before they are
	added to the table.  Thus NSIG might be smaller than value
	implied by the largest signo value defined in <signal.h>.

	We return the maximum value that can be used to obtain a meaningful
	symbolic name or message.

*/

int
signo_max ()
{
  return MAX_SIGNAL_NUMBER;
}


/*

NAME

	strsignal -- map a signal number to a signal message string

SYNOPSIS

	char *strsignal (int signo)

DESCRIPTION

	Maps an signal number to an signal message string, the contents of
	which are implementation defined.  On systems which have the external
	variable sys_siglist, these strings will be the same as the ones used
	by psignal().

	If the supplied signal number is within the valid range of indices
	for the sys_siglist, but no message is available for the particular
	signal number, then returns the string "Signal NUM", where NUM is the
	signal number.

	If the supplied signal number is not a valid index into sys_siglist,
	returns NULL.

	The returned string is only guaranteed to be valid only until the
	next call to strsignal.

*/

char *
strsignal (signo)
  int signo;
{
  if ((unsigned)signo > MAX_SIGNAL_NUMBER)
    {
      /* Out of range, just return NULL */
      return (char*)NULL;
    }

  return (char*)sys_siglist[ signo ];
}


/*

NAME

	strsigno -- map an signal number to a symbolic name string

SYNOPSIS

	char *strsigno (int signo)

DESCRIPTION

	Given an signal number, returns a pointer to a string containing
	the symbolic name of that signal number, as found in <signal.h>.

	If the supplied signal number is within the valid range of indices
	for symbolic names, but no name is available for the particular
	signal number, then returns the string "Signal NUM", where NUM is
	the signal number.

	If the supplied signal number is not within the range of valid
	indices, then returns NULL.

*/

char *
strsigno (signo)
  int signo;
{
  if ((unsigned)signo > MAX_SIGNAL_NUMBER)
    {
      /* Out of range, just return NULL */
      return (char*)NULL;
    }

  return (char*)signal_names[ signo ];
}


/*

NAME

	strtosigno -- map a symbolic signal name to a numeric value

SYNOPSIS

	int strtosigno (char *name)

DESCRIPTION

	Given the symbolic name of a signal, map it to a signal number.
	If no translation is found, returns 0.

*/

int
strtosigno (name)
  char *name;
{
  int signo;

  if (name != NULL)
    {
      for (signo = MAX_SIGNAL_NUMBER; signo > 1; signo--)
	{
	  if (strcmp (name, signal_names[signo]) == 0)
	    {
	      break;
	    }
	}
    }
  return (signo);
}

#endif  /* HAVE_STRSIGNAL */

/*

NAME

	psignal -- print message about signal to stderr

SYNOPSIS

	void psignal (unsigned signo, char *message);

DESCRIPTION

	Print to the standard error the message, followed by a colon,
	followed by the description of the signal specified by signo,
	followed by a newline.
*/

#ifndef HAVE_PSIGNAL

#ifndef __P
#  define __P(a) a
#endif

void
psignal (signo, message)
  unsigned signo;
  char *message;
{
  if ((unsigned)signo > MAX_SIGNAL_NUMBER)
    {
      fprintf (stderr, "%s: unknown signal %d\n", message, signo);
    }
  else
    {
      fprintf (stderr, "%s: %s\n", message, sys_siglist[signo]);
    }
}
#endif	/* HAVE_PSIGNAL */
