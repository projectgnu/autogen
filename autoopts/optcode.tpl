[=autogen template include
 $Id: optcode.tpl,v 1.10 1998/08/17 14:19:04 bkorb Exp $ =]
[=_IF copyright _exist
=]
static const char zCopyright[] =
       [=prog_name _cap copyright _get owner _get
       "#3$%s copyright %s %s" _printf _str=];[=
  _ELSE =]
#define zCopyright (const char*)NULL[=
  _ENDIF=][=

_IF copyright_note _exist
=]
static const char zCopyrightNotice[] =
       [=copyright_note _str=];[=

_ELIF copyright_gpl _exist
=]
static const char zCopyrightNotice[] =
       [=prog_name _cap "#" _gpl _str=];[=

_ELIF copyright_lgpl _exist
=]
static const char zCopyrightNotice[] =
       [=prog_name _cap owner _get "#" _lgpl _str=];[=
  _ELSE =]
#define zCopyrightNotice (const char*)NULL[=

_ENDIF "copyright notes"
=]
#include "[=_eval HDRNAME _env=]"
[=
_FOR flag=][=
  _IF call_proc _exist=]
extern tOptProc   [=call_proc=];[=
  _ENDIF=][=
/flag=]
extern tUsageProc [=_IF usage _exist=][=usage=][=_ELSE=]optionUsage[=_ENDIF=];

#ifndef NULL
#  define NULL 0x0
#endif

#ifndef EXIT_SUCCESS
#  define  EXIT_SUCCESS 0
#  define  EXIT_FAILURE 1
#endif
[=
_IF include _exist=]
/*
 *  global included definitions
 */[=_FOR include "
"=]
[=include=][=/include=]
[=_ENDIF "include exists" =][=


_FOR FLAG "\n"


=]
/*
 *  [=name _cap=] option description[=
  _IF flags_must _exist flags_cant _exist | =] with
 *  "Must also have options" and "Incompatible options"[=
  _ENDIF=]:
 */
tSCC    z[=name _cap=]Text[] =
       [=descrip _str=];[=
  _IF documentation _exist ! =]
tSCC    z[=name _cap "#_NAME[] =" + %-28s _printf =] "[=name _up=]";[=

    _IF disable _len 0 > =]
tSCC    zNot[=name _cap "#_Name[] =" + %-25s _printf =] "[=disable _down=]-[=
    name _down "echo '%s'|tr -- '_^' '--'" _printf _shell=]";
tSCC    zNot[=name _cap "#_Pfx[] =" + %-25s _printf =] "[=disable _down=]";[=

      _IF enable _len 0 > =]
tSCC    z[=name _cap "#_Name[] =" + %-28s _printf =] "[=enable _down=]-[=
    name _down "echo '%s'|tr -- '_^' '--'" _printf _shell=]";[=

      _ELSE "Enable does not exist" =]
#define z[=name _cap "#_Name" + %-28s _printf
          =] (zNot[=name _cap=]_Name + [=_eval disable _len 1 + =])[=
      _ENDIF "" =][=

    _ELSE "Disable does not exist" =]
