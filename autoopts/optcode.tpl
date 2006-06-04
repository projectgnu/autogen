[= autogen5 template  -*- Mode: Text -*-

#$Id: optcode.tpl,v 4.20 2006/06/04 22:14:04 bkorb Exp $

# Automated Options copyright 1992-2006 Bruce Korb
# Time-stamp:      "2006-06-04 15:13:37 bkorb"

=][=
;;; 
;;;  Compute the usage line.  It is complex because we are trying to
;;;  encode as much information as we can and still be comprehensible.
;;;
;;;  The rules are:  If any options have a "value" attribute, then
;;;  there are flags allowed, so include "-<flag>" on the usage line.
;;;  If the program has the "long_opts" attribute set, then we must
;;;  have "<option-name>" or "--<name>" on the line, depending on
;;;  whether or not there are flag options.  If any options take 
;;;  arguments, then append "[<val>]" to the flag description and
;;;  "[{=| }<val>]" to the option-name/name descriptions.  We won't
;;;  worry about being correct if every option has a required argument.
;;;  Finally, if there are no minimum occurrence counts (i.e. all
;;;  options are optional), then we put square brackets around the
;;;  syntax.
;;;

(define PROGNAME-ref (new-string-ref pname-up))

;;  Compute the option arguments
;;
(if (exist? "flag.arg-type")
    (set! tmp-val "[{=| }<val>]")
    (set! tmp-val "")
)

(define usage-line (string-append "USAGE:  %s "

  ;; If at least one option has a minimum occurrence count
  ;; we use curly brackets around the option syntax.
  ;;
  (if (not (exist? "flag.min")) "[ " "{ ")

  (if (exist? "flag.value")
      (string-append "-<flag>"
         (if (exist? "flag.arg-type") " [<val>]" "")
         (if (exist? "long-opts") " | " "") )
      (if (not (exist? "long-opts"))
         (string-append "<option-name>" tmp-val) "" )  )

  (if (exist? "long-opts")
      (string-append "--<name>" tmp-val) "" )

  (if (not (exist? "flag.min")) " ]..." " }...")
) )

(if (exist? "argument")
  (set! usage-line (string-append usage-line

        ;; the USAGE line plus the program name plus the argument goes
        ;; past 80 columns, then break the line, else separate with space
        ;;
        (if (< 80 (+ (string-length usage-line)
              (len "argument")
              (string-length prog-name) ))
            " \\\n\t\t"  " ")
        (get "argument")  ))
)

(define usage-ref (new-string-ref (string-append
        version-text "\n" usage-line "\n")))

=][= # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

INCLUDE "optmain.tpl"

=]

[=

IF (exist? "flag.arg-range")

=]#include <stdio.h>
#include <limits.h>[=

ENDIF  =][=

IF (or (= "optionPutShell"    (get "main.shell-process"))
       (= "optionParseShell"  (get "main.shell-parser"))
       (exist? "main.code")) =]
#define [= (set! make-test-main #t) main-guard =] 1[=
ENDIF
=]
#define OPTION_CODE_COMPILE 1
#include "[=

 (define lib-externs "")

 header-file =]"

#ifdef  __cplusplus
extern "C" {
#endif
[=

  CASE (get "copyright.type") =][=

    =  gpl  =][=(set! tmp-text (gpl  prog-name "" ))=][=
    = lgpl  =][=(set! tmp-text (lgpl prog-name (get "copyright.owner") ""))=][=
    =  bsd  =][=(set! tmp-text (bsd  prog-name (get "copyright.owner") ""))=][=
    = note  =][=(set! tmp-text (get  "copyright.text"))=][=
    *       =][=(set! tmp-text "Copyrighted")=][=

  ESAC      =][=

(if (not (exist? "copyright"))
    (begin
        (define cr-ref "NULL")
        (define cr-note-ref "NULL")
    )

    (begin
        (define cr-ref (new-string-ref
             (sprintf "%s copyright (c) %s %s, all rights reserved"
                 prog-name (get "copyright.date") (get "copyright.owner")
        )    ))

        (define cr-note-ref (new-string-ref tmp-text))
)   )

=]
extern tUsageProc [=
  (define usage-proc (get "usage" "optionUsage"))
  usage-proc =];
[=

IF (exist? "include")

=]
/*
 *  global included definitions
 */
[=(join "\n" (stack "include"))  =]
[=

ENDIF "include exists"

=]
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

  INVOKE Option_Strings   =][=

  (if (exist? "lib-name") (begin
      (set! lib-opt-ptr (string->c-name! (string-append
                        (get "lib-name") "_" (get "name") "_optDesc_p")))
      (set! lib-externs (string-append lib-externs
                        (sprintf "tOptDesc const* %-16s = optDesc + %d;\n"
                                 lib-opt-ptr (for-index) )  ))
  )   )             =][=

ENDFOR flag
=]

