[=autogen template include
#$Id: opthead.tpl,v 2.7 1998/09/28 19:33:01 bkorb Exp $
=]
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
=]
/*
 *  This file contains the programmatic interface to the
 *  Automated Options generated for the [=prog_name=] program.
 *  These macros are documented in the Autogen info file
 *  in the "Autoopts" chapter.  Please refer to that doc
 *  for usage help.
 */

#ifndef [=_eval DEFNAME _env=]
#define [=_eval DEFNAME _env=]
[=
_IF copyright _exist=]
/*
 * [=_eval prog_name _get _cap copyright _get owner _get
       "#3$%s copyright %s %s" _printf=][=

  _IF copyright_note _exist=]
 *
[=copyright_note=][=

  _ELIF copyright_gpl _exist=]
 *
[=_eval prog_name _get _cap "# * " _gpl=][=

  _ELIF copyright_lgpl _exist=]
 *
[=_eval prog_name _get _cap owner _get "# * " _lgpl=][=
  _ENDIF "copyright notes" =]
 */[=
_ENDIF "copyright exists" =]
#include <options.h>

/*
 *  Enumeration of each option:
 */[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac teOptIndex
 *
 *  title:  Option Index and Enumeration
 *
 *  description:
 *
 *  "This enum defines the complete set of options, both
 *  user specified and automatically provided.  This can be used,
 *  for example, to distinguish which of the equivalenced options
 *  was actually used.
 *
 *  @example
 *  switch (pOptDesc->optActualIndex) @{
 *  case INDEX_OPT_FIRST:\n"
 *  "    stuff;
 *  case INDEX_OPT_DIFFERENT:\n"
 *  "    different-stuff;\n"
 *  "default:\n"
 *  "    unknown-things;
 *  @}
 *  @end example"
=*/
=]
typedef enum {[=
_FOR flag=][=
  _IF documentation _exist ! =]
        INDEX_[=prefix _up #_ +=]OPT_[=name _up #%-16s _printf
               =] =[=_EVAL _index #%3d _printf=],[=
  _ENDIF =][=
/FLAG=][=

_EVAL flag _count "OPTCT=%d" _printf _shell =][=

_IF version _exist=]
        INDEX_[=prefix _up #_ +=]OPT_VERSION          = [=
                _EVAL "echo $OPTCT ; OPTCT=`expr $OPTCT + 1`" _shell=],[=
_ENDIF=]
        INDEX_[=prefix _up #_ +=]OPT_HELP             = [=
                _EVAL "echo $OPTCT ; OPTCT=`expr $OPTCT + 1`" _shell=],
        INDEX_[=prefix _up #_ +=]OPT_MORE_HELP        = [=
                _EVAL "echo $OPTCT ; OPTCT=`expr $OPTCT + 1`" _shell=][=

_IF homerc _exist=],
        INDEX_[=prefix _up #_ +=]OPT_SAVE_OPTS        = [=
                _EVAL "echo $OPTCT ; OPTCT=`expr $OPTCT + 1`" _shell=],
        INDEX_[=prefix _up #_ +=]OPT_LOAD_OPTS        = [=
                _EVAL "echo $OPTCT ; OPTCT=`expr $OPTCT + 1`" _shell=][=
_ENDIF=]
} te[=prefix _cap=]OptIndex;
[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac OPTION_CT
 *
 *  title:  Full Count of Options
 *
 *  description:
 *
 *  The full count of all options, both those defined
 *  and those generated automatically by AutoOpts.  This is primarily
 *  used to initialize the program option descriptor structure.
=*/
=]
#define [=prefix _up #_ +=]OPTION_CT    [=_eval "echo $OPTCT" _shell =][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac VERSION
 *
 *  title:  Version and Full Version
 *
 *  description:
 *
 *  If the @code{version} attribute is defined for the program,
 *  then a stringified version will be #defined as PROGRAM_VERSION and
 *  PROGRAM_FULL_VERSION.  PROGRAM_FULL_VERSION is used for printing
 *  the program version in response to the version option.  The version
 *  option is automatically supplied in response to this attribute, too.
 *
 *  You may access PROGRAM_VERSION via @code{programOptions.pzFullVersion}.
