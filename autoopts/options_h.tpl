
/*
 *  options.h  $Id: options_h.tpl,v 1.3 1998/07/02 23:00:18 bkorb Exp $
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 */

/*
 *  Automated Options copyright 1992-1998 Bruce Korb
 *
 *  Automated Options is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Automated Options is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Automated Options.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *
 * As a special exception, Bruce Korb gives permission for additional
 * uses of the text contained in his release of AutoOpts.
 *
 * The exception is that, if you link the AutoOpts library with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the AutoOpts library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Bruce Korb under
 * the name AutoOpts.  If you copy code from other sources under the
 * General Public License into a copy of AutoOpts, as the General Public
 * License permits, the exception does not apply to the code that you add
 * in this way.  To avoid misleading anyone as to the status of such
 * modified files, you must delete this exception notice from them.
 *
 * If you write modifications of your own for AutoOpts, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 */

#ifndef AUTOGEN_OPTIONS_H
#define AUTOGEN_OPTIONS_H

/*
 *  PUBLIC DEFINES
 *
 *  The following defines may be used in applications that need to test
 *  the state of an option.  To test against these masks and values,
 *  a pointer to an option descriptor must be obtained.  There are two
 *  ways:  1.  inside an option processing procedure, it is the second
 *  argument, conventionally "tOptDesc* pOD".  2.  Outside of an option
 *  procedure (or to reference a different option descriptor), use
 *  either "&DESC( opt_name )" or "&pfx_DESC( opt_name )".  See the
 *  relevent generated header file to determine which and what values
 *  for "opt_name" are available.
 */

#define OPTST_INIT         0x0000
#define OPTST_SET          0x0001
#define OPTST_PRESET       0x0002
#define OPTST_DEFINED      0x0004
#define OPTST_SET_MASK     0x000F  /* mask of flags that show set state */
#define OPTST_EQUIVALENCE  0x0010  /* selected by equiv'ed option       */
#define OPTST_DISABLED     0x0020  /* option with disable marker        */
#define OPTST_NO_INIT      0x0100  /* option cannot be preset           */
#define OPTST_NUMBER_OPT   0x0200  /* option is number option           */
#define OPTST_NUMERIC      0x0400  /* option has numeric value          */
#define OPTST_STACKED      0x0800  /* opt uses stackOptArg procedure    */
#define OPTST_NEGATABLE    0x1000  /* option may be negated with "NO"   */
#define OPTST_PERSISTENT   0xFF00  /* mask of flags that do not change  */

#define SELECTED_OPT( pod )  ( (pod)->fOptState & (OPTST_SET | OPTST_DEFINED))
#define UNUSED_OPT(   pod )  (((pod)->fOptState & OPTST_SET_MASK) == 0)
#define DISABLED_OPT( pod )  ( (pod)->fOptState & OPTST_DISABLED)
#define OPTION_STATE( pod )  ((pod)->fOptState)

/*
 *  PRIVATE INTERFACES
 *
 *  The following values are used in the generated code to communicate
 *  with the option library procedures.  They are not for public use
 *  and may be subject to change.
 */

/*
 *  Define any special processing flags
 */
#define OPTPROC_NONE        0x0000
#define OPTPROC_LONGOPT     0x0001 /* Process long style options           */
#define OPTPROC_SHORTOPT    0x0002 /* Process short style "flags"          */
#define OPTPROC_EXERC       0x0004 /* Preload options from exe's directory */
#define OPTPROC_ERRSTOP     0x0008 /* Stop on argument errors              */
#define OPTPROC_DISABLEOK   0x0010 /* Disabling options are allowed        */
#define OPTPROC_DISABLEDOPT 0x0020 /* Current option is disabled           */
#define OPTPROC_NO_REQ_OPT  0x0040 /* no options are required              */
#define OPTPROC_NUM_OPT     0x0080 /* there is a number option             */
#define OPTPROC_INITDONE    0x0100 /* have initializations been done?      */
#define OPTPROC_NEGATIONS   0x0200 /* any negation options?                */
#define OPTPROC_ENVIRON     0x0400 /* check environment?                   */

#define STMTS(s)  do { s; } while (0)

#define ARG_NONE  ' '
#define ARG_MUST  ':'
#define ARG_MAY   '?'

/*
 *  The following must be #defined instead of typedef-ed
 *  because "static const" cannot both be applied to a type,
 *  tho each individually can...so they all are
 */
#define tSCC      static const char
#define tSC       static char
#define tCUC      const unsigned char
#define tUC       unsigned char
#define tCC       const char
#define tUS       unsigned short
#define tUI       unsigned int

