[= autogen5 template
# $Id: opthead.tpl,v 2.20 2000/10/10 12:33:40 bkorb Exp $
=]
[=

;;  This is the first time through.  Save the output file name
;;  so the 'C' file can '#include' it easily."

(define hdrname (out-name))

;; The #define name we use to self-exclude the header"

(define defname (string-tr! (string-append prog-name "_" hdrname)
                     "-./=a-z" "____A-Z" ))

;; Let the real output begin...

=]
/*
 *  This file contains the programmatic interface to the Automated
 *  Options generated for the [=(. prog-name)=] program.
 *  These macros are documented in the AutoGen info file in the
 *  "AutoOpts" chapter.  Please refer to that doc for usage help.
 */

#ifndef [=(. defname)=]
#define [=(. defname)=]
[= Option_Copyright =]
#include <options.h>

/*
 *  Enumeration of each option:
 */
typedef enum {[=
FOR flag =][=
  IF (not (exist? "documentation")) =]
        INDEX_[=(. UP-prefix)=]OPT_[=
                (sprintf "%-16s" (string-upcase! (get "name"))) =] =[=
                (sprintf "%3d" (for-index))=],[=
  ENDIF =][=
ENDFOR flag=][=

(define option-ct (count "flag")) =][=

IF (exist? "version") =]
        INDEX_[=(. UP-prefix)=]OPT_VERSION          = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],[=
ENDIF =]
        INDEX_[=(. UP-prefix)=]OPT_HELP             = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        INDEX_[=(. UP-prefix)=]OPT_MORE_HELP        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=

IF (exist? "homerc") =],
        INDEX_[=(. UP-prefix)=]OPT_SAVE_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        INDEX_[=(. UP-prefix)=]OPT_LOAD_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=
ENDIF =]
} te[=(. Cap-prefix)=]OptIndex;

#define [=(. UP-prefix)=]OPTION_CT    [= (. option-ct) =][=
IF (exist? "version") =]
#define [=(. UP-prog)=]_VERSION       [=(c-string (get "version"))=]
#define [=(. UP-prog)=]_FULL_VERSION  "[=
  (. prog-name) =] - [= % prog_title "%s - " =]Ver. [=version=]"[=
ENDIF (exist? version) =]

/*
 *  Interface defines for all options.  Replace "n" with
 *  the UPPER_CASED option name (as in the te[=(. Cap-prefix)=]OptIndex
 *  enumeration above).  e.g. HAVE_[=(. UP-prefix)=]OPT( [=
    (string-upcase! (get "flag[].name" ))=] )
 */
#ifdef __STDC__
#define     [=(. UP-prefix)=]DESC(n)     [=prog_name
                 =]Options.pOptDesc[INDEX_[=
                 (. UP-prefix)=]OPT_ ## n]
#else
#define     [=(. UP-prefix)=]DESC(n)     [=prog_name
                 =]Options.pOptDesc[INDEX_[=
                 (. UP-prefix)=]OPT_/**/n]
#endif
#define     HAVE_[=(. UP-prefix)=]OPT(n) (! UNUSED_OPT(&[=(. UP-prefix)
                 =]DESC(n)))
#define      [=(. UP-prefix)=]OPT_ARG(n) ([=(. UP-prefix)
                 =]DESC(n).pzLastArg)
#define    STATE_[=(. UP-prefix)=]OPT(n) ([=(. UP-prefix)
                 =]DESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_[=(. UP-prefix)=]OPT(n) ([=(. UP-prefix)
                 =]DESC(n).optOccCt)
#define    ISSEL_[=(. UP-prefix)=]OPT(n) (SELECTED_OPT(&[=(. UP-prefix)
                 =]DESC(n)))
#define ISUNUSED_[=(. UP-prefix)=]OPT(n) (UNUSED_OPT(& [=(. UP-prefix)
                 =]DESC(n)))
#define  ENABLED_[=(. UP-prefix)=]OPT(n) (! DISABLED_OPT(& [=(. UP-prefix)
                 =]DESC(n)))
#define  STACKCT_[=(. UP-prefix)=]OPT(n) (((tArgList*)([=(. UP-prefix)
                         =]DESC(n).optCookie))->useCt)
#define STACKLST_[=(. UP-prefix)=]OPT(n) (((tArgList*)([=(. UP-prefix)
                         =]DESC(n).optCookie))->apzArgs)
