[+ AutoGen5 Template  -*-  Mode: C -*-

   c=%s-temp.c

+]
[+
  (define prog-name (string->c-name! (get "prog-name")))
  (out-move (string-append "getopt-" prog-name ".c"))
  (dne " *  " "/* " ) +]
 *
[+ (gpl (get "prog-name") " *  ") +]
 */
#include <config.h>
#include <sys/types.h>
#include "[+ (base-name) +].h"
#include <getopt.h>
#include "system.h"

static struct option a_long_opts[] = {[+
    FOR flag     +]
  { [+ (c-string (get "name")) +], [+
    (if (exist? "arg-type") "1" "0") +], NULL, [+

      IF (exist? "value") +]'[+ value +]'[+
      ELSE       +][+
        (+ (for-index) 128) +][+
      ENDIF      +] },[+

    ENDFOR flag  +]
  { "help", 0, NULL, 'h' },
  { "version", 0, NULL, [+ (+ (count "flag") 128) +] },
  { NULL, 0, NULL, 0 }
};

static char z_opts[] =
    "[+
    FOR flag     +][+
      CASE value +][+
      ~ [ :-]    +][+
      ~ [!-~]    +][+ value +][+ (if (exist? "arg-type") ":" "") +][+
      ESAC       +][+
    ENDFOR
    +]";

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
  "td=.opt-$$
   rm -rf .opt-*
   mkdir ${td}
   sdir=`cd ${srcdir:-.} ; pwd`
   cd ${td}
   CFLAGS=\"${CFLAGS}  `autoopts-config cflags` -DTEST_%2$s_OPTS\"
   LDFLAGS=\"${LDFLAGS} `autoopts-config ldflags`\"
   ${CC} ${CFLAGS} -o %1$s ${sdir}/%3$s.c ${LDFLAGS} || \
      kill -9 ${AG_pid}
   (./%1$s -: 2>&1) | \
      sed '1d;/more-help/d;s/--version\\[=arg\\]/--version      /'
   cd ..
   rm -rf ${td}"
    (get "prog-name")
    (string-upcase (. prog-name))
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
  optionUsage (pOptions, EXIT_SUCCESS);
}

void
doVersion(
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
{
  char* pz_by = _("[+(. prog-name)
    +] (core-utils) [+ version +]\n\
Written by [+(join ", " (stack "copyright.author"))+]\n\n\
Copyright (C) [+ copyright.date +] by [+ copyright.owner +]\n[+

CASE copyright.type +][+
*= gpl    +]\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.[+

ESAC +]\n");

  fputs (pz_by, stdout);
  exit (EXIT_SUCCESS);
}

[+
 IF (> (count "flag") (count "flag.min")) +]
static void
usage_too_many (const char* pz_optname)
{
  char* pz = _("[+(. prog-name)
    +] error: the '%s' option appears more than once\n");
  printf (pz, pz_optname);
  USAGE( EXIT_FAILURE );
}
[+
 ENDIF (> (count "flag") (count "flag.max"))
+][+
 IF (exist? "flag.flags-cant") +]
static void
usage_cannot (const char* pz_what, const char* pz_cant)
{
  char* pz = _("[+(. prog-name)
    +] error: the %s option cannot appear with the %s option\n");
  printf (pz, pz_what, pz_cant);
  USAGE (EXIT_FAILURE);
}
[+
 ENDIF
+][+
 IF (exist? "flag.flags-must") +]
static void
usage_must (const char* pz_what, const char* pz_must)
{
  char* pz = _("[+(. prog-name)
    +] error: the %s option must appear with the %s option\n");
  printf (pz, pz_what, pz_must);
  USAGE (EXIT_FAILURE);
}
[+
 ENDIF
+]
int
process_[+(. prog-name)
        +]_opts (int argc, char** argv)
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
    int lidx;
    int optc = getopt_long (argc, argv, z_opts, a_long_opts, &lidx);

    switch (optc) {
    case  -1: goto leave_processing;
    case   0:  break;[+
    FOR flag  +][+
      (define OPT-NAME (string-upcase! (string->c-name! (get "name"))))
+]
    case [+

      IF (exist? "value") +]'[+
        (if (== (get "value") "'") "\\'" (get "value")) +]'[+
      ELSE       +][+
        (+ (for-index) 128) +][+
      ENDIF      +]:[+

      IF (not (exist? "max")) +]
      if (HAVE_OPT( [+ (. OPT-NAME) +] ))
        usage_too_many ([+ (c-string (get "name")) +]);[+
      ENDIF
+]
      SET_OPT_[+ (. OPT-NAME) +][+
             (if (exist? "arg-type") "(optarg)") +];
      break;[+

    ENDFOR    +]
    case 'h': USAGE( EXIT_SUCCESS ); break;

    case [+ (+ (count "flag") 128) +]:
      version_etc (stdout, [+ (c-string (get "prog-name"))+], 
                  [+ (c-string (get "prog-name"))+], 
                  [+ (c-string (get "version"))
                   +], [+ (c-string (if (exist? "copyright.author")
                                     (join ", " (stack "copyright.author"))
                                     (get "copyright.owner")))
                  +]);
      exit (EXIT_SUCCESS);
      break;

    default:
      optionUsage (&[+ (. prog-name) +]Options, EXIT_FAILURE);
    }
  } leave_processing:;[+

FOR flag +][+
  IF
     (set! OPT-NAME (string-upcase! (string->c-name! (get "name"))))
     (or (exist? "flags-cant") (exist? "flags-must")) +]
  if (HAVE_OPT( [+ (string-upcase OPT-NAME) +] )) {
    static const char z_what[] = [+ (c-string (get "name")) +];[+
    FOR flags-cant +]
    if (HAVE_OPT( [+ (string-upcase! (get "flags-cant")) +] ))
      usage_cannot (z_what, [+ (c-string (get "flags-cant")) +]);[+
    ENDFOR cant    +][+
    FOR flags-must +]
    if (! HAVE_OPT( [+ (string-upcase! (get "flags-must")) +] ))
      usage_must (z_what, [+ (c-string (get "flags-must")) +]);[+
    ENDFOR must    +]
  }
[+
  ENDIF  +][+
ENDFOR   +]
  return 0;
}
