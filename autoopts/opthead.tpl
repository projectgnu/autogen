[= autogen5 template -*- Mode: C -*-

# $Id: opthead.tpl,v 4.10 2005/12/13 19:16:44 bkorb Exp $
# Automated Options copyright 1992-2005 Bruce Korb
# Time-stamp:      "2005-12-13 11:12:57 bkorb"

=]
/*
 *  This file contains the programmatic interface to the Automated
 *  Options generated for the [=prog-name=] program.
 *  These macros are documented in the AutoGen info file in the
 *  "AutoOpts" chapter.  Please refer to that doc for usage help.
 */
[= (make-header-guard "autoopts") =]
[= Option_Copyright =][=
% config-header "\n#include \"%s\""=]
#include <autoopts/options.h>

/*
 *  Ensure that the library used for compiling this generated header is at
 *  least as new as the version current when the header template was released
 *  (not counting patch version increments).  Also ensure that the oldest
 *  tolerable version is at least as old as what was current when the header
 *  template was released.
 */
#define AO_TEMPLATE_VERSION XXX
#if (AO_TEMPLATE_VERSION < OPTIONS_MINIMUM_VERSION) \
 || (AO_TEMPLATE_VERSION > OPTIONS_STRUCT_VERSION)
# error option template version mismatches autoopts/options.h header
#endif

/*
 *  Enumeration of each option:
 */
typedef enum {[=
FOR flag    =][=
  IF (not (exist? "documentation")) =]
        [= (sprintf "%-26s =%3d," (index-name "name") (for-index)) =][=
  ENDIF     =][=
ENDFOR flag =][=

(define option-ct (count "flag")) =][=

IF (exist? "version") =]
        [= (. INDEX-pfx) =]VERSION          = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],[=
ENDIF =]
        [= (. INDEX-pfx) =]HELP             = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        [= (. INDEX-pfx) =]MORE_HELP        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=

IF (exist? "homerc") =],
        [= (. INDEX-pfx) =]SAVE_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        [= (. INDEX-pfx) =]LOAD_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=
ENDIF =]
} te[=(. Cap-prefix)=]OptIndex;

#define [=(. UP-prefix)=]OPTION_CT    [= (. option-ct) =][=
IF (exist? "version") =]
#define [=(. pname-up)=]_VERSION       [=(c-string (get "version"))=]
#define [=(. pname-up)=]_FULL_VERSION  [=(c-string version-text) =][=
ENDIF (exist? version) =]

