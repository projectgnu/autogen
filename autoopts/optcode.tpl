[= autogen5 template  -*- Mode: C -*-
#$Id: optcode.tpl,v 2.33 2000/03/19 00:17:23 bruce Exp $
=]
#include "[=(. hdrname)=]"
[=
IF (not (exist? "copyright") )
=]
#define zCopyright       (const char*)NULL
#define zCopyrightNotice (const char*)NULL[=
ELSE  =]
tSCC zCopyright[] =
       [= (c-string
       (sprintf "%s copyright (c) %s %s, all rights reserved" (. prog-name)
                (get "copyright.date") (get "copyright.owner") )) =];
tSCC zCopyrightNotice[] =
       [=
  CASE (get "copyright.type") =][=

    =  gpl  =][=(c-string (gpl  (. prog-name) "" ))=][=

    = lgpl  =][=(c-string (lgpl (. prog-name) (get "copyright.owner")
                                "" ))=][=

    =  bsd  =][=(c-string (bsd  (. prog-name) (get "copyright.owner")
                                "" ))=][=

    = note  =][=(c-string (get "copyright.text"))=][=

    *       =]"Copyrighted"[=

  ESAC =];[=

ENDIF "copyright notes"=]
[=
FOR flag =][=
  IF (exist? "call_proc") =]
extern tOptProc   [= call_proc =];[=
  ENDIF =][=
ENDFOR flag=]
extern tUsageProc [=
  (if (exist? "usage") (get "usage") "optionUsage") =];

#ifndef NULL
#  define NULL 0x0
#endif

#ifndef EXIT_SUCCESS
#  define  EXIT_SUCCESS 0
#  define  EXIT_FAILURE 1
#endif
[=
IF (exist? "include") =]
/*
 *  global included definitions
 */[=

  FOR include "\n" =]
[=(get "include") =][=
  ENDFOR include =]
[=ENDIF "include exists" =][=
(define number-arg (make-regexp "=.*"))
(define cap-name "")

=][=


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #


=][=

FOR FLAG "\n" =][=

  Option_Strings =][=

ENDFOR FLAG
=]

/*
 *  Help option description:
 */
tSCC zHelpText[]  = "Display usage information and exit";
tSCC zHelp_Name[] = "help";

/*
 *  More_Help option description:
 */
tSCC zMore_HelpText[]  = "Extended usage information passed thru pager";
tSCC zMore_Help_Name[] = "more-help";[=
IF (exist? "version")
=]

/*
 *  Version option description:
 */
tSCC zVersionText[]    = "Output version information and exit";
tSCC zVersion_Name[]   = "version";[=
ENDIF (exist? "version")  =][=

IF (exist? "homerc")
=]

/*
 *  Save_Opts option description:
 */
tSCC zSave_OptsText[]  = "Save the option state to an rc file";
tSCC zSave_Opts_Name[] = "save-opts";

/*
 *  Load_Opts option description:
 */
extern tOptProc doLoadOpt;
tSCC    zLoad_OptsText[]     = "Load options from an rc file";
tSCC    zLoad_Opts_NAME[]    = "LOAD_OPTS";
tSCC    zNotLoad_Opts_Name[] = "no-load-opts";
tSCC    zNotLoad_Opts_Pfx[]  = "no";
#define zLoad_Opts_Name        (zNotLoad_Opts_Name + 3)[=
ENDIF (exist? "homerc") =][=


IF (or (exist? "flag.flag_code") (exist? "flag.call_proc")) =][=

   # "For test builds, no need to call option procs" =][=

  IF (exist? "test_main") =]
#if ! defined( TEST_[=(. pname-up)=]_OPTS )[=
  ENDIF

  =]
/*
 *  Procedures to call when option(s) are encountered
 */[=


  FOR flag =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

    IF (exist? "call_proc") =]
extern tOptProc [=(get "call_proc")=];[=

    ELIF (exist? "flag_code") =]
static tOptProc doOpt[=(. cap-name)=];[=

    ENDIF =][=
  ENDFOR flag   =][=
 
  IF (exist? "test_main") =][=

     # "A test environment is to be generated" =]

#else /* *is*  defined( TEST_[=(. pname-up)=]_OPTS ) */
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */[=
    FOR flag =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

      IF (exist? "call_proc") =]
#define [=(get "call_proc")=] [=
          IF   (not (exist? "max")) =](tpOptProc)NULL[=
          ELIF (> (string->number (get "max")) 1) =]stackOptArg[=
          ELSE =](tpOptProc)NULL[=
          ENDIF=][=

      ELIF (exist? "flag_code") =]
#define doOpt[=(. cap-name)=] [=
          IF   (not (exist? "max")) =](tpOptProc)NULL[=
          ELIF (> (string->number (get "max")) 1) =]stackOptArg[=
          ELSE =](tpOptProc)NULL[=
          ENDIF=][=

      ENDIF=][=
    ENDFOR flag=]
#endif /* defined( TEST_[=(. pname-up)=]_OPTS ) */[=
  ENDIF (exist? "test_main") =]
