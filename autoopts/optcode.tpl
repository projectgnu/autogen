[= autogen5 template  -*- Mode: Text -*-

#$Id: optcode.tpl,v 4.7 2005/02/04 05:10:07 bkorb Exp $

# Automated Options copyright 1992-2005 Bruce Korb

=][=

INCLUDE "optmain.tpl" =][=

IF (exist? "flag.arg-range")

=]#include <stdio.h>
#include <limits.h>[=

ENDIF  =][=

IF (or (= "putBourneShell" (get "main.shell-process"))
       (= "putShellParse"  (get "main.shell-parser"))
       (exist? "main.code")) =]
#define [= (set! make-test-main #t) main-guard =] 1[=
ENDIF
=]
#define OPTION_CODE_COMPILE 1
#include "[= (. header-file) =]"

#ifdef  __cplusplus
extern "C" {
#endif[=

IF (not (exist? "copyright") )

=]
#define zCopyright       NULL
#define zCopyrightNotice NULL[=
ELSE  =]
tSCC zCopyright[] =
       [= (set! tmp-text (kr-string
       (sprintf "%s copyright (c) %s %s, all rights reserved" (. prog-name)
                (get "copyright.date") (get "copyright.owner") )))
       tmp-text =];
tSCC zCopyrightNotice[] =
       [=

  CASE (get "copyright.type") =][=

    =  gpl  =][=(set! tmp-text (gpl  prog-name "" ))=][=
    = lgpl  =][=(set! tmp-text (lgpl prog-name (get "copyright.owner") ""))=][=
    =  bsd  =][=(set! tmp-text (bsd  prog-name (get "copyright.owner") ""))=][=
    = note  =][=(set! tmp-text (get  "copyright.text"))=][=
    *       =][=(set! tmp-text "Copyrighted")=][=

  ESAC =][=

(emit (def-file-line "copyright.text" extract-fmt))
(kr-string tmp-text) =];[=

ENDIF "copyright notes"

=]
extern tUsageProc [=
  (define usage-proc (get "usage" "optionUsage"))
  usage-proc =];
[=
IF (exist? "include") =]
/*
 *  global included definitions
 */
[=(join "\n" (stack "include"))  =]
[=ENDIF "include exists" =]
#ifndef NULL
#  define NULL 0
#endif
#ifndef EXIT_SUCCESS
#  define  EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#  define  EXIT_FAILURE 1
#endif[=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

FOR flag "\n"      =][=

  Option_Strings   =][=

ENDFOR flag
=]

/*
 *  Help/More_Help[= version "/Version"=] option descriptions:
 */
tSCC zHelpText[]       = "Display usage information and exit";
tSCC zHelp_Name[]      = "help";

tSCC zMore_HelpText[]  = "Extended usage information passed thru pager";
tSCC zMore_Help_Name[] = "more-help";[=

IF (exist? "version")

=]

tSCC zVersionText[]    = "Output version information and exit";
tSCC zVersion_Name[]   = "version";[=
ENDIF (exist? "version")  =][=

IF (exist? "homerc")
=]

/*
 *  Save/Load_Opts option description:
 */
tSCC zSave_OptsText[]     = "Save the option state to a config file";
tSCC zSave_Opts_Name[]    = "save-opts";

tSCC zLoad_OptsText[]     = "Load options from a config file";
tSCC zLoad_Opts_NAME[]    = "LOAD_OPTS";

tSCC zNotLoad_Opts_Name[] = "no-load-opts";
tSCC zNotLoad_Opts_Pfx[]  = "no";
#define zLoad_Opts_Name   (zNotLoad_Opts_Name + 3)[=
ENDIF (exist? "homerc") =][=

  invoke declare-option-callbacks  =][=

IF (and (exist? "version") make-test-main) =]
#ifdef [=(. main-guard)     =]
# define DOVERPROC doVersionStderr
#else
# define DOVERPROC doVersion
#endif /* [=(. main-guard)=] */[=
ENDIF   =]

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [=(. pname-cap)=] Option Descriptions.
 */