/*
 *  Interface defines for all options.  Replace "n" with
 *  the UPPER_CASED option name (as in the te[=(. Cap-prefix)=]OptIndex
 *  enumeration above).  e.g. HAVE_[=(. UP-prefix)=]OPT( [=
    (up-c-name "flag[].name") =] )
 */[=

IF (> 1 (string-length UP-prefix))

=]
#define         DESC(n) [=(. pname)=]Options.pOptDesc[INDEX_OPT_ ## n]
#define     HAVE_OPT(n) (! UNUSED_OPT(& DESC(n)))
#define      OPT_ARG(n) (DESC(n).pzLastArg)
#define    STATE_OPT(n) (DESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_OPT(n) (DESC(n).optOccCt)
#define    ISSEL_OPT(n) (SELECTED_OPT(&DESC(n)))
#define ISUNUSED_OPT(n) (UNUSED_OPT(& DESC(n)))
#define  ENABLED_OPT(n) (! DISABLED_OPT(& DESC(n)))
#define  STACKCT_OPT(n) (((tArgList*)(DESC(n).optCookie))->useCt)
#define STACKLST_OPT(n) (((tArgList*)(DESC(n).optCookie))->apzArgs)
#define    CLEAR_OPT(n) STMTS( \
                DESC(n).fOptState &= OPTST_PERSISTENT;   \
                if ( (DESC(n).fOptState & OPTST_INITENABLED) == 0) \
                    DESC(n).fOptState |= OPTST_DISABLED; \
                DESC(n).optCookie = NULL )[=

ELSE we have a prefix:

=][=  (sprintf "
#define         %1$sDESC(n) %2$sOptions.pOptDesc[INDEX_%1$sOPT_ ## n]
#define     HAVE_%1$sOPT(n) (! UNUSED_OPT(& %1$sDESC(n)))
#define      %1$sOPT_ARG(n) (%1$sDESC(n).pzLastArg)
#define    STATE_%1$sOPT(n) (%1$sDESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_%1$sOPT(n) (%1$sDESC(n).optOccCt)
#define    ISSEL_%1$sOPT(n) (SELECTED_OPT(&%1$sDESC(n)))
#define ISUNUSED_%1$sOPT(n) (UNUSED_OPT(& %1$sDESC(n)))
#define  ENABLED_%1$sOPT(n) (! DISABLED_OPT(& %1$sDESC(n)))
#define  STACKCT_%1$sOPT(n) (((tArgList*)(%1$sDESC(n).optCookie))->useCt)
#define STACKLST_%1$sOPT(n) (((tArgList*)(%1$sDESC(n).optCookie))->apzArgs)
#define    CLEAR_%1$sOPT(n) STMTS( \\
                %1$sDESC(n).fOptState &= OPTST_PERSISTENT;   \\
                if ( (%1$sDESC(n).fOptState & OPTST_INITENABLED) == 0) \\
                    %1$sDESC(n).fOptState |= OPTST_DISABLED; \\
                %1$sDESC(n).optCookie = NULL )"

  UP-prefix pname )   =][=

ENDIF prefix/not      =]

/*
 *  Interface defines for specific options.
 */[=

FOR flag              =][=
  save-name-morphs    =][=

  IF (set! opt-name   (string-append OPT-pfx UP-name))
     (set! descriptor (string-append UP-prefix "DESC(" UP-name ")" ))

     (exist? "documentation")

   =][=
   IF (hash-ref have-cb-procs flg-name)
=]
#define SET_[= (string-append OPT-pfx UP-name) =]   STMTS( \
        (*([=(. descriptor)=].pOptProc))( &[=(. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=(for-index)=] )[=

   ENDIF              =][=
 ELSE                 =][=
   Option_Defines     =][=
 ENDIF                =][=
ENDFOR  flag

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

Autoopts maintained option values.

If *any* option flag value is specified,
then we provide flag characters for our options.
Otherwise, we will use the INDEX_* values for the option value.

There are no documentation strings because these defines
are used identically to the user-generated VALUE defines.

:=]
[=

DEFINE set-std-value =]
#define [= (sprintf "%-23s " (string-append VALUE-pfx (get "val-UPNAME"))) =][=
  CASE (set! tmp-val (get "val-name"))
       (get tmp-val)        =][=
   == ""   =][=

     (if (exist? tmp-val)
         (if (not (exist? "long-opts"))
             (error (sprintf "'%s' may not be empty" tmp-val))
             (string-append INDEX-pfx (get "val-UPNAME"))  )
         (sprintf "'%s'" (get "std-value"))
     )     =][=

   == "'"  =]'\''[=
   ~~ .    =]'[=(get tmp-val)=]'[=
   *       =][=(error "value (flag) codes must be single characters") =][=
   ESAC    =][=
ENDDEF set-std-value        =][=

IF (exist? "flag.value")    =][=

  IF (exist? "version")     =][=
    set-std-value
       val-name    = "version-value"
       val-UPNAME  = "VERSION"
       std-value   = "v"    =][=
  ENDIF  have "version"     =][=

  IF (exist? "homerc")      =][=
    set-std-value
       val-name    = "save-opts-value"
       val-UPNAME  = "SAVE_OPTS"
       std-value   = ">"    =][=

    set-std-value
       val-name    = "load-opts-value"
       val-UPNAME  = "LOAD_OPTS"
       std-value   = "<"    =][=
  ENDIF  have "homerc"      =][=

  set-std-value
       val-name    = "help-value"
       val-UPNAME  = "HELP"
       std-value   = "?"    =][=

  set-std-value
       val-name    = "more-help-value"
       val-UPNAME  = "MORE_HELP"
       std-value   = "!"    =][=

ELSE  NO "flag.value"       =][=

  IF (exist? "version") =]
