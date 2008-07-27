[= autogen5 template -*- Mode: C -*-

# $Id: opthead.tpl,v 4.32 2008/07/27 20:06:05 bkorb Exp $
# Automated Options copyright 1992-2007 Bruce Korb
# Time-stamp:      "2008-07-05 14:17:55 bkorb"

##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
##  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following md5sums:
##
##  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
##  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd

=]
/*
 *  This file contains the programmatic interface to the Automated
 *  Options generated for the [=prog-name=] program.
 *  These macros are documented in the AutoGen info file in the
 *  "AutoOpts" chapter.  Please refer to that doc for usage help.
 */
[= (make-header-guard "autoopts")       =][=

  `test_exe() {
     test -n "${1}" && test -x ${1} && ${1} -v >/dev/null
     test $? -ne 0 && return 1
     CLexe=${1}
   }
   test_exe ${CLexe} || \
     test_exe ${top_builddir}/columns/columns || \
     test_exe @bindir@/columns || \
     die "cannot locate columns program"
  `                                     =][=
% config-header "\n#include \"%s\""     =]
#include <autoopts/options.h>
[= IF (not (exist? "library"))          =]
/*
 *  Ensure that the library used for compiling this generated header is at
 *  least as new as the version current when the header template was released
 *  (not counting patch version increments).  Also ensure that the oldest
 *  tolerable version is at least as old as what was current when the header
 *  template was released.
 */
#define AO_TEMPLATE_VERSION @AO_TEMPLATE_VERSION@
#if (AO_TEMPLATE_VERSION < OPTIONS_MINIMUM_VERSION) \
 || (AO_TEMPLATE_VERSION > OPTIONS_STRUCT_VERSION)
# error option template version mismatches autoopts/options.h header
  Choke Me.
#endif
[= ENDIF not a library =]
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

IF (exist? "library")           =]
        LIBRARY_OPTION_COUNT[=

ELSE                            =][=

  IF (exist? "resettable")      =]
        [= (. INDEX-pfx) =]RESET            = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],[=
  ENDIF                         =][=

  IF (exist? "version")         =]
        [= (. INDEX-pfx) =]VERSION          = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],[=
  ENDIF                         =]
        [= (. INDEX-pfx) =]HELP             = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        [= (. INDEX-pfx) =]MORE_HELP        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=

  IF (exist? "usage-opt")       =],
        [= (. INDEX-pfx) =]USAGE            = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=
  ENDIF                         =][=

  IF (exist? "homerc")          =],
        [= (. INDEX-pfx) =]SAVE_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=],
        [= (. INDEX-pfx) =]LOAD_OPTS        = [=
                (set! option-ct (+ option-ct 1)) (- option-ct 1)=][=
  ENDIF                         =][=
ENDIF                           =]
} te[=(. Cap-prefix)=]OptIndex;

#define [=(. UP-prefix)=]OPTION_CT    [= (. option-ct) =][=
IF (exist? "version") =]
#define [=(. pname-up)=]_VERSION       [=(c-string (get "version"))=]
#define [=(. pname-up)=]_FULL_VERSION  [=(c-string version-text) =][=
ENDIF (exist? version) =]