=*/
=][=

_IF version _exist =]
#define [=prog_name _up=]_VERSION       [=version _str=]
#define [=prog_name _up=]_FULL_VERSION  "[=prog_name=] - [=prog_title
                                             "# - " +=]Ver. [=version=]"[=
_ENDIF version-exists =]

/*
 *  Interface defines for all options.  Replace "n" with
 *  the UPPER_CASED option name (as in the te[=prefix _cap=]OptIndex
 *  enumeration above).  e.g. HAVE_[=prefix _up #_ +=]OPT( [=

    _FOR flag[0]=][=name _up=][=/flag=] )
 */[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac DESC
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Option Descriptor
 *
 *  description:
 *
 *  This macro is used internally by other AutoOpt macros.
 *  It is not for general use.  It is used to obtain the option description
 *  corresponding to its @strong{UPPER CASED} option name argument.
 *  This is primarily used in the following macro definitions:
=*/
=]
#define     [=prefix _up #_ +=]DESC(n)     [=prog_name
                 =]Options.pOptDesc[INDEX_[=
                 prefix _up #_ +=]OPT_ ## n][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac HAVE_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Have this option?
 *
 *  description:
 *
 *  "This macro yields true if the option has been specified
 *  in any fashion at all.  It is used thus:
 *
 *  @example
 *  if (HAVE_OPT( OPT_NAME )) @{\n"
 *  "    <do-things-associated-with-opt-name>;
 *  @}
 *  @end example"
=*/
=]
#define     HAVE_[=prefix _up #_ +=]OPT(n) (! UNUSED_OPT(&[=prefix _up #_ +
                 =]DESC(n)))[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac OPT_ARG
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Option Argument String
 *
 *  description:
 *
 *  "The option argument value as a pointer to string.
 *  Note that argument values that have been specified as numbers
 *  are stored as numbers.  For such options, use instead the
 *  @code{OPT_VALUE_optname} define.  It is used thus:
 *
 *  @example
 *  if (HAVE_OPT( OPT_NAME )) @{\n"
 *  "    char* p = OPT_ARG( OPT_NAME );\n"
 *  "    <do-things-with-opt-name-argument-string>;
 *  @}
 *  @end example"
=*/
=]
#define      [=prefix _up #_ +=]OPT_ARG(n) ([=prefix _up #_ +
                 =]DESC(n).pzLastArg)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac STATE_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Option State
 *
 *  description:
 *
 *  "If you need to know if an option was set because of
 *  presetting actions (RC/INI processing or environment variables),
 *  versus a command line entry versus one of the SET/DISABLE macros,
 *  then use this macro.  It will contain one of four values:
 *  @code{OPTST_INIT}, @code{OPTST_SET}, @code{OPTST_PRESET}
 *  or @code{OPTST_DEFINED}.  It is used thus:
 *
 *  @example
 *  switch (STATE_OPT( OPT_NAME )) @{\n"
 *  "    case OPTST_INIT:\n"
 *  "        not-preset, set or on the command line.  (unless CLEAR-ed)\n\n"
 *
 *  "    case OPTST_SET:\n"
 *  "        option set via the SET_OPT_OPT_NAME() macro.\n\n"
 *
 *  "    case OPTST_PRESET:\n"
 *  "        option set via an RC/INI file or environment variable\n\n"
 *
 *  "    case OPTST_DEFINED:\n"
 *  "        option set via a command line option.\n\n"
 *
 *  "    default:\n"
 *  "        cannot happen :)
 *  @}
 *  @end example"
=*/
=]
#define    STATE_[=prefix _up #_ +=]OPT(n) ([=prefix _up #_ +
                 =]DESC(n).fOptState & OPTST_SET_MASK)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac COUNT_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Definition Count
 *
 *  description:
 *
 *  "This macro will tell you how many times the option was
 *  specified on the command line.  It does not include counts
 *  of preset options.
 *
 *  @example
 *  if (COUNT_OPT( OPT_NAME ) != desired-count) @{\n"
 *  "    make-an-undesireable-message.
 *  @}
 *  @end example"
=*/
=]
#define    COUNT_[=prefix _up #_ +=]OPT(n) ([=prefix _up #_ +
                 =]DESC(n).optOccCt)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac ISSEL_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Is Option Selected?
 *
 *  description:
 *
 *  This macro yields true if the option has been
 *  specified either on the command line or via a SET/DISABLE macro.