#define zNot[=name _cap "#_Pfx"   + %-25s _printf =] (const char*)NULL
#define zNot[=name _cap "#_Name"  + %-25s _printf =] (const char*)NULL
tSCC    z[=name _cap "#_Name[] =" + %-28s _printf =] "[=enable _down #- +=][=
    name _down "echo '%s'|tr -- '_^' '--'" _printf _shell=]";[=

    _ENDIF "disable" =][=


  # IF  there is a non-zero length argument
        *AND* it is not ':'
        *AND* it is not a numeric value, ...  =][=

    _IF flag_arg _len 0 >
      flag_arg _get #: !=
      flag_arg _get #=.* ~ !  & & =]
tSCC    z[=name _cap "DefaultArg[] =" + %-28s _printf =] [=flag_arg _str=];[=
    _ENDIF "" =][=


    _IF flags_must _exist=]
static const int
    a[=name _cap=]MustList[] = {[=_FOR flags_must=]
    INDEX_[=prefix _up #_ +=]OPT_[=flags_must _up=],[=/flags_must
           =] NO_EQUIVALENT};[=
    _ENDIF "" =][=


    _IF flags_cant _exist=]
static const int
    a[=name _cap=]CantList[] = {[=_FOR flags_cant=]
    INDEX_[=prefix _up #_ +=]OPT_[=flags_cant _up=],[=/flags_cant
           =] NO_EQUIVALENT};[=

    _ENDIF "flags_cant _exist" =][=

  _ENDIF documentation =][=

/FLAG

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
tSCC zMore_Help_Name[] = "more-help";
[=_IF version _exist=] 
/*
 *  Version option description:
 */
tSCC zVersionText[]  = "Output version information and exit";
tSCC zVersion_Name[] = "version";
[=_ENDIF=][=_IF homerc _exist=]
/*
 *  Save_Opts option description:
 */
tSCC zSave_OptsText[]  = "Save the computed option state to an rc file";
tSCC zSave_Opts_Name[] = "save-opts";
[=_ENDIF=][=


_IF flag.flag_code _exist flag.call_proc _exist |=][=

   # "For test builds, no need to call option procs" =][=

  _IF TEST_MAIN _env ! ! test_main _exist | =]
#if ! defined( TEST_[=prog_name _up #_ +=]OPTS )[=
  _ENDIF "TEST_MAIN _env ! ! test_main _exist |"

  =]
/*
 *  Procedures to call when option(s) are encountered
 */[=


  _FOR FLAG=][=

    _IF call_proc _exist=]
extern tOptProc [=call_proc=];[=

    _ELIF flag_code _exist=]
static tOptProc doOpt[=name _cap=];[=

    _ENDIF=][=
  /FLAG=]
[=
 
  _IF TEST_MAIN _env ! ! test_main _exist | =][=

     # "A test environment is to be generated" =]

#else /* *is*  defined( TEST_[=prog_name _up #_ +=]OPTS ) */
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */[=
    _FOR FLAG=][=

      _IF call_proc _exist=]
#define [=call_proc=] [=
          _IF max _get _val 1 > =]stackOptArg[=
          _ELSE =](tpOptProc)NULL[=
          _ENDIF=][=

      _ELIF flag_code _exist=]
#define doOpt[=name _cap=] [=
          _IF max _get _val 1 > =]stackOptArg[=
          _ELSE =](tpOptProc)NULL[=
          _ENDIF=][=

      _ENDIF=][=
    /FLAG=]
#endif /* defined( TEST_[=prog_name _up #_ +=]OPTS ) */[=_ENDIF=]
[=


_ENDIF "flag.flag_code _exist flag.call_proc _exist |" =]
/*
 *  These are always callable, whether
 *  TEST_[=prog_name _up #_ +=]OPTS is defined or not
 */
static tOptProc doUsageOpt;[=_IF version _exist=]
extern tOptProc doVersion[=_ENDIF=];
extern tOptProc doPagedUsage;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [=prog_name _cap=] Option Descriptions.
 */
