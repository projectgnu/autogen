[+ AutoGen5 Template  -*- Mode: C -*-

   c=%s-temp.c

# Time-stamp:      "2005-02-14 08:23:31 bkorb"

+][+
`stamp=\`sed 's,.*stamp:,,' <<'_EOF_'
  Time-stamp:        "2005-02-07 10:18:18 bkorb"
_EOF_
\` `
+][+
  (if (not (exist? "settable"))
      (error "'settable' must be specified globally for getopt_long\n"))
  (define prog-name (string->c-name! (get "prog-name")))
  (define PROG-NAME (string-upcase prog-name))
  (out-move (string-append "getopt-" prog-name ".c"))
  (dne " *  " "/* " ) +]
 *
[+ CASE copyright.type +][+
   = gpl  +][+ (gpl  prog-name " *  ") +][+
   = lgpl +][+ (lgpl prog-name (if (exist? "copyright.owner")
                                   (get "copyright.owner")
                                   (get "copyright.author")) " *  ") +][+
   = note +][+ (prefix " *  " (get "copyright.text")) +][+
   ESAC   +]
 *
 *  Last template edit: [+ `echo $stamp` +]
 *  $Id: getopt.tpl,v 4.3 2005/02/14 16:25:37 bkorb Exp $
 */
#include <sys/types.h>
#include <stdlib.h>
#include "[+ (base-name) +].h"[+

IF (exist? "long-opts") +]
#include <getopt.h>

/*
 *  getopt_long option descriptor
 */
static struct option a_long_opts[] = {[+

  FOR flag            +][+
    (sprintf

       "\n  { %-20s %d, NULL, VALUE_OPT_%s },"
          (string-append (c-string (get "name")) ",")
          (if (exist? "arg-type") 1 0)
          (string-upcase (string->c-name! (get "name")))
    ) +][+

  ENDFOR flag

+]
  { "help",              0, NULL, VALUE_OPT_HELP },[+
IF (exist? "version") +]
  { "version",           0, NULL, VALUE_OPT_VERSION },[+
ENDIF +]
  { NULL,                0, NULL, 0 }
};
[+ ENDIF +]
/*
 *  Option flag character list
 */