=*/
=]
#define    ISSEL_[=prefix _up #_ +=]OPT(n) (SELECTED_OPT(&[=prefix _up #_ +
                 =]DESC(n)))[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac ISUNUSED_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Never Specified?
 *
 *  description:
 *
 *  This macro yields true if the option has
 *  never been specified, or has been cleared via the
 *  @code{CLEAR_OPT()} macro.
=*/
=]
#define ISUNUSED_[=prefix _up #_ +=]OPT(n) (UNUSED_OPT(& [=prefix _up #_ +
                 =]DESC(n)))[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac ENABLED
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Is Option Enabled?
 *
 *  description:
 *
 *  Yields true if the option defaults to disabled and
 *  @code{ISUNUSED_OPT()} would yield true.  It also yields true if
 *  the option has been specified with a disablement prefix,
 *  disablement value or the @code{DISABLE_OPT_OPT_NAME} macro was invoked.
=*/
=]
#define  ENABLED_[=prefix _up #_ +=]OPT(n) (! DISABLED_OPT(& [=prefix _up #_ +
                 =]DESC(n)))[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac STACKCT_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Stacked Arg Count
 *
 *  description:
 *
 *  "When the option handling attribute is specified
 *  as @code{stack_arg}, this macro may be used to determine how
 *  many of them actually got stacked.
 *
 *  Do not use this on options that have not been stacked or has not been
 *  specified (the @code{stack_arg} attribute must have been specified,
 *  and @code{HAVE_OPT(<OPTION>)} must yield TRUE).
 *  Otherwise, you will likely page fault.
 *
 *  @example
 *  if (HAVE_OPT( OPT_NAME )) @{\n"
 *  "    int     ct = STACKCT_OPT(  OPT_NAME );\n"
 *  "    char**  pp = STACKLST_OPT( OPT_NAME );\n\n"
 *
 *  "    do  @{\n"
 *  "        char* p = *pp++;\n"
 *  "        do-things-with-p;\n"
 *  "    @} while (--ct > 0);
 *  @}
 *  @end example"
=*/
=]
#define  STACKCT_[=prefix _up #_ +=]OPT(n) (((tArgList*)([=prefix _up #_ +
                         =]DESC(n).optCookie))->useCt)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac STACKLST_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Argument Stack
 *
 *  description:
 *
 *  "The address of the list of pointers to the
 *  option arguments.  The pointers are ordered by the order in
 *  which they were encountered in the option presets and
 *  command line processing.
 *
 *  Do not use this on options that have not been stacked or has not been
 *  specified (the @code{stack_arg} attribute must have been specified,
 *  and @code{HAVE_OPT(<OPTION>)} must yield TRUE).
 *  Otherwise, you will likely page fault.
 *
 *  @example
 *  if (HAVE_OPT( OPT_NAME )) @{\n"
 *  "    int     ct = STACKCT_OPT(  OPT_NAME );\n"
 *  "    char**  pp = STACKLST_OPT( OPT_NAME );\n\n"
 *
 *  "    do  @{\n"
 *  "        char* p = *pp++;\n"
 *  "        do-things-with-p;\n"
 *  "    @} while (--ct > 0);
 *  @}
 *  @end example"
=*/
=]
#define STACKLST_[=prefix _up #_ +=]OPT(n) (((tArgList*)([=prefix _up #_ +
                         =]DESC(n).optCookie))->apzArgs)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac CLEAR_OPT
 *
 *  macro_arg:  <OPT_NAME>
 *
 *  title:  Clear Option Markings
 *
 *  description:
 *
 *  Make as if the option had never been specified.
 *  @code{HAVE_OPT(<OPTION>)} will yield @code{FALSE}
 *  after invoking this macro.