/*
 *  Interface defines for all options.  Replace "n" with the UPPER_CASED
 *  option name (as in the te[=(. Cap-prefix)=]OptIndex enumeration above).
 *  e.g. HAVE_[=(. UP-prefix)=]OPT( [= (up-c-name "flag[].name") =] )
 */[=

IF (exist? "library")

=]
extern tOptDesc * const [= (. lib-opt-ptr) =];[=

ENDIF is a library                    =][=

CASE guard-option-names               =][=
!E                                    =][=
      (set! tmp-val (string-append "[" INDEX-pfx "## n]"))
      =][=

=  full-enum                          =][=
      (set! tmp-val "[n]")            =][=

=* no-warn                            =][=
      (set! tmp-val (string-append "[" INDEX-pfx "## n]"))
      =][=

*                                     =][=
      (set! tmp-val (string-append "[" INDEX-pfx "## n]"))
      =][=

ESAC                                  =][=

(if (exist? "library")
    (set! tmp-val (string-append "(" lib-opt-ptr tmp-val ")"))
    (set! tmp-val (string-append "(" pname "Options.pOptDesc" tmp-val ")")) )

(ag-fprintf 0 "\n#define %8sDESC(n) " UP-prefix) tmp-val

=][=

IF (> 1 (string-length UP-prefix))

=]
#define     HAVE_OPT(n) (! UNUSED_OPT(& DESC(n)))
#define      OPT_ARG(n) (DESC(n).optArg.argString)
#define    STATE_OPT(n) (DESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_OPT(n) (DESC(n).optOccCt)
#define    ISSEL_OPT(n) (SELECTED_OPT(&DESC(n)))
#define ISUNUSED_OPT(n) (UNUSED_OPT(& DESC(n)))
#define  ENABLED_OPT(n) (! DISABLED_OPT(& DESC(n)))
#define  STACKCT_OPT(n) (((tArgList*)(DESC(n).optCookie))->useCt)
#define STACKLST_OPT(n) (((tArgList*)(DESC(n).optCookie))->apzArgs)
#define    CLEAR_OPT(n) STMTS( \
                DESC(n).fOptState &= OPTST_PERSISTENT_MASK;   \
                if ( (DESC(n).fOptState & OPTST_INITENABLED) == 0) \
                    DESC(n).fOptState |= OPTST_DISABLED; \
                DESC(n).optCookie = NULL )[=

ELSE we have a prefix:

=][=  (sprintf "
#define     HAVE_%1$sOPT(n) (! UNUSED_OPT(& %1$sDESC(n)))
#define      %1$sOPT_ARG(n) (%1$sDESC(n).optArg.argString)
#define    STATE_%1$sOPT(n) (%1$sDESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_%1$sOPT(n) (%1$sDESC(n).optOccCt)
#define    ISSEL_%1$sOPT(n) (SELECTED_OPT(&%1$sDESC(n)))
#define ISUNUSED_%1$sOPT(n) (UNUSED_OPT(& %1$sDESC(n)))
#define  ENABLED_%1$sOPT(n) (! DISABLED_OPT(& %1$sDESC(n)))
#define  STACKCT_%1$sOPT(n) (((tArgList*)(%1$sDESC(n).optCookie))->useCt)
#define STACKLST_%1$sOPT(n) (((tArgList*)(%1$sDESC(n).optCookie))->apzArgs)
#define    CLEAR_%1$sOPT(n) STMTS( \\
                %1$sDESC(n).fOptState &= OPTST_PERSISTENT_MASK;   \\
                if ( (%1$sDESC(n).fOptState & OPTST_INITENABLED) == 0) \\
                    %1$sDESC(n).fOptState |= OPTST_DISABLED; \\
                %1$sDESC(n).optCookie = NULL )"

  UP-prefix pname ) =][=

ENDIF prefix/not    =][=

  IF (exist? "export")  =]

/* * * * * *
 *
 *  Globals exported from the [=prog_title=] option definitions
 */
[=  (join "\n\n" (stack "export")) =][=
  ENDIF  export? =]
[=

CASE guard-option-names               =][=
!E                                    =][=
=  full-enum                          =][=


=* no-warn                            =]
/*
 *  Make sure there are no #define name conflicts with the option names
 */[=
    FOR  flag                         =]
#undef [= (up-c-name "name")          =][=
    ENDFOR flag                       =][=


*                                     =][=

   (define undef-list "\n#else  /* NO_OPTION_NAME_WARNINGS */")
   (define conf-warn-fmt (string-append
   "\n# ifdef    %1$s"
   "\n#  warning undefining %1$s due to option name conflict"
   "\n#  undef   %1$s"
   "\n# endif" ))

=]
/*
 *  Make sure there are no #define name conflicts with the option names
 */
#ifndef     NO_OPTION_NAME_WARNINGS[=
    FOR  flag       =][=

    (set! opt-name   (up-c-name "name"))
    (set! undef-list (string-append undef-list "\n# undef " opt-name))
    (sprintf conf-warn-fmt opt-name)
    =][=

    ENDFOR flag     =][=

  (. undef-list)=]
#endif  /*  NO_OPTION_NAME_WARNINGS */
[=

ESAC on guard-option-names

=]
/* * * * * *
 *
 *  Interface defines for specific options.
 */[=

FOR flag =][=

  INVOKE save-name-morphs =][=

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

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Autoopts maintained option values.

If *any* option flag value is specified,
then we provide flag characters for our options.
Otherwise, we will use the INDEX_* values for the option value.

There are no documentation strings because these defines
are used identically to the user-generated VALUE defines.

=][=

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
ENDDEF set-std-value            =][=

IF (exist? "flag.value")        =][=

  INVOKE set-std-value
       val-name    = "help-value"
       val-UPNAME  = "HELP"
       std-value   = "?"        =][=

  INVOKE set-std-value
       val-name    = "more-help-value"
       val-UPNAME  = "MORE_HELP"
       std-value   = "!"        =][=

  IF (exist? "resettable")      =][=
    INVOKE set-std-value
       val-name    = "reset-value"
       val-UPNAME  = "RESET"
       std-value   = "R"        =][=
  ENDIF  have "reset"           =][=

  IF (exist? "version")         =][=
    INVOKE set-std-value
       val-name    = "version-value"
       val-UPNAME  = "VERSION"
       std-value   = "v"        =][=
  ENDIF  have "version"         =][=

  IF (exist? "usage-opt")       =][=
    INVOKE set-std-value
       val-name    = "usage-value"
       val-UPNAME  = "USAGE"
       std-value   = "u"        =][=
  ENDIF  have "usage-opt"       =][=

  IF (exist? "homerc")          =][=
    INVOKE set-std-value
       val-name    = "save-opts-value"
       val-UPNAME  = "SAVE_OPTS"
       std-value   = ">"        =][=

    INVOKE set-std-value
       val-name    = "load-opts-value"
       val-UPNAME  = "LOAD_OPTS"
       std-value   = "<"        =][=
  ENDIF  have "homerc"          =][=

ELSE  NO "flag.value"           =][=

  IF (exist? "resettable")      =]
