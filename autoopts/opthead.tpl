[=autogen template include $Id: opthead.tpl,v 1.5 1998/07/02 23:51:55 bkorb Exp $ =]
[= # "This is the first time through.  Save the output file name
              so the 'C' file can '#include' it easily." =][=

_SETENV HDRNAME _outfile =][=

#  IF    long options are disallowed
     AND at least one flag character (value) is supplied
     AND the option count is not equal to the value count
   THEN ...  =][=

_IF long_opts  _exist !
    flag.value _exist
    flag.value _count flag _count != & & =][=
  _ERROR "Error:  long options are not allowed, therefore\n"
         "        every option must have exactly one value specified" + =][=
_ENDIF =][=


# "The #define name we use to self-exclude the header" =][=

_SETENV DEFNAME prog_name _get _outfile
    "#2$echo '%s_%s'|tr -- '-./[a-z]' '___[A-Z]'" _printf _shell=][=

# "Let the real output begin..."
#
=]#ifndef [=_eval DEFNAME _env=]
#define [=_eval DEFNAME _env=]
[=
_IF copyright _exist=]
/*
 * [=_eval prog_name _get _cap copyright _get owner _get
       "#3$%s copyright %s %s" _printf=][=_IF copyright_note _exist=]
 *
[=copyright_note=][=_ELIF copyright_gpl _exist=]
 *
[=_eval prog_name _get _cap "# * " _gpl=][=_ELIF copyright_lgpl _exist=]
 *
[=_eval prog_name _get _cap owner _get "# * " _lgpl=][=
_ENDIF "copyright notes" =]
 */[=
_ENDIF "copyright exists" =]
#include <options.h>

/*
 *  Enumeration of each option:
 */
typedef enum {[=_FOR flag=]
        INDEX_[=prefix _up #_ +=]OPT_[=name _up=],[=/FLAG=][=

_IF version _exist=]
        INDEX_[=prefix _up #_ +=]OPT_VERSION,[=_ENDIF

=]
        INDEX_[=prefix _up #_ +=]OPT_HELP,
        INDEX_[=prefix _up #_ +=]OPT_MORE_HELP[=

_IF homerc _exist=],
        INDEX_[=prefix _up #_ +=]OPT_SAVE_OPTS[=_ENDIF

=]
} te[=prefix _cap=]OptIndex;

/*
 *  Interface defines for the option
 *  indexes, values and total count
 */
#define [=prefix _up #_ +=]OPTION_CT    [=_eval
                  flag _count 2 +
                  version _exist +
                  homerc  _exist + =][=

          # "The '+ 2' is for the HELP and MORE_HELP options
                  and 'version' and 'homerc' macros add one option each"
=][=_IF version _exist=]
#define [=prog_name _up=]_VERSION              [=_eval version _get _str=]
#define [=prog_name _up=]_FULL_VERSION         "[=prog_name=] - [=prog_title
                                             "# - " +=]Ver. [=version=]"[=
_ENDIF=]
#define [=prefix _up #_ +=]DESC(n)      [=prog_name=]Options.pOptDesc[INDEX_[=
                 prefix _up #_ +=]OPT_ ## n]

/*
 *  Define the option values, and, for numerically valued options,
 *  the (argument) value of the option, and, for equivalenced-to
 *  options, a macro to identify the actual option value used.
 *  The AODFT_* define is for internal use only.
 */[=  _FOR flag=]
#define AODFT_[=prefix _up #_ +=]OPT_[=name _up "#%-14s" _printf=] [=
        _IF enabled _exist =]0[=_ELSE=]OPTST_DISABLED[=_ENDIF=]