[=


ENDIF  (or (exist? "flag.flag_code") (exist? "flag.call_proc"))  =]
/*
 *  These are always callable, whether
 *  TEST_[=(. pname-up)=]_OPTS is defined or not
 */
static tOptProc doUsageOpt;[=
IF (exist? "version") =]
extern tOptProc doVersion;[=
ENDIF =]
extern tOptProc doPagedUsage;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [=(. pname-cap)=] Option Descriptions.
 */
static tOptDesc optDesc[ [=(. UP-prefix)=]OPTION_CT ] = {[=

(define default-opt-index -1)
(define up-name "") =][=

FOR flag "\n" =][=
  Option_Descriptor =][=

  ;;  IF this is the default option AND we already have one,...
  ;;  THEN remember this index
  ;;
  (if (and (exist? "default") (>= default-opt-index 0))
      (error (sprintf "\n\tDefault argument %d duplicates %d\n"
                      (for-index) default-opt-index) ))

  ;;  IF this is the default option AND this option takes an argument,
  ;;  THEN remember this index
  ;;
  (if (and (exist? "default")
           (> (len "flag_arg") 0))
      (set! default-opt-index (for-index)) ) =][=

ENDFOR flag

=][=

IF (exist? "version") =]

  {  /* entry idx, value */ INDEX_[= (. UP-prefix) =]OPT_VERSION, VALUE_[=
                                     (. UP-prefix) =]OPT_VERSION,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_MAY,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ doVersion,
     /* desc, NAME, name */ zVersionText,      (const char*)NULL,
                            zVersion_Name,
     /* disablement strs */ (const char*)NULL, (const char*)NULL },[=
ENDIF=]

  {  /* entry idx, value */ INDEX_[= (. UP-prefix) =]OPT_HELP, VALUE_[=
                                     (. UP-prefix) =]OPT_HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ doUsageOpt,
     /* desc, NAME, name */ zHelpText,         (const char*)NULL,
                            zHelp_Name,
     /* disablement strs */ (const char*)NULL, (const char*)NULL },

  {  /* entry idx, value */ INDEX_[= (. UP-prefix) =]OPT_MORE_HELP, VALUE_[=
                                     (. UP-prefix) =]OPT_MORE_HELP,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ doPagedUsage,
     /* desc, NAME, name */ zMore_HelpText,    (const char*)NULL,
                            zMore_Help_Name,
     /* disablement strs */ (const char*)NULL, (const char*)NULL }[=

IF homerc _exist
=],

  {  /* entry idx, value */ INDEX_[= (. UP-prefix) =]OPT_SAVE_OPTS, VALUE_[=
                                     (. UP-prefix) =]OPT_SAVE_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ '?',
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max act ct  */ 0, 1, 0,
     /* opt state flags  */ OPTST_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ (tOptProc*)NULL,
     /* desc, NAME, name */ zSave_OptsText,    (const char*)NULL,
                            zSave_Opts_Name,
     /* disablement strs */ (const char*)NULL, (const char*)NULL },

  {  /* entry idx, value */ INDEX_[= (. UP-prefix) =]OPT_LOAD_OPTS, VALUE_[=
                                     (. UP-prefix) =]OPT_LOAD_OPTS,
     /* equiv idx value  */ NO_EQUIVALENT, 0,
     /* option argument  */ ARG_MUST,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, NOLIMIT, 0,
     /* opt state flags  */ OPTST_DISABLEOK | OPTST_DISABLED | OPTST_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL, (const int*)NULL,
     /* option proc      */ doLoadOpt,
     /* desc, NAME, name */ zLoad_OptsText,  zLoad_Opts_NAME,
                            zLoad_Opts_Name,
     /* disablement strs */ zNotLoad_Opts_Name, zNotLoad_Opts_Pfx }[=
ENDIF=]
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [= (. pname-cap) =] Option Environment
 */
tSCC   zPROGNAME[]   = "[= (. pname-up) =]";
tSCC   zUsageTitle[] = [=
IF (exist? "version") =][= (. pname-up) =]_FULL_VERSION [=
ELSE                  =]"[=(. prog-name)=] - " [=
       (c-string (get "prog_title")) =][=
ENDIF =]
    "\n[= USAGE_LINE =]\n";[=

IF homerc  _exist=]
tSCC   zRcName[]     = "[=
  IF (> (string-length (get "rcfile")) 0)
        =][=rcfile=][=
  ELSE  =].[=(. pname-down)=]rc[=
  ENDIF =]";
tSCC*  apzHomeList[] = {[=
  FOR homerc=]
       [= (c-string (get "homerc")) =],[=
  ENDFOR homerc=]
       (char*)NULL };[=
ELSE=]
#define zRcName     (tCC*)NULL
#define apzHomeList (tCC**)NULL[=
ENDIF=][=

IF (exist? "explain") =]
tSCC   zExplain[]    = "\n"
       [= (c-string (get "explain")) =] "\n";[=
ELSE=]
#define zExplain (const char*)NULL[=
ENDIF=]
[=
IF (exist? "detail") =]
tSCC    zDetail[]     = "\n"
       [= (c-string (get "detail")) =] "\n";
