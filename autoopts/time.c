
/*
 *  $Id: time.c,v 4.2 2008/11/02 18:51:00 bkorb Exp $
 *  Time-stamp:      "2008-11-01 19:52:20 bkorb"
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *
 *  AutoOpts is available under any one of two licenses.  The license
 *  in use must be one of these two and the choice is under the control
 *  of the user of the license.
 *
 *   The GNU Lesser General Public License, version 3 or later
 *      See the files "COPYING.lgplv3" and "COPYING.gplv3"
 *
 *   The Modified Berkeley Software Distribution License
 *      See the file "COPYING.mbsd"
 *
 *  These files have the following md5sums:
 *
 *  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
 *  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
 *  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
 */

#ifndef HAVE_PARSE_TIME

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#ifndef _
#define _(_s)  _s
#endif

#ifndef NUL
#define NUL '\0'
#endif

typedef enum {
  NOTHING_IS_DONE,
  DAY_IS_DONE,
  HOUR_IS_DONE,
  MINUTE_IS_DONE,
  SECOND_IS_DONE
} whats_done_t;

#define SEC_PER_MIN 60
#define SEC_PER_HR  (SEC_PER_MIN * 60)
#define SEC_PER_DAY (SEC_PER_HR  * 24)
#define TIME_MAX    0x7FFFFFFF

extern time_t parse_time(char const * in_pz);

static time_t
parse_hr_min_sec(time_t start, char * pz)
{
  int lpct = 0;

  /* For as long as our scanner pointer points to a colon *AND*
     we've not looped before, then keep looping.  (two iterations max) */
  while ((*pz == ':') && (lpct++ == 0))
    {
      if (  (start > TIME_MAX / 60)
         || ! isdigit((int)*++pz))
        {
          errno = EINVAL;
          return ~0;
        }

      start *= 60;
      errno = 0;

      {
        unsigned long v = strtoul(pz, &pz, 10);
        if (errno != 0)
          return ~0;

        if (start > TIME_MAX - v)
          {
            errno = EINVAL;
            return ~0;
          }
        start += v;
      }
    }

  /* allow for trailing spaces */
  while (isspace(*pz))   pz++;
  if (*pz != NUL)
    {
      errno = EINVAL;
      return ~0;
    }

  return start;
}

time_t
parse_time(char const * in_pz)
{
  whats_done_t whatd_we_do = NOTHING_IS_DONE;

  char * pz;
  time_t val;
  time_t res = 0;

  while (isspace(*in_pz))      in_pz++;
  if (! isdigit((int)*in_pz))  goto bad_time;
  pz = (char *)in_pz;

  do  {

    errno = 0;
    val = strtol(pz, &pz, 10);
    if (errno != 0)
      goto bad_time;

    /*  IF we find a colon, then we're going to have a seconds value.
        We will not loop here any more.  We cannot already have parsed
        a minute value and if we've parsed an hour value, then the result
        value has to be less than an hour. */
    if (*pz == ':')
      {
        if (whatd_we_do >= MINUTE_IS_DONE)
          break;

        val = parse_hr_min_sec(val, pz);

        if ((errno != 0) || (res > TIME_MAX - val))
          break;

        if ((whatd_we_do == HOUR_IS_DONE) && (val >= SEC_PER_HR))
          break;

        /* Check for overflow */
        if (res > TIME_MAX - val)
          break;

        return res + val;
      }

    {
      unsigned int mult;

      while (isspace(*pz))   pz++;

      switch (*pz)
        {
        default:  goto bad_time;
        case NUL:
          /* Check for overflow */
          if (res > TIME_MAX - val)
            goto bad_time;

          return val + res;

        case 'd':
          if (whatd_we_do >= DAY_IS_DONE)
            goto bad_time;
          mult = SEC_PER_DAY;
          whatd_we_do = DAY_IS_DONE;
          break;

        case 'h':
          if (whatd_we_do >= HOUR_IS_DONE)
            goto bad_time;
          mult = SEC_PER_HR;
          whatd_we_do = HOUR_IS_DONE;
          break;

        case 'm':
          if (whatd_we_do >= MINUTE_IS_DONE)
            goto bad_time;
          mult = SEC_PER_MIN;
          whatd_we_do = MINUTE_IS_DONE;
          break;

        case 's':
          mult = 1;
          whatd_we_do = SECOND_IS_DONE;
          break;
        }

      /*  Check for overflow:  value that overflows or an overflowing
          result when "val" gets added to it.  */
      if (val > TIME_MAX / mult)
        break;

      val *= mult;
      if (res > TIME_MAX - val)
        break;

      res += val;

      while (isspace(*++pz))   ;
      if (*pz == NUL)
        return res;

      if (! isdigit(*pz))
        break;
    }

  } while (whatd_we_do < SECOND_IS_DONE);

 bad_time:

  fprintf(stderr, _("Invalid time duration:  %s\n"), in_pz);
  errno = EINVAL;
  return (time_t)~0;
}

#endif

/*=export_func  optionTimeVal
 * private:
 *
 * what:  process an option with a time value.
 * arg:   + tOptions* + pOpts    + program options descriptor +
 * arg:   + tOptDesc* + pOptDesc + the descriptor for this arg +
 *
 * doc:
 *  Decipher a time duration value.
=*/
void
optionTimeVal(tOptions* pOpts, tOptDesc* pOD )
{
    long  val;

    if ((pOD->fOptState & OPTST_RESET) != 0)
        return;

    val = parse_time(pOD->optArg.argString);
    if (errno != 0)
        goto bad_time;

    if (pOD->fOptState & OPTST_ALLOC_ARG) {
        AGFREE(pOD->optArg.argString);
        pOD->fOptState &= ~OPTST_ALLOC_ARG;
    }

    pOD->optArg.argInt = val;
    return;

bad_time:
    fprintf( stderr, zNotNumber, pOpts->pzProgName, pOD->optArg.argString );
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0)
        (*(pOpts->pUsageProc))(pOpts, EXIT_FAILURE);

    pOD->optArg.argInt = ~0;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/numeric.c */