/*
 *  It is so disgusting that there must be so many ways
 *  of specifying TRUE and FALSE.
 */
typedef enum { AG_FALSE = 0, AG_TRUE } ag_bool;

/*
 *  Define a structure that describes each option and
 *  a pointer to the procedure that handles it.
 *  The argument is the count of this flag previously seen.
 */
typedef struct options  tOptions;
typedef struct optDesc  tOptDesc;
typedef struct optNames tOptNames;

/*
 *  The option procedures do the special processing for each
 *  option flag that needs it.
 */
typedef void (tOptProc)( tOptions*  pOpts, tOptDesc* pOptDesc );
typedef tOptProc*  tpOptProc;

/*
 *  The usage procedure will never return.  It calls "exit(2)"
 *  with the "exitCode" argument passed to it.
 */
typedef void (tUsageProc)( tOptions* pOpts, int exitCode );
typedef tUsageProc* tpUsageProc;

/*
 *  Special definitions.  "NOLIMIT" is the 'max' value to use when
 *  a flag may appear multiple times without limit.  "NO_EQUIVALENT"
 *  is an illegal value for 'optIndex' (option description index).
 */
#define NOLIMIT          ((tUC)~0)
#define OPTION_LIMIT     0x7F
#define NO_EQUIVALENT    (OPTION_LIMIT+1)

/*
 *  Special values for optValue.  It must not be generatable from the
 *  computation "optIndex +96".  Since "optIndex" is limited to 100, ...
 */
#define NUMBER_OPTION    '#'

typedef struct argList tArgList;
#define MIN_ARG_ALLOC_CT   6
#define INCR_ARG_ALLOC_CT  8
struct argList {
    int               useCt;
    int               allocCt;
    char*             apzArgs[ MIN_ARG_ALLOC_CT ];
};

struct optDesc {
    tCUC              optIndex;
    tCUC              optValue;
    tUC               optActualIndex;
    tUC               optActualValue;

    tUC               optArgType;
    tCUC              optEquivIndex;
    tCUC              optMinCt;
    tCUC              optMaxCt;

    tUC               optOccCt;
    tUC               optFill;
    tUS               fOptState;

    char*             pzLastArg;
    void*             optCookie;
    const int *       pOptMust;
    const int *       pOptCant;
    tpOptProc         pOptProc;

    const char*       pzText;
    const char*       pz_NAME;
    const char*       pz_Name;
    const char*       pz_DisableName;
};

typedef struct specOptIndex tSpecOptIndex;
struct specOptIndex {
    tUC               more_help;
    tUC               save_opts;
    tUC               number_option;
    tUC               filler;
};

/*
 *  Be sure to change this value every time a change is made
 *  that changes the interface.  This way, the "optionProcess()"
 *  routine may exit with an informative message instead of,
 *  for example, page faulting.
 */
#define OPTIONS_STRUCT_VERSION 2

struct options {
    const int         structVersion;
    const char*       pzProgPath;
    const char*       pzProgName;
    const char*       pzPROGNAME;
    const char*       pzRcName;
    const char*       pzCopyright;
    const char*       pzCopyNotice;
    const char*       pzFullVersion;
    const char**      papzHomeList;
    const char*       pzUsageTitle;
    const char*       pzExplain;
    const char*       pzDetail;
    const char*       pzDetailFile;
    tpUsageProc       pUsageProc;
    tUI               fOptSet;
    tUI               curOptIdx;
    char*             pzCurOpt;
    tSpecOptIndex     specOptIdx;
    const int         optCt;
    const int         presetOptCt;
    tOptDesc*         pOptDesc;
};

#ifdef  __cplusplus
extern "C" {
#endif
/*
 *  optionProcess scans the argument flags completely.
 *  The return value is the argument index of the first argument
 *  following the flag list, per "getopt(3)".
 */
int     optionProcess( tOptions* pOpts, int argCt, char** argVect );

/*
 *  optionSave saves the option state into an RC or INI file in
 *  the *LAST* defined directory in the papzHomeList.
 */
void    optionSave(   tOptions* pOpts );

/*
 *  optionLoadLine will load an option in the text buffer as
 *  a normal (not preset) option.
 */
void    optionLoadLine( tOptions* pOpts, char*  pzLine );

/*
 *  putBourneShell outputs the option state to standard out in a way
 *  that the Bourne and Korn shells can interpret.
 */
void    putBourneShell( tOptions* pOpts );

/*
 *  stackOptArg saves the option argument into an option-specific list.
 *  It will allocate the list the first time and extend it as needed.
 */
tOptProc stackOptArg, unstackOptArg;

#ifdef  __cplusplus
}
#endif
#endif /* AUTOGEN_OPTIONS_H */