/*
 *  Help/More_Help[= version "/Version"=] option descriptions:
 */
#define zHelpText           [=
 (new-string-ref "Display usage information and exit") =]
#define zHelp_Name          [= (new-string-ref "help") =]

#define zMore_HelpText      [=
 (new-string-ref "Extended usage information passed thru pager") =]
#define zMore_Help_Name     [= (new-string-ref "more-help") =][=

IF (exist? "version")

=]

#define zVersionText        [=
 (new-string-ref "Output version information and exit") =]
#define zVersion_Name       [= (new-string-ref "version") =][=

ENDIF (exist? "version")    =][=

IF (exist? "homerc")

=]

/*
 *  Save/Load_Opts option description:
 */
#define zSave_OptsText      [=
 (new-string-ref "Save the option state to a config file") =]
#define zSave_Opts_Name     [=
 (new-string-ref "save-opts") =]

#define zLoad_OptsText      [=
 (new-string-ref "Load options from a config file") =]
#define zLoad_Opts_NAME     [= (new-string-ref "LOAD_OPTS") =]

#define zNotLoad_Opts_Name  [= (new-string-ref "no-load-opts") =]
#define zNotLoad_Opts_Pfx   [= (new-string-ref "no") =]
#define zLoad_Opts_Name     (zNotLoad_Opts_Name + 3)[=