=*/
=]
#define    CLEAR_[=prefix _up #_ +=]OPT(n) STMTS( \
                [=prefix _up #_ +=]DESC(n).fOptState &= OPTST_PERSISTENT;   \
                if ( ([=prefix _up #_ +
                    =]DESC(n).fOptState & OPTST_INITENABLED) == 0) \
                    [=prefix _up #_ +=]DESC(n).fOptState |= OPTST_DISABLED; \
                [=prefix _up #_ +=]DESC(n).optCookie = (void*)NULL )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

Option specific definitions

=]

/*
 *  Interface defines for specific options.
 */[=

_FOR flag =][=
 _IF documentation _exist =][=
   _IF call_proc _exist flag_code _exist | =]
#define SET_[=prefix _up #_ +=]OPT_[=name _up=]   STMTS( \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                           prog_name=]Options, \
                [=prog_name=]Options.pOptDesc + [=_eval _index=] )[=
   _ENDIF "callout procedure exists" =][=

 _ELSE "not a documentation option" =][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac VALUE_OPT_optname
 *
 *  title:  Option Flag Value
 *
 *  description:
 *
 *  "This is a #define for the flag character used to
 *  specify an option on the command line.  If @code{value} was not
 *  specified for the option, then it is a unique number associated
 *  with the option.  @code{option value} refers to this value,
 *  @code{option argument} refers to the (optional) argument to the
 *  option.
 *
 *  @example
 *  switch (WHICH_OPT_OTHER_OPT) @{
 *  case VALUE_OPT_OPT_NAME:\n"
 *  "    this-option-was-really-opt-name;
 *  case VALUE_OPT_OTHER_OPT:\n"
 *  "    this-option-was-really-other-opt;
 *  @}
 *  @end example"
=*/
=]
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
        _ENDIF=][=#


* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac OPT_VALUE_optname
 *
 *  title:  Option Argument Value
 *
 *  description:
 *
 *  This macro gets emitted only for options that
 *  take numeric arguments.  The macro yields a word-sized integer
 *  containing the numeric value of the option argument.
 *
 *  @example
 *  int opt_val = OPT_VALUE_OPT_NAME;
 *  @end example
=*/
=][=
  _IF flag_arg _get #=.* ~
=]
#define [=prefix _up #_ +=]OPT_VALUE_[=name _up "#%-14s" _printf
                =] ((int)([=prefix _up #_ +=]OPT_ARG([=name _up=])))[=
  _ENDIF=][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac WHICH_OPT_optname
 *
 *  title:  Which Equivalenced Option
 *
 *  description:
 *
 *  "This macro gets emitted only for equivalenced-to
 *  options.  It is used to distinguish which of the several
 *  equivalence class members set the equivalenced-to option.
 *
 *  @example
 *  switch (WHICH_OPT_OTHER_OPT) @{
 *  case VALUE_OPT_OPT_NAME:\n"
 *  "    this-option-was-really-opt-name;
 *  case VALUE_OPT_OTHER_OPT:\n"
 *  "    this-option-was-really-other-opt;
 *  @}
 *  @end example"
=*/
=][=
  _IF equivalence _get _UP name _get _UP = =]
#define WHICH_[=prefix _up #_ +=]OPT_[=name _up "#%-14s" _printf
                =] ([=prefix _up #_ +=]DESC([=name _up=]).optActualValue)
#define WHICH_[=prefix _up #_ +=]IDX_[=name _up "#%-14s" _printf
                =] ([=prefix _up #_ +=]DESC([=name _up=]).optActualIndex)[=
  _ENDIF=][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac SET_OPT_optname
 *
 *  title:  Force an option to be set
 *
 *  description:
 *
 *  This macro gets emitted only when the given
 *  option has the @code{setable} attribute specified.
 *
 *  The form of the macro will actually depend on whether the
 *  option is equivalenced to another, has an option argument
 *  and/or has an assigned handler procedure.  If the option has
 *  an argument, then this macro will too.
 *
 *  @example
 *  SET_OPT_OPT_NAME( "string-value" );
 *  @end example
=*/
=][=
  _IF setable _exist =][=
    _IF  equivalence _exist !
         equivalence _get _UP name _get _UP == |


 =]
#define SET_[=prefix _up #_ +=]OPT_[=name _up=][=
      _IF flag_arg _exist=](a)[=_ENDIF=]   STMTS( \
        [=prefix _up #_ +=]DESC([=name _up
                       =]).optActualIndex = [=_eval _index=]; \
        [=prefix _up #_ +=]DESC([=name _up
                       =]).optActualValue = VALUE_[=prefix _up #_ +
                       =]OPT_[=name _up=]; \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState |= OPTST_SET[=
      _IF flag_arg _exist=]; \
        [=prefix _up #_ +=]DESC([=name _up=]).pzLastArg  = [=
        _IF flag_arg _get #=.* ~=](char*)atoi[=_ENDIF=](a)[=
      _ENDIF flag_arg-exists =][=
      _IF call_proc _exist
          flag_code _exist |
          flag_proc _exist |
          stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                  prog_name=]Options, \
                [=prog_name=]Options.pOptDesc + [=_eval _index=] )[=
      _ENDIF "callout procedure exists" =] )[=


    _ELSE "not equivalenced"
=]
#define SET_[=prefix _up #_ +=]OPT_[=name _up=][=
      _IF flag_arg _exist=](a)[=_ENDIF=]   STMTS( \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).optActualIndex = [=_eval _index=]; \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).optActualValue = VALUE_[=prefix _up #_ +
                       =]OPT_[=name _up=]; \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=equivalence _up
                       =]).fOptState |= OPTST_SET | OPTST_EQUIVALENCE[=
      _IF flag_arg _exist=]; \
        [=prefix _up #_ +=]DESC([=equivalence _up=]).pzLastArg  = [=
        _IF flag_arg _get #=.* ~=](char*)atoi[=_ENDIF=](a)[=
      _ENDIF flag_arg-exists =][=
      _IF call_proc _exist
          flag_code _exist |
          flag_proc _exist |
          stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                           prog_name=]Options, \
                [=prog_name=]Options.pOptDesc + INDEX_[=
                         prefix _up #_ +=]OPT_[=equivalence _up=] )[=
      _ENDIF "callout procedure exists" =] )[=

    _ENDIF is/not equivalenced =][=

  _ENDIF setable =][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac DISABLE_OPT_optname
 *
 *  title:  Disable an option
 *
 *  description:
 *
 *  This macro is emitted if it is both setable
 *  and it can be disabled.  If it cannot be disabled, it may
 *  always be CLEAR-ed (see above).
 *
 *  The form of the macro will actually depend on whether the
 *  option is equivalenced to another, and/or has an assigned
 *  handler procedure.  Unlike the @code{SET_OPT} macro,
 *  this macro does not allow an option argument.
 *
 *  @example
 *  DISABLE_OPT_OPT_NAME;
 *  @end example
=*/
=][=
  _IF setable _exist disable _exist & =]