#define VALUE_[=prefix _up #_ +=]OPT_[=name _up "#%-14s" _printf=] [=

        _IF   value _len 1 = =]'[=value=]'[=
        _ELIF value _get _up NUMBER = =]NUMBER_OPTION[=
              _SETENV NUMBER_OPTION _index =][=
        _ELIF value _exist =][=
              _ERROR name _get
                     "Error:  value for opt %s must be single char or 'NUMBER'"
                     _printf =][=
        _ELIF _index 0x20 <= =][=_eval _index=][=
        _ELSE                =][=_eval _index 96 +=][=
        _ENDIF=][=

        _IF flag_arg _get #=.* ~=]
#define [=prefix _up #_ +=]OPT_VALUE_[=name _up "#%-14s" _printf
                =] ((t_word)([=prefix _up #_ +=]OPT_ARG([=name _up=])))[=
        _ENDIF=][=


        _IF equivalence _get _UP name _get _UP ==]
#define WHICH_[=prefix _up #_ +=]OPT_[=name _up "#%-14s" _printf
                =] ([=prefix _up #_ +=]DESC([=name _up=]).optActualValue)[=
        _ENDIF=][=

        _IF setable _exist=][=

           _IF equivalence _exist equivalence _get _UP name _get _UP != &=]
#define SET_[=prefix _up #_ +=]OPT_[=name _up=][=
            _IF flag_arg _exist=](a)[=
            _ENDIF=]   STMTS( \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).fOptState |= OPTST_SET | OPTST_EQUIVALENCE[=
            _IF flag_arg _exist=]; \
        [=prefix _up #_ +=]DESC([=equivalence _up=]).pzLastArg  = [=
                 _IF flag_arg _get #=.* ~=](char*)atoi[=_ENDIF=](a)[=
            _ENDIF=][=_IF call_proc _exist
                          flag_code _exist |
                          flag_proc _exist |
                          stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                           prog_name=]Options, \
                         [=prog_name=]Options.pOptDesc + INDEX_[=
                         prefix _up #_ +=]OPT_[=equivalence _up=] )[=
            _ENDIF "callout procedure exists" =] )[=


              _ELSE "NOT equivalenced" =]
#define SET_[=prefix _up #_ +=]OPT_[=name _up=][=
            _IF flag_arg _exist=](a)[=
            _ENDIF=]   STMTS( \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState |= OPTST_SET[=
            _IF flag_arg _exist=]; \
        [=prefix _up #_ +=]DESC([=name _up=]).pzLastArg  = [=
                 _IF flag_arg _get #=.* ~=](char*)atoi[=_ENDIF=](a)[=
            _ENDIF=][=_IF call_proc _exist
                          flag_code _exist |
                          flag_proc _exist |
                          stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                  prog_name=]Options, \
                         [=prog_name=]Options.pOptDesc + INDEX_[=
                         prefix _up #_ +=]OPT_[=name _up=] )[=
            _ENDIF "callout procedure exists" =] )[=_ENDIF=][=


            _IF invertedopts _exist disable _exist | =]
#define NEG_[=prefix _up #_ +=]OPT_[=name _up=][=
            _IF flag_arg _exist=](a)[=
            _ENDIF=]   STMTS( \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=name _up
            =]).fOptState |= OPTST_SET | OPTST_DISABLED[=
            _IF flag_arg _exist=]; \
        [=prefix _up #_ +=]DESC([=name _up=]).pzLastArg  = [=
                 _IF flag_arg _get #=.* ~=](char*)atoi[=_ENDIF=](a)[=
            _ENDIF=][=_IF call_proc _exist
                          flag_code _exist |
                          flag_proc _exist |
                          stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                  prog_name=]Options, \
                         [=prog_name=]Options.pOptDesc + INDEX_[=
                         prefix _up #_ +=]OPT_[=name _up=] )[=
            _ENDIF "callout procedure exists" =] )[=
        _ENDIF "invertedopts _exist disable _exist |" =]
[=      _ENDIF setable exists =][=

    /flag=][=

  _IF flag.value _exist =][=
    _IF version _exist=]
#define VALUE_[=prefix _up #_ +=]OPT_VERSION        'v'[=
    _ENDIF =][=

    _IF homerc _exist=]
#define VALUE_[=prefix _up #_ +=]OPT_SAVE_OPTS      '>'[=
    _ENDIF
=]
#define VALUE_[=prefix _up #_ +=]OPT_HELP           '?'
#define VALUE_[=prefix _up #_ +=]OPT_MORE_HELP      '!'[=


  _ELSE "flag.value *DOES NOT* exist" =][=

    _IF version _exist =]
#define VALUE_[=prefix _up #_ +=]OPT_VERSION        INDEX_[=
                                      prefix _up #_ +=]OPT_VERSION[=
    _ENDIF=][=
    _IF homerc _exist =]
#define VALUE_[=prefix _up #_ +=]OPT_SAVE_OPTS      INDEX_[=
                                      prefix _up #_ +=]OPT_SAVE_OPTS[=
  _ENDIF=]
#define VALUE_[=prefix _up #_ +=]OPT_HELP           INDEX_[=
                                      prefix _up #_ +=]OPT_HELP
#define VALUE_[=prefix _up #_ +=]OPT_MORE_HELP      INDEX_[=
                                      prefix _up #_ +=]OPT_MORE_HELP
[=_ENDIF=]