#define    CLEAR_[=(. UP-prefix)=]OPT(n) STMTS( \
                [=(. UP-prefix)=]DESC(n).fOptState &= OPTST_PERSISTENT;   \
                if ( ([=(. UP-prefix)
                    =]DESC(n).fOptState & OPTST_INITENABLED) == 0) \
                    [=(. UP-prefix)=]DESC(n).fOptState |= OPTST_DISABLED; \
                [=(. UP-prefix)=]DESC(n).optCookie = (void*)NULL )

/*
 *  Interface defines for specific options.
 */[=
  (define UP-name "")
  (define descriptor "")
  (define opt-name "")   =][=

FOR flag =][=
  (set! UP-name    (string-upcase! (get "name")))
  (set! opt-name   (string-append UP-prefix "OPT_" UP-name))
  (set! descriptor (string-append UP-prefix "DESC(" UP-name ")" )) =][=

 IF (exist? "documentation") =][=
   IF (or (exist? "call_proc") (exist? "flag_code"))
=]
#define SET_[=(. UP-prefix)=]OPT_[=(. UP-name)=]   STMTS( \
        (*([=(. descriptor)=].pOptProc))( &[=(. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=(for-index)=] )[=

   ENDIF =][=
 ELSE "not a documentation option" =][=
   Option_Defines =][=
 ENDIF documentation =][=
ENDFOR flag=][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

Autoopts maintained option values.

If *any* option flag value is specified,
then we provide flag characters for our options.
Otherwise, we will use the INDEX_* values for the option value.

There are no documentation strings because these defines
are used identically to the user-generated VALUE defines.

:=]
[=
IF (exist? "flag.value") =][=
  IF (exist? "version") =]
#define VALUE_[=(. UP-prefix)=]OPT_VERSION        'v'[=
  ENDIF =][=

  IF (exist? "homerc")=]
#define VALUE_[=(. UP-prefix)=]OPT_SAVE_OPTS      '>'
#define VALUE_[=(. UP-prefix)=]OPT_LOAD_OPTS      '<'[=
  ENDIF
=]
#define VALUE_[=(. UP-prefix)=]OPT_HELP           '?'
#define VALUE_[=(. UP-prefix)=]OPT_MORE_HELP      '!'[=


ELSE "flag.value *DOES NOT* exist" =][=

  IF (exist? "version") =]
#define VALUE_[=(. UP-prefix)=]OPT_VERSION        INDEX_[=
                                      (. UP-prefix)=]OPT_VERSION[=
  ENDIF=][=
  IF (exist? "homerc") =]
#define VALUE_[=(. UP-prefix)=]OPT_SAVE_OPTS      INDEX_[=
                                      (. UP-prefix)=]OPT_SAVE_OPTS
#define VALUE_[=(. UP-prefix)=]OPT_LOAD_OPTS      INDEX_[=
                                      (. UP-prefix)=]OPT_LOAD_OPTS[=
  ENDIF=]
#define VALUE_[=(. UP-prefix)=]OPT_HELP           INDEX_[=
                                      (. UP-prefix)=]OPT_HELP
#define VALUE_[=(. UP-prefix)=]OPT_MORE_HELP      INDEX_[=
                                      (. UP-prefix)=]OPT_MORE_HELP[=
ENDIF=]

/*
 *  Interface defines not associated with particular options
 */
#define  ERRSKIP_[=(. UP-prefix)=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet &= ~OPTPROC_ERRSTOP )
#define  ERRSTOP_[=(. UP-prefix)=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet |= OPTPROC_ERRSTOP )
#define  RESTART_[=(. UP-prefix)=]OPT(n) STMTS( \
                [=(. pname)=]Options.curOptIdx = (n); \
                [=(. pname)=]Options.pzCurOpt  = (char*)NULL )
#define    START_[=(. UP-prefix)=]OPT    RESTART_[=(. UP-prefix)
                =]OPT(1)
#define     [=(. UP-prefix)=]USAGE(c)    (*[=(. pname)
                 =]Options.pUsageProc)( &[=(. pname)=]Options, c )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *
=]

/* * * * * *
 *
 *  Declare the [=(. prog-name)=] option descriptor.
 */
#ifdef  __cplusplus
extern "C" {
#endif

extern tOptions   [=(. pname)=]Options;[=

IF (exist? "export")=]

/* * * * * *
 *
 *  Globals exported from the [=prog_title=] option definitions
 */[=
  FOR export "\n"=]
[= (get "export") =][=
  ENDFOR export=][=
ENDIF=]

#ifdef  __cplusplus
}
#endif
#endif /* [=(. defname)=] */