#define DISABLE_[=prefix _up #_ +=]OPT_[=name _up=]   STMTS( \
        [=prefix _up #_ +=]DESC([=name _up=]).fOptState &= OPTST_PERSISTENT; \
        [=prefix _up #_ +=]DESC([=name _up
            =]).fOptState |= OPTST_SET | OPTST_DISABLED; \
        [=prefix _up #_ +=]DESC([=name _up=]).pzLastArg  = (char*)NULL[=
    _IF call_proc _exist
        flag_code _exist |
        flag_proc _exist |
        stack_arg _exist |=]; \
        (*([=prefix _up #_ +=]DESC([=name _up=]).pOptProc))( &[=
                  prog_name=]Options, \
                [=prog_name=]Options.pOptDesc + [=_eval _index=] )[=
     _ENDIF "callout procedure exists" =] )[=

  _ENDIF setable/disableable-exists =][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

End of option-specific defines

 =][=

 _ENDIF documentation =][=
/flag=][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *

Autoopts maintained option values.

If *any* option flag value is specified,
then we provide flag characters for our options.
Otherwise, we will use the INDEX_* values for the option value.

There are no documentation strings because these defines
are used identically to the user-generated VALUE defines.

:=]
[=
_IF flag.value _exist =][=
  _IF version _exist =]
#define VALUE_[=prefix _up #_ +=]OPT_VERSION        'v'[=
  _ENDIF =][=

  _IF homerc _exist=]
#define VALUE_[=prefix _up #_ +=]OPT_SAVE_OPTS      '>'
#define VALUE_[=prefix _up #_ +=]OPT_LOAD_OPTS      '<'[=
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
                                      prefix _up #_ +=]OPT_SAVE_OPTS