#define [= (. VALUE-pfx) =]RESET          [= (. INDEX-pfx) =]RESET[=
  ENDIF  have "reset"           =][=
  IF (exist? "version")         =]
#define [= (. VALUE-pfx) =]VERSION        [= (. INDEX-pfx) =]VERSION[=
  ENDIF  have "version"         =]
#define [= (. VALUE-pfx) =]HELP           [= (. INDEX-pfx) =]HELP
#define [= (. VALUE-pfx) =]MORE_HELP      [= (. INDEX-pfx) =]MORE_HELP[=
  IF (exist? "usage-opt")       =]
#define [= (. VALUE-pfx) =]USAGE          [= (. INDEX-pfx) =]USAGE[=
  ENDIF  have "usage-opt"       =][=
  IF (exist? "homerc")          =]
#define [= (. VALUE-pfx) =]SAVE_OPTS      [= (. INDEX-pfx) =]SAVE_OPTS
#define [= (. VALUE-pfx) =]LOAD_OPTS      [= (. INDEX-pfx) =]LOAD_OPTS[=
  ENDIF  have "homerc"          =][=
ENDIF    have flag.value/not    =][=

IF (exist? "homerc")            =]
#define SET_[=(. OPT-pfx)=]SAVE_OPTS(a)   STMTS( \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).fOptState &= OPTST_PERSISTENT_MASK; \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).fOptState |= OPTST_SET; \
        [=(. UP-prefix)=]DESC(SAVE_OPTS).optArg.argString = (char const*)(a) )[=
ENDIF                           =][=

IF (not (exist? "library"))

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

  ENDIF    have/don't have prefix  ' =][=

ENDIF is not a library

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

=][=
(tpl-file-line extract-fmt)
=][=

IF (not (exist? "library"))

=]
/* * * * * *
 *
 *  Declare the [=prog-name=] option descriptor.
 */
#ifdef  __cplusplus
extern "C" {
#endif

extern tOptions   [=(. pname)=]Options;[=

 (if (> (string-length added-hdr) 0)
     (begin
        (emit "\n")
        (shellf "sort -u <<_EOF_\n%s_EOF_" added-hdr)
 )   )
=]

#if defined(ENABLE_NLS)
# ifndef _
#    include <stdio.h>
     static inline char* aoGetsText( char const* pz ) {
         if (pz == NULL) return NULL;
         return (char*)gettext( pz );
     }
#    define _(s)  aoGetsText(s)
# else  /* _() */
#    define _(s)  s
# endif /* _() */

# define OPT_NO_XLAT_CFG_NAMES  STMTS([=(. pname)=]Options.fOptSet |= \
                                    OPTPROC_NXLAT_OPT_CFG;)
# define OPT_NO_XLAT_OPT_NAMES  STMTS([=(. pname)=]Options.fOptSet |= \
                                    OPTPROC_NXLAT_OPT|OPTPROC_NXLAT_OPT_CFG;)

# define OPT_XLAT_CFG_NAMES     STMTS([=(. pname)=]Options.fOptSet &= \
                                  ~(OPTPROC_NXLAT_OPT|OPTPROC_NXLAT_OPT_CFG);)
# define OPT_XLAT_OPT_NAMES     STMTS([=(. pname)=]Options.fOptSet &= \
                                  ~OPTPROC_NXLAT_OPT;)

#else   /* ENABLE_NLS */
# define OPT_NO_XLAT_CFG_NAMES
# define OPT_NO_XLAT_OPT_NAMES

# define OPT_XLAT_CFG_NAMES
# define OPT_XLAT_OPT_NAMES

# ifndef _
#   define _(_s)  _s
# endif
#endif  /* ENABLE_NLS */

#ifdef  __cplusplus
}
#endif[=

ENDIF this is not a lib

=]
#endif /* [=(. header-guard)=] */[= # /*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * opthead.tpl ends here */=]