[=
ELSE
=]
#define zDetail (const char*)NULL[=
ENDIF=][=

IF (exist? "detail_file") =]
tSCC    zDetailFile[] = [= (c-string (get "detail_file")) =];[=
ELSE=]
#define zDetailFile (const char*)NULL[=
ENDIF=][=

IF (not (exist? "usage")) =]
extern  tUsageProc optionUsage;[=
ENDIF=][=

IF (exist? "version") =]
tSCC    zFullVersion[] = [=(. pname-up)=]_FULL_VERSION;[=

ELSE=]
#define zFullVersion (const char*)NULL[=
ENDIF=]

tOptions [=(. pname)=]Options = {
    OPTIONS_STRUCT_VERSION,
    (char*)NULL,    (char*)NULL,    zPROGNAME,
    zRcName,        zCopyright,     zCopyrightNotice,
    zFullVersion,   apzHomeList,    zUsageTitle,
    zExplain,       zDetail,        zDetailFile,
    [=IF (exist? "usage")=][=usage=][=
      ELSE               =]optionUsage[=ENDIF=],
    ( OPTPROC_NONE[=                IF (not (exist? "allow_errors"))     =]
    + OPTPROC_ERRSTOP[=    ENDIF=][=IF      (exist? "flag.disable")      =]
    + OPTPROC_DISABLEOK[=  ENDIF=][=IF      (exist? "exerc")             =]
    + OPTPROC_EXERC[=      ENDIF=][=IF      (exist? "flag.value")        =]
    + OPTPROC_SHORTOPT[=   ENDIF=][=IF      (exist? "long_opts")         =]
    + OPTPROC_LONGOPT[=    ENDIF=][=IF (not (exist? "flag.min"))         =]
    + OPTPROC_NO_REQ_OPT[= ENDIF=][=IF      (exist? "flag.disable")      =]
    + OPTPROC_NEGATIONS[=  ENDIF=][=IF (>=   number-opt-index 0)         =]
    + OPTPROC_NUM_OPT[=    ENDIF=][=IF      (exist? "environrc")         =]
    + OPTPROC_ENVIRON[=    ENDIF=][=IF (and (exist? "plus_marks")
                                            (exist? "flag.disable"))     =]
    + OPTPROC_PLUSMARKS[=  ENDIF=][=IF (not (exist? "argument"))         =]
    + OPTPROC_NO_ARGS[=           ELIF (not (==* (get "argument") "[" )) =]
    + OPTPROC_ARGS_REQ[=   ENDIF=] ),
    0, (char*)NULL,
    { INDEX_[= (. UP-prefix) =]OPT_MORE_HELP,
      [=IF (exist? "homerc") 
             =]INDEX_[= (. UP-prefix) =]OPT_SAVE_OPTS[=
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
    optDesc
};

/*
 *  Create the static procedure(s) declared above.
 */
    static void
doUsageOpt( tOptions*  pOpts, tOptDesc* pOD )
{
    [= (. UP-prefix) =]USAGE( EXIT_SUCCESS );
}[=

IF (exist? "flag.flag_code") =][=

  IF (exist? "test_main") =]

#if ! defined( TEST_[= (. pname-up) =]_OPTS )[=

  ENDIF =][=

  FOR flag =][=

    IF (exist? "flag_code") =]

/* * * * * * *
 *
 *   For the "[=(string-capitalize! (get "name"))=] Option".
 */
    static void
doOpt[=(string-capitalize (get "name"))
     =]( tOptions* pOptions, tOptDesc* pOptDesc )
{
[=flag_code=]
}[=
    ENDIF "flag_code _exist" =][=
  ENDFOR flag =][=
ENDIF "flag.flag_code _exist"
=][=

IF (exist? "test_main") =][=
  IF (exist? "flag.flag_code") =]

#else /* *is* defined TEST_[= (. pname-up) =]_OPTS */[=

  ELSE=]
#if defined( TEST_[= (. pname-up) =]_OPTS )[=

  ENDIF

=]
    int
main( int argc, char** argv )
{[=

  IF (= (get "test_main") "putShellParse") =]
    extern tOptions  genshelloptOptions;
    extern void      putShellParse( tOptions* );
    extern tOptions* pShellParseOptions;

    /*
     *  Stash a pointer to the options we are generating.
     *  `genshellUsage()' will use it.
     */
    pShellParseOptions = &[=prog_name=]Options;
    (void)optionProcess( &genshelloptOptions, argc, argv );
    putShellParse( &[=prog_name=]Options );[=

  ELSE=]
    (void)optionProcess( &[=prog_name=]Options, argc, argv );[=
    IF (> (string-length (get "test_main")) 3) =]

    {
        void [=test_main=]( tOptions* );
        [=test_main=]( &[=prog_name=]Options );
    }
[=  ELSE=]
    putBourneShell( &[=prog_name=]Options );[=

    ENDIF =][=
  ENDIF=]
    return EXIT_SUCCESS;
}
#endif  /* defined TEST_[= (. pname-up) =]_OPTS */[=
ENDIF "test_main"=]