#define [= (. VALUE-pfx) =]VERSION        [= (. INDEX-pfx) =]VERSION[=
  ENDIF=][=
  IF (exist? "homerc") =]
#define [= (. VALUE-pfx) =]SAVE_OPTS      [= (. INDEX-pfx) =]SAVE_OPTS
#define [= (. VALUE-pfx) =]LOAD_OPTS      [= (. INDEX-pfx) =]LOAD_OPTS[=
  ENDIF=]
#define [= (. VALUE-pfx) =]HELP           [= (. INDEX-pfx) =]HELP
#define [= (. VALUE-pfx) =]MORE_HELP      [= (. INDEX-pfx) =]MORE_HELP[=
ENDIF=][=

IF (exist? "homerc") =]
#define SET_[=(. OPT-pfx)=]SAVE_OPTS(a)   STMTS( \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).fOptState &= OPTST_PERSISTENT; \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).fOptState |= OPTST_SET; \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).pzLastArg  = (const char*)(a) )[=
ENDIF
=]

/*
 *  Interface defines not associated with particular options
 */
#define ERRSKIP_[=

IF (> 1 (string-length UP-prefix))

=][= (sprintf  "OPTERR  STMTS( %1$sOptions.fOptSet &= ~OPTPROC_ERRSTOP )
#define ERRSTOP_OPTERR  STMTS( %1$sOptions.fOptSet |= OPTPROC_ERRSTOP )
#define RESTART_OPT(n)  STMTS( \\
                %1$sOptions.curOptIdx = (n); \\
                %1$sOptions.pzCurOpt  = NULL )
#define START_OPT       RESTART_OPT(1)
#define USAGE(c)        (*%1$sOptions.pUsageProc)( &%1$sOptions, c )"
   pname ) =][=

ELSE  we have a prefix

=][= (sprintf  "%1$sOPTERR  STMTS( %2$sOptions.fOptSet &= ~OPTPROC_ERRSTOP )
#define ERRSTOP_%1$sOPTERR  STMTS( %2$sOptions.fOptSet |= OPTPROC_ERRSTOP )
#define RESTART_%1$sOPT(n)  STMTS( \\
                %2$sOptions.curOptIdx = (n); \\
                %2$sOptions.pzCurOpt  = NULL )
#define START_%1$sOPT       RESTART_%1$sOPT(1)
#define %1$sUSAGE(c)        (*%2$sOptions.pUsageProc)( &%2$sOptions, c )"

  UP-prefix  pname ) =][=

ENDIF    have/don't have prefix  '

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

=][=
(tpl-file-line extract-fmt)
=]
/* * * * * *
 *
 *  Declare the [=prog-name=] option descriptor.
 */
#ifdef  __cplusplus
extern "C" {
#endif

extern tOptions   [=(. pname)=]Options;[=

IF (exist? "export")=]

/* * * * * *
 *
 *  Globals exported from the [=prog_title=] option definitions
 */
[= FOR export "\n\n" =][=
      export         =][=
   ENDFOR export     =][=
ENDIF=]

#ifndef _
#  if ENABLE_NLS
#    include <stdio.h>
     static inline char* aoGetsText( const char* pz ) {
         if (pz == NULL) return NULL;
         return (char*)gettext( pz );
     }
#    define _(s)  aoGetsText(s)
#  else  /* ENABLE_NLS */
#    define _(s)  s
#  endif /* ENABLE_NLS */
#endif

#ifdef  __cplusplus
}
#endif
#endif /* [=(. header-guard)=] */
/*
 * Local Variables:
 * Mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * options.h ends here */