static char z_opts[] = "[+ # close quote for emacs " +][+
    FOR flag            +][+
      CASE value        +][+
      ~ [!-~]           +][+ value +][+

        CASE arg-type   +][+
        =* str          +]:[+
        == ""           +][+
        *               +][+ (error (sprintf
        "error in %s opt: The only allowed arg type is 'string'\n"
        (get "name") )) +][+
        ESAC            +][+

      ESAC              +][+

    ENDFOR              +][+
    IF  (not (exist? "help-value")) +]?[+
    ELSE                +][+
      CASE help-value   +][+
      == ""             +][+
      == '"'            +]\"[+
      *                 +][+ help-value  +][+
      ESAC              +][+
    ENDIF               +][+
    IF  (exist? "version")  +][+
      IF  (not (exist? "version-value")) +]v[+
      ELSE              +][+
        CASE version-value +][+
        == ""           +][+
        == '"'          +]\"[+
        *               +][+ version-value +][+
        ESAC            +][+
      ENDIF             +][+
    ENDIF               +][+
   # open quote for emacs " +]";

/*
 *  AutoOpts library replacement routines:
 */
void
optionUsage (tOptions* pOptions, int status)
{
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
             [+ (. prog-name) +]Options.pzProgName);
  else
    {
      fputs (_(
[+ (kr-string (string-append (shellf
  "[ \"${VERBOSE:-false}\" = true ] && set -x ; td=.opt-$$
   rm -rf .opt-*
   mkdir ${td}
   sdir=`cd ${srcdir:-.} ; pwd`
   cd ${td}
   CFLAGS=\"${CFLAGS}  `autoopts-config cflags` -DTEST_%2$s_OPTS\"
   LDFLAGS=\"${LDFLAGS} `autoopts-config ldflags`\"
   ${CC:-cc} ${CFLAGS} -o %1$s ${sdir}/%3$s.c ${LDFLAGS} || \
      kill -9 ${AG_pid}
   (./%1$s -: 2>&1) | \
      sed '1d;/more-help/d
           s/--version\\[=arg\\]/--version      /
           /version information and exit/s/-v \\[arg\\]/-v      /'
   cd ..
   [ \"${VERBOSE:-false}\" = true ] || rm -rf ${td}"
    (get "prog-name")
    (. PROG-NAME)
    (base-name)
    ) "\n" )) +]), stdout);
    }

  exit (status);
}

void
doPagedUsage(
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
{
  fputs (_("[+(. prog-name)
    +] error: paged usage help has been disabled\n"), stderr);
  optionUsage (pOptions, EXIT_FAILURE);
}

void
doVersion(
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
{
  char* pz_by = _("[+ # " +][+

  (sprintf "%s%s %s" prog-name
     (if (exist? "prog-group")
         (sprintf " (%s)" (get "prog-group"))
         "" )
     (get "version") ) +]\n\
Written by [+(join ", " (stack "copyright.author"))+].\n\n\
Copyright (C) [+ copyright.date +] by [+ copyright.owner +]\n[+

CASE copyright.type +][+
*= gpl    +]\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.[+

ESAC +][+ # " +]\n");

  fputs (pz_by, stdout);
  exit (EXIT_SUCCESS);
}

/*
 *  If an option appears more often than is allowed, ...
 */
static void
usage_too_many (tOptDesc* pOptDesc)
{
  char* pz = _("[+(. prog-name)
    +] error: the '%s' option appears more than %d times\n");
  printf (pz, pOptDesc->pz_Name, pOptDesc->optMaxCt);
  USAGE( EXIT_FAILURE );
}
[+
 IF (exist? "flag.min")
+]
/*
 *  There is at least one option that must appear.
 */
static void
usage_too_few (tOptDesc* pOptDesc)
{
  char* pz = _("[+(. prog-name)
    +] error: the '%s' option must appear %d times\n");
  printf (pz, pOptDesc->pz_Name, pOptDesc->optMinCt);
  USAGE( EXIT_FAILURE );
}
[+
 ENDIF
+][+
 IF (exist? "flag.flags-cant")
+]
/*
 *  There is at least one pair of options that may not appear together
 *  on the command line.
 */
static void
usage_cannot (const char* pz_what, const char* pz_cant)
{
  char* pz = _("[+(. prog-name)
    +] error: the `%s' option conflicts with `%s'\n");
  printf (pz, pz_what, pz_cant);
  USAGE (EXIT_FAILURE);
}
[+
 ENDIF
+][+
 IF (exist? "flag.flags-must")
+]
/*
 *  There is at least one pair of options that are required to appear
 *  together on the command line.
 */
static void
usage_must (const char* pz_what, const char* pz_must)
{
  char* pz = _("[+(. prog-name)
    +] error: the `%s' option requires `%s'\n");
  printf (pz, pz_what, pz_must);
  USAGE (EXIT_FAILURE);
}
[+
 ENDIF
+]
/*
 *  Process the options for the "[+(. prog-name)+]" program.
 *  This function was generated to use the getopt_long(3GNU) function.
 *  There are [+ (+ (count "flag") (if (exist? "version") 2 1))
              +] options for this program,
 * including "help (usage)"[+
    IF (exist? "version") +] and "version"[+ ENDIF +].
 */
int
process_[+(. prog-name)+]_opts (int argc, char** argv)
{
  {
    char* pz_prog = strrchr (argv[0], '/');
    if (pz_prog != NULL)
      pz_prog++;
    else
      pz_prog = argv[0];
    [+ (. prog-name) +]Options.pzProgName = pz_prog;
  }

  for (;;) {
    switch ([+

IF (exist? "long-opts")
      +]getopt_long (argc, argv, z_opts, a_long_opts, NULL)[+
ELSE  +]getopt (argc, argv, z_opts)[+
ENDIF +]) {
    case  -1: goto leave_processing;
    case   0: break;[+
    FOR flag  +][+
      (define OPT-NAME (string-upcase! (string->c-name! (get "name"))))
+]

    case VALUE_OPT_[+ (. OPT-NAME) +]:[+

      IF (not (exist? "max")) +]
      if (HAVE_OPT( [+(. OPT-NAME)+] ))
        usage_too_many (&DESC([+(. OPT-NAME) +]));[+

      ELIF (not (= (get "max") "nolimit"))  +]
      if (DESC([+(. OPT-NAME)+]).optOccCt++ >= DESC([+(. OPT-NAME)+]).optMaxCt)
        usage_too_many (&DESC([+(. OPT-NAME) +]));[+
      ENDIF
+]
      SET_OPT_[+(. OPT-NAME)+][+ (if (exist? "arg-type") "(optarg)") +];
      break;[+

    ENDFOR +]

    case VALUE_OPT_HELP:
      USAGE (EXIT_SUCCESS);
      /* NOTREACHED */
[+ IF (exist? "version") +]
    case VALUE_OPT_VERSION:
      doVersion (&[+ (. prog-name) +]Options, &DESC(VERSION));
      /* NOTREACHED */
[+ ENDIF +]
    default:
      USAGE (EXIT_FAILURE);
    }
  } leave_processing:;
[+
FOR flag +][+
  IF
     (set! OPT-NAME (string-upcase! (string->c-name! (get "name"))))
     (define check-have-opt (or (exist? "flags-cant") (exist? "flags-must")))
     check-have-opt
+]
  if (HAVE_OPT( [+ (. OPT-NAME) +] )) {[+

    FOR flags-cant +]
    if (HAVE_OPT( [+ (string-upcase! (get "flags-cant")) +] ))
      usage_cannot (DESC([+ (. OPT-NAME) +]).pz_Name, DESC([+
                   (string-upcase! (get "flags-cant")) +]).pz_Name);[+
    ENDFOR cant    +][+

    FOR flags-must +]
    if (! HAVE_OPT( [+ (string-upcase! (get "flags-must")) +] ))
      usage_must (DESC([+ (. OPT-NAME) +]).pz_Name, DESC([+
                   (string-upcase! (get "flags-must")) +]).pz_Name);[+
    ENDFOR must    +][+
    IF (exist? "min") +][+
      IF (> (string->number (get "min" "0")) 1) +]
    if (DESC([+(. OPT-NAME)+]).optOccCt < DESC([+(. OPT-NAME)+]).optMinCt)
      usage_too_few (&DESC([+(. OPT-NAME) +]));[+

      ENDIF +][+
    ENDIF +]
  }
[+

  ENDIF

+][+

  IF (exist? "min")  +][+
    IF (. check-have-opt)
+]  else[+

    ELSE
+]
  if ([+ #
       We have a minimum count, but we have not checked for option existence
       yet because there are no option interdependencies.  We must therefore
       now check to see if the option has appeared the required number of
       times.  In the absence of a max count, our limit must be one and we
       only check for presence.  If a max count exists, then we will also
       have kept the occurrence count.  Check that against the limit. +][+

      IF (not (exist? "max"))
        +]! HAVE_OPT( [+ (. OPT-NAME) +] )[+
      ELSE  max ct exists
        +]DESC([+(. OPT-NAME)+]).optOccCt < DESC([+(. OPT-NAME)+]).optMinCt[+
      ENDIF +])[+

    ENDIF +]
    usage_too_few (&DESC([+(. OPT-NAME) +]));
[+
  ENDIF  +][+
ENDFOR   +]
  return 0;
}