static tOptDesc optDesc[ [=prefix _up #_ +=]OPTION_CT ] = {[=

_FOR flag "\n" =][=

  _IF documentation _exist =]
  {  /* entry idx, value */ 0, 0,
     /* equiv idx, value */ 0, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 1, 0,
     /* opt state flags  */ OPTST_DOCUMENT | OPTST_NO_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ [=
         _IF   call_proc _exist=][=call_proc=][=
         _ELIF flag_code _exist=]doOpt[=name _cap=][=
         _ELSE                 =](tpOptProc)NULL[=
         _ENDIF =],
     /* desc, NAME, name */ z[=name _cap=]Text, (const char*)NULL,
                            (const char*)NULL,
     /* disablement strs */ (const char*)NULL, (const char*)NULL },[=

  _ELSE

=]
  {  /* entry idx, value */ [=
          _eval _index=], VALUE_[=prefix _up #_ +=]OPT_[=name _up=],
     /* equiv idx, value */ [=
          _IF equivalence _exist equivalence _get name _get = &
              =]NO_EQUIVALENT, 0,[=
          _ELIF equivalence _exist
              =]~0, ~0,[=
          _ELSE
              =][=_eval _index=], VALUE_[=prefix _up #_ +=]OPT_[=name _up=],[=
          _ENDIF=]
     /* option argument  */ [=
         _IF   value _get _up NUMBER = =]ARG_MUST[=
         _ELIF flag_arg _exist !       =]ARG_NONE[=
         _ELIF flag_arg _len           =]ARG_MUST[=
         _ELSE                         =]ARG_MAY[=_ENDIF=],
     /* equivalenced to  */ [=
         _IF equivalence _exist equivalence _get name _get != &
             =]INDEX_[=prefix _up #_ +=]OPT_[=equivalence _up=][=
         _ELSE =]NO_EQUIVALENT[=_ENDIF=],
     /* min, max, act ct */ [=_eval min _get _val=], [=
         _IF max _exist=][=max=][=_ELSE=]1[=_ENDIF=], 0,
     /* opt state flags  */ [=
         _IF flag_arg _get #=.* ~=]OPTST_NUMERIC | [=     _ENDIF=][=
         _IF disable   _exist    =]OPTST_DISABLEOK | [=   _ENDIF=][=
         _IF stack_arg _exist    =]OPTST_STACKED | [=     _ENDIF=][=
         _IF enabled   _exist !  =]OPTST_DISABLED | [=
         _ELSE                   =]OPTST_INITENABLED | [= _ENDIF=][=
         _IF no_preset _exist    =]OPTST_NO_INIT[=
         _ELSE                   =]OPTST_INIT[=           _ENDIF=],
     /* last opt argumnt */ [=

         _IF   flag_arg _len 0  =
               flag_arg _get #: = |
               flag_arg _get #= = |
              =](char*)NULL[=

         _ELIF flag_arg _get #=.* ~
              =](char*)[=_eval flag_arg _get
              "echo %s|sed 's/=//'" _printf _shell =][=

         _ELSE=]z[=name _cap=]DefaultArg[=_ENDIF=],
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ [=
         _IF flags_must _exist=]a[=name _cap=]MustList[=
         _ELSE                =](const int*)NULL[=_ENDIF=], [=
         _IF flags_cant _exist=]a[=name _cap=]CantList[=
         _ELSE                =](const int*)NULL[=_ENDIF=],
     /* option proc      */ [=
         _IF   call_proc _exist=][=call_proc=][=
         _ELIF flag_code _exist=]doOpt[=name _cap=][=
         _ELIF flag_proc _exist=]doOpt[=flag_proc _cap=][=
         _ELIF stack_arg _exist=][=
               _IF equivalence _exist ! equivalence _get name _get = |
                      =]stackOptArg[=
               _ELSE  =]unstackOptArg[=_ENDIF=][=
         _ELSE               =](tpOptProc)NULL[=_ENDIF=],
     /* desc, NAME, name */ z[=name _cap=]Text,  z[=name _cap=]_NAME,
                            z[=name _cap=]_Name,
     /* disablement strs */ zNot[=name _cap=]_Name, zNot[=name _cap=]_Pfx },[=

  _ENDIF documentation =][=

/flag

=][=

_IF version _exist=]

  {  /* entry idx, value */ INDEX_[=prefix _up #_ +=]OPT_VERSION, VALUE_[=
                                    prefix _up #_ +=]OPT_VERSION,
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
_ENDIF=]

  {  /* entry idx, value */ INDEX_[=prefix _up #_ +=]OPT_HELP, VALUE_[=
                                    prefix _up #_ +=]OPT_HELP,
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

  {  /* entry idx, value */ INDEX_[=prefix _up #_ +=]OPT_MORE_HELP, VALUE_[=
                                    prefix _up #_ +=]OPT_MORE_HELP,
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

_IF homerc _exist
=],

  {  /* entry idx, value */ INDEX_[=prefix _up #_ +=]OPT_SAVE_OPTS, VALUE_[=
                                    prefix _up #_ +=]OPT_SAVE_OPTS,
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
     /* disablement strs */ (const char*)NULL, (const char*)NULL }[=
_ENDIF=]
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the [=prog_name _cap=] Option Environment
 */
tSCC   zPROGNAME[]   = "[=prog_name _up=]";[=

 # "The default Resource Config file name is target OS specific,
          but only if we have not been provided with a value."

=]
tSCC   zUsageTitle[] = "[=prog_name=] - " [=prog_title _str=]
    "\nUSAGE:  %s [=
        _IF flag.value _exist =][-<flag> [<val>]]... [=
        _ELIF long_opts _exist !=][<option-name>[{=| }<value>]] ...[=
        _ENDIF=][=
        _IF long_opts  _exist =][--<name>[{=| }<val>]]... [=
        _ENDIF=][=
        _IF argument _exist =][=

          #  IF the argument description is not likely to fit on a line ...
          =][=
          _IF flag.value _exist
              long_opts _exist &
              argument _len 16 >= &
              =]\\\n"
    "\t\t[=_ENDIF=]" [=argument _str=] "[=
        _ENDIF=]\n";[=

_IF homerc  _exist=]
tSCC   zRcName[]     = "[=
  _IF rcfile _len            =][=rcfile=][=
  _ELIF TARGETOS _env DOS = !=].[=prog_name _down=]rc[=
  _ELSE                      =][=prog_name  _up=].INI[=
  _ENDIF=]";
tSCC*  apzHomeList[] = {[=_FOR homerc=]
       [=homerc _str=],[=/homerc=]
       (char*)NULL };[=
_ELSE=]
#define zRcName     (tCC*)NULL
#define apzHomeList (tCC**)NULL[=
_ENDIF=][=

_IF explain _exist=]
tSCC   zExplain[]    = "\n"
       [=explain _str=] "\n";[=_ELSE=]
#define zExplain (const char*)NULL[=_ENDIF=]
[=
_IF detail _exist=]
tSCC    zDetail[]     = "\n"
       [=detail _str=] "\n";
[=_ELSE=]
#define zDetail (const char*)NULL[=_ENDIF=][=

_IF detail_file _exist=]
tSCC    zDetailFile[] = "[=_IF detail_file _len=][=detail_file=][=
                        _ELSE=][=prog_name=][=_ENDIF=]";[=_ELSE=]