/*
 *  Interface defines for all options.  Replace "n" with the UPPER_CASED
 *  option name (as in the te[=prefix _cap=]OptIndex enumeration above).
 *  e.g. HAVE_[=prefix _up #_ +=]OPT( [=

           # "Use the first option name as an example" =][=

          flag[0]=][=name _up=][=/flag=] )
 *
 *    CLEAR - Mark the option as if never present
 *    STATE - one of the OPTST_SET_MASK values (INIT, SET, PRESET, DEFINED)
 *    COUNT - Number of times option was DEFINED!!
 *     HAVE - Has option been SET, PRESET or DEFINED?
 *  OPT_ARG - Argument string for the option
 *    ISSEL - Has option been SET or DEFINED?
 * ISUNUSED - Has option never been SET, PRESET or DEFINED?
 * INVERTED - Was option marked with '+' instead of '-'?
 *  STACKCT - How many arguments were stacked?
 * STACKLST - ptr to list of ptrs to arguments
 *
 * Macros to alter processing state:
 *
 *  ERRSTOP - Stop and show usage for option errors
 *  ERRSKIP - Ignore option errors (tho processing stops)
 *  RESTART - Resume option processing from given argument index
 *    START - Start option processing from the beginning (index == 1)
 */
#define    CLEAR_[=prefix _up #_ +=]OPT(n) STMTS( \
                 [=prefix _up #_ +=]DESC(n).fOptState &= ~OPTST_SET_MASK; \
                 [=prefix _up #_ +=]DESC(n).fOptState |= OPTST_INIT | AODFT_[=
                           prefix _up #_ +=]OPT_ ## n; \
                 [=prefix _up #_ +=]DESC(n).optCookie = (void*)NULL )
#define    STATE_[=prefix _up #_ +=]OPT(n) ([=prefix _up #_ +
                 =]DESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_[=prefix _up #_ +=]OPT(n) ([=prefix _up #_ +=]DESC(n).optOccCt)
#define      [=prefix _up #_ +=]OPT_ARG(n) ([=prefix _up #_ +=]DESC(n).pzLastArg)
#define     HAVE_[=prefix _up #_ +=]OPT(n) (! UNUSED_OPT(&   [=prefix _up #_ +=]DESC(n)))
#define    ISSEL_[=prefix _up #_ +=]OPT(n) (SELECTED_OPT(&   [=prefix _up #_ +=]DESC(n)))
#define ISUNUSED_[=prefix _up #_ +=]OPT(n) (UNUSED_OPT(&     [=prefix _up #_ +=]DESC(n)))
#define  ENABLED_[=prefix _up #_ +=]OPT(n) (! DISABLED_OPT(& [=prefix _up #_ +=]DESC(n)))
#define  STACKCT_[=prefix _up #_ +=]OPT(n) (((tArgList*)([=prefix _up #_ +
                         =]DESC(n).optCookie))->useCt)
#define STACKLST_[=prefix _up #_ +=]OPT(n) (((tArgList*)([=prefix _up #_ +
                         =]DESC(n).optCookie))->apzArgs)
#define    WHICH_[=prefix _up #_ +=]OPT(n) ([=prefix _up #_ +
                         =]DESC(n).optActualValue)

#define  ERRSTOP_[=prefix _up #_ +=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet |= OPTPROC_ERRSTOP )
#define  ERRSKIP_[=prefix _up #_ +=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet &= ~OPTPROC_ERRSTOP )
#define  RESTART_[=prefix _up #_ +=]OPT(n) STMTS( [=prog_name  
                         =]Options.curOptIdx = (n); \
                               [=prog_name
                 =]Options.pzCurOpt  = (char*)NULL )
#define    START_[=prefix _up #_ +=]OPT    RESTART_[=prefix _up #_ +=]OPT(1)
#define     [=prefix _up #_ +=]USAGE(c) (*[=prog_name=]Options.pUsageProc)( &[=
                 prog_name=]Options, c )
/* * * * * *
 *
 *  Declare the [=prog_name=] option descriptions.
 */
#ifdef  __cplusplus
extern "C" {
#endif
[=flag=][=_IF call_proc _exist=]
extern tOptProc   [=call_proc=];[=_ENDIF=][=/flag=]
extern tUsageProc [=_IF usage _exist=][=usage=][=_ELSE=]optionUsage[=_ENDIF=];
extern tOptions   [=prog_name=]Options;[=

_IF export _exist=]

/* * * * * *
 *
 *  Globals exported from the [=prog_title=] option definitions
 */
[=_FOR export "

"=][=export=][=/export=][=_ENDIF=]

#ifdef  __cplusplus
}
#endif
#endif /* [=_eval DEFNAME _env=] */