#define VALUE_[=prefix _up #_ +=]OPT_SAVE_OPTS      INDEX_[=
                                      prefix _up #_ +=]OPT_LOAD_OPTS[=
  _ENDIF=]
#define VALUE_[=prefix _up #_ +=]OPT_HELP           INDEX_[=
                                      prefix _up #_ +=]OPT_HELP
#define VALUE_[=prefix _up #_ +=]OPT_MORE_HELP      INDEX_[=
                                      prefix _up #_ +=]OPT_MORE_HELP[=
_ENDIF=][=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac ERRSKIP_OPTERR
 *
 *  title:  Ignore Option Errors
 *
 *  description:
 *
 *  When it is necessary to continue (return to caller)
 *  on option errors, invoke this option.  It is reversable.
 *  @xref{ERRSTOP_OPTERR}.
=*/
=]

/*
 *  Interface defines not associated with particular options
 */
#define  ERRSKIP_[=prefix _up #_ +=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet &= ~OPTPROC_ERRSTOP )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac ERRSTOP_OPTERR
 *
 *  title:  Stop on Errors
 *
 *  description:
 *
 *  After invoking this macro, if @code{optionProcess()}
 *  encounters an error, it will call @code{exit(1)} rather than return.
 *  This is the default processing mode.  It can be overridden by
 *  specifying @code{allow_errors} in the definitions file,
 *  or invoking the macro @xref{ERRSKIP_OPTERR}.
=*/
=]
#define  ERRSTOP_[=prefix _up #_ +=]OPTERR STMTS( [=prog_name
                         =]Options.fOptSet |= OPTPROC_ERRSTOP )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac RESTART_OPT
 *
 *  macro_arg:  n
 *
 *  title:  Resume Option Processing
 *
 *  description:
 *
 *  If option processing has stopped (either because of an error
 *  or something was encountered that looked like a program argument),
 *  it can be resumed by providing this macro with the index @code{n}
 *  of the next option to process and calling @code{optionProcess()} again.
=*/
=]
#define  RESTART_[=prefix _up #_ +=]OPT(n) STMTS( \
                [=prog_name=]Options.curOptIdx = (n); \
                [=prog_name=]Options.pzCurOpt  = (char*)NULL )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac START_OPT
 *
 *  title:  Restart Option Processing
 *
 *  description:
 *
 *  This is just a shortcut for RESTART_OPT(1) (@xref{RESTART_OPT}).
=*/
=]
#define    START_[=prefix _up #_ +=]OPT    RESTART_[=prefix _up #_ +
                =]OPT(1)[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/*=usermac USAGE
 *
 *  macro_arg:  exit-code
 *
 *  title:  Usage invocation macro
 *
 *  description:
 *
 *  This macro invokes the procedure registered to display
 *  the usage text.  Normally, this will be @code{optionUsage} from the
 *  Autoopts library, but you may select another procedure by specifying
 *  @code{usage = "proc_name"} program attribute.  This procedure must
 *  take two arguments: first, a pointer to the option descriptor, and
 *  second the exit code.  The macro supplies the option descriptor
 *  automatically.  This routine is expected to call @code{exit()} with
 *  the provided exit code.
 *
 *  The @code{optionUsage} routine also behaves differently depending
 *  on the exit code.  If the exit code is zero, it is assumed that
 *  assistance has been requested.  Consequently, a little more
 *  information is provided than when displaying usage and exiting
 *  with a non-zero exit code.
=*/
=]
#define     [=prefix _up #_ +=]USAGE(c)    (*[=prog_name
                 =]Options.pUsageProc)( &[=prog_name=]Options, c )[=#

* * * * * * * * * * * * * * * * * * * * * * * * * * * *
=]

/* * * * * *
 *
 *  Declare the [=prog_name=] option descriptor.
 */
#ifdef  __cplusplus
extern "C" {
#endif

extern tOptions   [=prog_name=]Options;[=

_IF export _exist=]

/* * * * * *
 *
 *  Globals exported from the [=prog_title=] option definitions
 */[=
  _FOR export "\n"=]
[=export =][=
  /export=][=
_ENDIF=]

#ifdef  __cplusplus
}
#endif
#endif /* [=_eval DEFNAME _env=] */