#define zDetailFile (const char*)NULL[=_ENDIF=][=

_IF usage _exist ! =]
extern  tUsageProc optionUsage;[=_ENDIF=][=

_IF version _exist=]
tSCC    zFullVersion[] = [=prog_name _up=]_FULL_VERSION;[=

_ELSE=]
#define zFullVersion (const char*)NULL[=
_ENDIF=]

tOptions [=prog_name=]Options = {
    OPTIONS_STRUCT_VERSION,
    (char*)NULL,    (char*)NULL,    zPROGNAME,
    zRcName,        zCopyright,     zCopyrightNotice,
    zFullVersion,   apzHomeList,    zUsageTitle,
    zExplain,       zDetail,        zDetailFile,
    [=_IF usage _exist=][=usage=][=_ELSE=]optionUsage[=_ENDIF=],
    ( OPTPROC_NONE[=                 _IF allow_errors   _exist ! =]
    + OPTPROC_ERRSTOP[=    _ENDIF=][=_IF flag.disable   _exist   =]
    + OPTPROC_DISABLEOK[=  _ENDIF=][=_IF exerc          _exist   =]
    + OPTPROC_EXERC[=      _ENDIF=][=_IF flag.value     _exist   =]
    + OPTPROC_SHORTOPT[=   _ENDIF=][=_IF long_opts      _exist   =]
    + OPTPROC_LONGOPT[=    _ENDIF=][=_IF flag.min       _exist ! =]
    + OPTPROC_NO_REQ_OPT[= _ENDIF=][=_IF flag.disable   _exist   =]
    + OPTPROC_NEGATIONS[=  _ENDIF=][=_IF NUMBER_OPTION  _env     =]
    + OPTPROC_NUM_OPT[=    _ENDIF=][=_IF environrc      _exist   =]
    + OPTPROC_ENVIRON[=    _ENDIF=][=_IF plus_marks     _exist
                                         flag.disable   _exist & =]
    + OPTPROC_PLUSMARKS[=  _ENDIF=] ),
    0, (char*)NULL,
    { INDEX_[=prefix _up #_ +=]OPT_MORE_HELP,
      [=_IF homerc _exist=]INDEX_[=prefix _up #_ +=]OPT_SAVE_OPTS[=
        _ELSE            =] 0 /* no option state saving */[=_ENDIF=],
      [=_IF NUMBER_OPTION _env
             =][=_eval NUMBER_OPTION _env=] /* index of '-#' option */[=
        _ELSE=]NO_EQUIVALENT /* no '-#' option */[=_ENDIF=], 0 },
    [=prefix _up #_ +=]OPTION_CT, [=_eval flag _len=],
    optDesc
};

/*
 *  Create the static procedure(s) declared above.
 */
    static void
doUsageOpt( tOptions*  pOpts, tOptDesc* pOD )
{
    [=prefix _up #_ +=]USAGE( EXIT_SUCCESS );
}[=

_IF flag.flag_code _exist =][=

_IF TEST_MAIN _env ! !
    test_main _exist | =]

#if ! defined( TEST_[=prog_name _up #_ +=]OPTS )[=

_ENDIF "test_main _exist TEST_MAIN _env |" =]
/*
 *  Generated option processing procedures
 */[=

_FOR flag =][=

_IF flag_code _exist=]
/* * * * * * *
 *
 *   For the "[=name _cap=] Option".
 */
    static void
doOpt[=name _cap=]( tOptions* pOptions, tOptDesc* pOptDesc )
{
[=flag_code=]
}[=
_ENDIF "flag_code _exist" =][=/FLAG=][=
_ENDIF "flag.flag_code _exist" =][=


_IF TEST_MAIN _env ! ! test_main _exist | =]
[=_IF flag.flag_code _exist=]
#else /* *is* defined TEST_[=prog_name _up #_ +=]OPTS */
[=_ELSE=]
#if defined( TEST_[=prog_name _up #_ +=]OPTS )
[=_ENDIF=]
    void
main( int argc, char** argv )
{
    (void)optionProcess( &[=prog_name=]Options, argc, argv );[=
_IF test_main _len 3 >=]
    {
        void [=test_main=]( tOptions* );
        [=test_main=]( &[=prog_name=]Options );
    }[=

_ELIF TEST_MAIN _env 3 > =]
    {
        void [=_eval TEST_MAIN _env=]( tOptions* );
        [=_eval TEST_MAIN _env=]( &[=prog_name=]Options );
    }[=

_ELSE=]
    putBourneShell( &[=prog_name=]Options );[=

_ENDIF=]
    exit( EXIT_SUCCESS );
}
#endif  /* defined TEST_[=prog_name _up #_ +=]OPTS */[=
_ENDIF "test_main"=]