static tOptDesc optDesc[ [=(. UP-prefix)=]OPTION_CT ] = {[=

FOR flag "\n"           =][=

  INVOKE option_descriptor =][=

ENDFOR flag

=][=

IF (exist? "version")   =]

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]VERSION, [= (. VALUE-pfx) =]VERSION,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_MAY,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ [=(if make-test-main "DOVERPROC" "doVersion")=],
     /* desc, NAME, name */ zVersionText, NULL, zVersion_Name,
     /* disablement strs */ NULL, NULL },[=

ENDIF =]

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]HELP, [= (. VALUE-pfx) =]HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_IMM,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ doUsageOpt,
     /* desc, NAME, name */ zHelpText, NULL, zHelp_Name,
     /* disablement strs */ NULL, NULL },

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]MORE_HELP, [= (. VALUE-pfx) =]MORE_HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_IMM,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL,  NULL,
     /* option proc      */ doPagedUsage,
     /* desc, NAME, name */ zMore_HelpText, NULL, zMore_Help_Name,
     /* disablement strs */ NULL, NULL }[=

IF (exist? "homerc")

=],

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]SAVE_OPTS, [= (. VALUE-pfx) =]SAVE_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ '?',
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL,  NULL,
     /* option proc      */ NULL,
     /* desc, NAME, name */ zSave_OptsText, NULL, zSave_Opts_Name,
     /* disablement strs */ NULL, NULL },

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]LOAD_OPTS, [= (. VALUE-pfx) =]LOAD_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_MUST,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, NOLIMIT, 0,
     /* opt state flags  */ OPTST_DISABLE_IMM,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ doLoadOpt,
     /* desc, NAME, name */ zLoad_OptsText, zLoad_Opts_NAME, zLoad_Opts_Name,
     /* disablement strs */ zNotLoad_Opts_Name, zNotLoad_Opts_Pfx }[=

ENDIF

=]
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [= (. pname-cap) =] Option Environment
 */
tSCC   zPROGNAME[]   = "[= (. pname-up) =]";
tSCC   zUsageTitle[] =
[= USAGE_LINE           =];[=

IF (exist? "homerc") =]
tSCC   zRcName[]     = "[=
  (if (not (exist? "rcfile"))
      (string-append "." pname-down "rc")
      (get "rcfile") ) =]";
tSCC*  apzHomeList[] = {[=
  FOR homerc            =]
       [= (kr-string (get "homerc")) =],[=
  ENDFOR homerc=]
       NULL };[=

ELSE                    =]
#define zRcName     NULL
#define apzHomeList NULL[=
ENDIF                   =][=

(define patch-text (lambda (t-name)
  (set! tmp-text (kr-string (string-append "\n"

  (shell (string-append
    "sed 's/@[a-z]*{\\([^{@}]*\\)}/``\\1'\"''/g\" <<'_EODetail_'\n"
    (get t-name)
    "\n_EODetail_" ))
  "\n" ))) ))

(define bug-text "\n\ntSCC   zBugsAddr[]    = %s;")

(if (exist? "copyright.eaddr")
    (sprintf bug-text (kr-string (get "copyright.eaddr")))

    (if (exist? "eaddr")
        (sprintf bug-text (kr-string (get "eaddr")))

        "\n\n#define zBugsAddr NULL" )  )

                        =][=

IF (or (exist? "explain") (== (get "main.main-type") "for-each"))  =]
tSCC   zExplain[]     = [=

 (if (exist? "explain")
     (patch-text "explain")
     (set! tmp-text "")  )

 (if (== (get "main.main-type") "for-each")
   (set! tmp-text (string-append tmp-text

"\n\"If no arguments are provided, input arguments are read from stdin,\\n\\
one per line; blank and '#'-prefixed lines are comments.\\n\\
'stdin' may not be a terminal (tty).\\n\"" ))  )

 tmp-text =];[=

ELSE                    =]
#define zExplain NULL[=
ENDIF                   =][=

IF (exist? "detail")    =]
tSCC    zDetail[]     = [= (patch-text "detail") tmp-text =];[=

ELSE                    =]
#define zDetail         NULL[=
ENDIF                   =][=

IF (exist? "version")   =]
tSCC    zFullVersion[] = [=(. pname-up)=]_FULL_VERSION;[=

ELSE                    =]
#define zFullVersion    NULL[=
ENDIF                   =][=
(tpl-file-line extract-fmt)
=]
#if defined(ENABLE_NLS)
# define OPTPROC_BASE OPTPROC_TRANSLATE
  static option_translation_proc_t translate_option_strings;
#else
# define OPTPROC_BASE OPTPROC_NONE
# define translate_option_strings NULL
#endif /* ENABLE_NLS */