ENDIF (exist? "homerc")     =][=

INVOKE declare-callbacks    =][=

IF (and (exist? "version") make-test-main) =]
#ifdef [=(. main-guard)     =]
# define DOVERPROC optionVersionStderr
#else
# define DOVERPROC optionPrintVersion
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
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 1, 0,
     /* opt state flags  */ OPTST_SET_ARGTYPE(OPARG_TYPE_STRING)
                          | OPTST_ARG_OPTIONAL, 0,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ [=
         (if make-test-main "DOVERPROC" "optionPrintVersion")=],
     /* desc, NAME, name */ zVersionText, NULL, zVersion_Name,
     /* disablement strs */ NULL, NULL },[=

ENDIF =]

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]HELP, [= (. VALUE-pfx) =]HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 1, 0,
     /* opt state flags  */ OPTST_IMM, 0,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ doUsageOpt,
     /* desc, NAME, name */ zHelpText, NULL, zHelp_Name,
     /* disablement strs */ NULL, NULL },

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]MORE_HELP, [= (. VALUE-pfx) =]MORE_HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 1, 0,
     /* opt state flags  */ OPTST_IMM, 0,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL,  NULL,
     /* option proc      */ optionPagedUsage,
     /* desc, NAME, name */ zMore_HelpText, NULL, zMore_Help_Name,
     /* disablement strs */ NULL, NULL }[=

IF (exist? "homerc")

=],

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]SAVE_OPTS, [= (. VALUE-pfx) =]SAVE_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 1, 0,
     /* opt state flags  */ OPTST_SET_ARGTYPE(OPARG_TYPE_STRING)
                          | OPTST_ARG_OPTIONAL, 0,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL,  NULL,
     /* option proc      */ NULL,
     /* desc, NAME, name */ zSave_OptsText, NULL, zSave_Opts_Name,
     /* disablement strs */ NULL, NULL },

  {  /* entry idx, value */ [=
        (. INDEX-pfx) =]LOAD_OPTS, [= (. VALUE-pfx) =]LOAD_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, NOLIMIT, 0,
     /* opt state flags  */ OPTST_DISABLE_IMM | \
			OPTST_SET_ARGTYPE(OPARG_TYPE_STRING), 0,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ optionLoadOpt,
     /* desc, NAME, name */ zLoad_OptsText, zLoad_Opts_NAME, zLoad_Opts_Name,
     /* disablement strs */ zNotLoad_Opts_Name, zNotLoad_Opts_Pfx }[=

ENDIF

=]
};
[= (. lib-externs) =]
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [= (. pname-cap) =] Option Environment
 */[=

(define rc-ref (if (not (exist? "homerc")) "NULL"
        (new-string-ref (if (exist? "rcfile") (get "rcfile")
             (string-append "." pname-down "rc")  ))  ))

(define bug-ref (if (exist? "copyright.eaddr")
        (new-string-ref (get "copyright.eaddr"))
        (if (exist? "eaddr")
        (new-string-ref (get "eaddr"))
        "NULL"  )))

(define full-ver-ref (if (not (exist? "version")) "NULL"
        (new-string-ref version-text) ))

(define patch-text (lambda (t-name)
  (string-append
    "\n"

    (shell (string-append
      "sed 's/@[a-z]*{\\([^{@}]*\\)}/``\\1'\"''/g\" <<'_EODetail_'\n"
      (get t-name)
      "\n_EODetail_" ))

    "\n"
) ))

(define explain-ref
  (if (or (exist? "explain") (== (get "main.main-type") "for-each"))
      (begin
 	 (if (exist? "explain")
 	     (set! tmp-text (patch-text "explain"))
 	     (set! tmp-text "")  )
         (if (== (get "main.main-type") "for-each")
             (set! tmp-text (string-append tmp-text

"\nIf no arguments are provided, input arguments are read from stdin,
one per line; blank and `#'-prefixed lines are comments.
'stdin' may not be a terminal (tty).\n" ))

         )

         (new-string-ref tmp-text)
      )

      "NULL"
) )

(define detail-ref (if (not (exist? "detail")) "NULL"
  (new-string-ref (patch-text "detail")) ))

(tpl-file-line extract-fmt)

=][=

IF (exist? "homerc")

=]
tSCC*  apzHomeList[] = {[=
  FOR homerc            =]
    [= (new-string-ref (get "homerc")) =],[=
  ENDFOR homerc=]
    NULL };[=

ELSE

=]
#define apzHomeList NULL[=

ENDIF

=]
#if defined(ENABLE_NLS)
# define OPTPROC_BASE OPTPROC_TRANSLATE
  static tOptionXlateProc translate_option_strings;
#else
# define OPTPROC_BASE OPTPROC_NONE
# define translate_option_strings NULL
#endif /* ENABLE_NLS */

tOptions [=(. pname)=]Options = {
    OPTIONS_STRUCT_VERSION,
    0, NULL,                    /* original argc + argv    */
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
    0, NULL,                    /* current option index, current option */
    NULL,         NULL,         [= (. PROGNAME-ref) =],
    [= (. rc-ref)      =], [= (. cr-ref) =],   [= (. cr-note-ref) =],
    [= (. full-ver-ref)=], apzHomeList,  [= (. usage-ref) =],
    [= (. explain-ref) =], [= (. detail-ref) =],      optDesc,
    [= (. bug-ref)     =], /* address to send bugs to */
    NULL, NULL,                 /* extensions/saved state  */
    [= (. usage-proc) =],       /* usage procedure */
    translate_option_strings,   /* translation procedure */
    /*
     *  Indexes to special options
     */
    { [= (. INDEX-pfx) =]MORE_HELP,
      [=IF (exist? "homerc")
             =][= (. INDEX-pfx) =]SAVE_OPTS[=
        ELSE =] 0 /* no option state saving */[=
        ENDIF=],
      [= (if (>= number-opt-index 0) number-opt-index "NO_EQUIVALENT")
        =] /* index of '-#' option */,
      [= (if (>= default-opt-index 0) default-opt-index "NO_EQUIVALENT")
        =] /* index of default opt */
    },
    [= (. UP-prefix) =]OPTION_CT, [=(count "flag")=] /* user option count */
};
[=

FOR lib-name

=]
tOptDesc* [= (string->c-name! (get "lib-name")) =]_optDesc_p = NULL;
[=

ENDFOR

=]
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