tOptions [=(. pname)=]Options = {
    OPTIONS_STRUCT_VERSION,
    NULL,         NULL,         zPROGNAME,
    zRcName,      zCopyright,   zCopyrightNotice,
    zFullVersion, apzHomeList,  zUsageTitle,
    zExplain,     zDetail,      NULL,           [= (. usage-proc) =],
    ( OPTPROC_BASE[=                IF (not (exist? "allow-errors"))     =]
    + OPTPROC_ERRSTOP[=    ENDIF=][=IF      (exist? "flag.value")        =]
    + OPTPROC_SHORTOPT[=   ENDIF=][=IF      (exist? "long-opts")         =]
    + OPTPROC_LONGOPT[=    ENDIF=][=IF (not (exist? "flag.min"))         =]
    + OPTPROC_NO_REQ_OPT[= ENDIF=][=IF      (exist? "flag.disable")      =]
    + OPTPROC_NEGATIONS[=  ENDIF=][=IF (>=   number-opt-index 0)         =]
    + OPTPROC_NUM_OPT[=    ENDIF=][=IF      (exist? "environrc")         =]
    + OPTPROC_ENVIRON[=    ENDIF=][=IF (and (exist? "plus-marks")
                                            (exist? "flag.disable"))     =]
    + OPTPROC_PLUSMARKS[=  ENDIF=][=IF (not (exist? "argument"))         =]
    + OPTPROC_NO_ARGS[=           ELIF (not (==* (get "argument") "[" )) =]
    + OPTPROC_ARGS_REQ[=   ENDIF=][=IF      (exist? "reorder-args")      =]
    + OPTPROC_REORDER[=    ENDIF=][=IF      (exist? "gnu-usage")         =]
    + OPTPROC_GNUUSAGE[=   ENDIF=][=IF (or  (exist? "flag.immediate")
                                            (exist? "flag.immed-disable")
                                            (exist? "homerc")  )         =]
    + OPTPROC_HAS_IMMED[=  ENDIF=] ),
    0, NULL,
    { [= (. INDEX-pfx) =]MORE_HELP,
      [=IF (exist? "homerc")
             =][= (. INDEX-pfx) =]SAVE_OPTS[=
        ELSE =] 0 /* no option state saving */[=
        ENDIF=],
      [= IF (>= number-opt-index 0)
              =][= (. number-opt-index) =] /* index of '-#' option */[=
         ELSE =]NO_EQUIVALENT /* no '-#' option */[=
         ENDIF  =],
      [=
         IF (>= default-opt-index 0)
              =][= (. default-opt-index) =] /* index of default opt */[=
         ELSE =]NO_EQUIVALENT /* no default option */[=
         ENDIF =] },
    [= (. UP-prefix) =]OPTION_CT, [=(count "flag")=] /* user option count */,
    optDesc,
    0, NULL,          /* original argc + argv    */
    zBugsAddr,        /* address to send bugs to */
    translate_option_strings
};

/*
 *  Create the static procedure(s) declared above.
 */
static void
doUsageOpt(
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
{
    [= (. UP-prefix) =]USAGE( EXIT_SUCCESS );
}[=

IF (or (exist? "flag.flag-code")
       (exist? "flag.extract-code")
       (exist? "flag.arg-range")
       (match-value? ~* "flag.arg-type" "key|set")) =][=

  invoke  define-option-callbacks  =][=

ENDIF                              =][=

IF (. make-test-main)              =][=
  invoke build-test-main           =][=

ELIF (exist? "guile-main")         =][=
  invoke build-guile-main          =][=

ELIF (exist? "main")               =][=
  invoke build-main                =][=

ENDIF "test/guile main"

=][=
(tpl-file-line extract-fmt)
=]
#if ENABLE_NLS
#include <string.h>
#include <stdio.h>
#include <autoopts/usage-txt.h>

static char*
AO_gettext( const char* pz )
{
    char* pzRes;
    if (pz == NULL)
        return NULL;
    pzRes = _(pz);
    if (pzRes == pz)
        return pzRes;
    pzRes = strdup( pzRes );
    if (pzRes == NULL) {
        fputs( _("No memory for duping translated strings\n"), stderr );
        exit( EXIT_FAILURE );
    }
    return pzRes;
}

/*
 *  This invokes the translation code (e.g. gettext(3)).
 */
static void
translate_option_strings( void )
{
    /*
     *  Guard against re-translation.  It won't work.  The strings will have
     *  been changed by the first pass through this code.  One shot only.
     */
    if (option_usage_text.field_ct == 0)
        return;
    /*
     *  Do the translations.  The first pointer follows the field count field.
     *  The field count field is the size of a pointer.
     */
    {
        char** ppz = (char**)(void*)&(option_usage_text);
        int    ix  = option_usage_text.field_ct;

        do {
            ppz++;
            *ppz = AO_gettext(*ppz);
        } while (--ix > 0);
    }
    option_usage_text.field_ct = 0;

    {
        tOptDesc* pOD = [=(. pname)=]Options.pOptDesc;
        int       ix  = [=(. pname)=]Options.optCt;

        for (;;) {[=

  FOR field IN pzText pz_NAME pz_Name pz_DisableName pz_DisablePfx  =][=

    (sprintf "\n            pOD->%1$-16s = AO_gettext(pOD->%1$s);"
             (get "field"))  =][=

  ENDFOR =]
            if (--ix <= 0)
                break;
            pOD++;
        }
    }[=

  FOR field IN pzCopyright pzCopyNotice pzFullVersion pzUsageTitle
               pzExplain pzDetail  =][=

    (sprintf "\n    %1$sOptions.%2$-13s = AO_gettext(%1$sOptions.%2$s);"
             pname (get "field"))  =][=

  ENDFOR =]
}

#endif /* ENABLE_NLS */

#ifdef  __cplusplus
}
#endif
