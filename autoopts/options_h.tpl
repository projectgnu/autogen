
/*
 *  options.h  $Id: options_h.tpl,v 2.21 2000/10/28 18:17:32 bkorb Exp $
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 */

/*
 *  Automated Options copyright 1992-1999 Bruce Korb
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
 * uses of the text contained in his release of options.h.
 *
 * The exception is that, if you reference this file with a #include
 * statement when compiling a C or C++ source file, this does not
 * cause the resulting object file or linked executable to be covered
 * by the GNU General Public License.
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
 *  relevant generated header file to determine which and what values
 *  for "opt_name" are available.
 */

#define OPTST_INIT         0x00000  /* Initial compiled value            */
#define OPTST_SET          0x00001  /* Set via the "SET_OPT()" macro     */
#define OPTST_PRESET       0x00002  /* Set via an RC/INI file            */
#define OPTST_DEFINED      0x00004  /* Set via a command line option     */
#define OPTST_SET_MASK     0x0000F  /* mask of flags that show set state */

#define OPTST_EQUIVALENCE  0x00010  /* selected by equiv'ed option       */
#define OPTST_DISABLED     0x00020  /* option is in disabled state       */

#define OPTST_NO_INIT      0x00100  /* option cannot be preset           */
#define OPTST_NUMBER_OPT   0x00200  /* opt value (flag) is any digit     */
#define OPTST_STACKED      0x00400  /* opt uses stackOptArg procedure    */
#define OPTST_INITENABLED  0x00800  /* option defaults to enabled        */
#define OPTST_ENUMERATION  0x01000  /* opt arg is an enum (keyword list) */
#define OPTST_BOOLEAN      0x02000  /* opt arg is boolean-valued         */
#define OPTST_NUMERIC      0x04000  /* opt arg has numeric value         */
#define OPTST_DOCUMENT     0x08000  /* opt is for documentation only     */
#define OPTST_IMM          0x10000  /* process option on first pass      */
#define OPTST_DISABLE_IMM  0x20000  /* process disablement on first pass */

#define OPTST_PERSISTENT   0xFFF00  /* mask of flags that do not change  */

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
#define OPTPROC_LONGOPT     0x0001 /* Process long style options      */
#define OPTPROC_SHORTOPT    0x0002 /* Process short style "flags"     */
#define OPTPROC_ERRSTOP     0x0004 /* Stop on argument errors         */
#define OPTPROC_DISABLEDOPT 0x0008 /* Current option is disabled      */
#define OPTPROC_NO_REQ_OPT  0x0010 /* no options are required         */
#define OPTPROC_NUM_OPT     0x0020 /* there is a number option        */
#define OPTPROC_INITDONE    0x0040 /* have initializations been done? */
#define OPTPROC_NEGATIONS   0x0080 /* any negation options?           */
#define OPTPROC_ENVIRON     0x0100 /* check environment?              */
#define OPTPROC_NO_ARGS     0x0200 /* Disallow remaining arguments    */
#define OPTPROC_ARGS_REQ    0x0400 /* Require arguments after options */

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
#define tUI       unsigned int
#define tUL       unsigned long

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

/*
 *  Descriptor structure for each option.
 *  Only the fields marked "PUBLIC" are for public use.
 */
struct optDesc {
    tCUC              optIndex;         /* PUBLIC */
    tCUC              optValue;         /* PUBLIC */
    tUC               optActualIndex;   /* PUBLIC */
    tUC               optActualValue;   /* PUBLIC */

    tUC               optArgType;
    tCUC              optEquivIndex;    /* PUBLIC */
    tCUC              optMinCt;
    tCUC              optMaxCt;

    tUL               optOccCt;         /* PUBLIC */
    tUL               fOptState;        /* PUBLIC */
    char*             pzLastArg;        /* PUBLIC */
    void*             optCookie;        /* PUBLIC */

    const int *       pOptMust;
    const int *       pOptCant;
    tpOptProc         pOptProc;
    const char*       pzText;

    const char*       pz_NAME;
    const char*       pz_Name;
    const char*       pz_DisableName;
    const char*       pz_DisablePfx;
};

/*
 *  Some options need special processing, so we store their
 *  indexes in a known place:
 */
typedef struct specOptIndex tSpecOptIndex;
struct specOptIndex {
    tUC               more_help;
    tUC               save_opts;
    tUC               number_option;
    tUC               default_opt;
};

/*
 *  Be sure to change this value every time a change is made that
 *  changes the interface structure or the meanings of any of its
 *  fields.  This way, the "optionProcess()" routine may exit with an
 *  informative message instead of, for example, page faulting.
 */
#define  OPTIONS_STRUCT_VERSION  32896

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
    int               origArgCt;
    char**            origArgVect;
    void*             futureUse;
};

#ifdef  __cplusplus
extern "C" {
#endif

#if defined( __STDC__ ) || defined( __cplusplus )
#  ifndef PROTO
#    define PROTO(s) s
#  endif

/*
 *  All this "DEF_PROC_*" weirdness is so that I can compile the option
 *  processing code with a K&R compiler.  Yuck.  Don't use them unless
 *  you really have to.  (Still better than ugly if-defs, tho.)
 */
#define DEF_PROC_0( id ) \
          id( void ); \
          id( void )

#define DEF_PROC_1( id, a1_type, a1_name ) \
          id( a1_type a1_name ); \
          id( a1_type a1_name )

#define DEF_PROC_2( id, a1t, a1n, a2t, a2n ) \
          id( a1t a1n, a2t a2n ); \
          id( a1t a1n, a2t a2n )

#define DEF_PROC_3( id, a1t, a1n, a2t, a2n, a3t, a3n ) \
          id( a1t a1n, a2t a2n, a3t a3n ); \
          id( a1t a1n, a2t a2n, a3t a3n )

#define DEF_PROC_4( id, a1t, a1n, a2t, a2n, a3t, a3n, a4t, a4n ) \
          id( a1t a1n, a2t a2n, a3t a3n, a4t a4n ); \
          id( a1t a1n, a2t a2n, a3t a3n, a4t a4n )

#else
#  ifndef PROTO
#    define PROTO(s) ()
#  endif

#define DEF_PROC_0( id ) \
          id()

#define DEF_PROC_1( id, a1_type, a1_name ) \
          id( a1_name ) a1_type a1_name;

#define DEF_PROC_2( id, a1t, a1n, a2t, a2n ) \
          id( a1n, a2n ) a1t a1n; a2t a2n;

#define DEF_PROC_3( id, a1t, a1n, a2t, a2n, a3t, a3n ) \
          id( a1n, a2n, a3n ) \
          a1t a1n; a2t a2n; a3t a3n;

#define DEF_PROC_4( id, a1t, a1n, a2t, a2n, a3t, a3n, a4t, a4n ) \
          id( a1n, a2n, a3n ) \
          a1t a1n; a2t a2n; a3t a3n; a4t a4n;

#  ifndef void
#    define void int
#  endif

#  ifndef const
#    define const
#  endif
#endif

/*
 * Return the full version string compiled into the library.
 */
const char	*optionVersion PROTO(( void ));

/*
 *  optionProcess scans the argument flags completely.
 *  The return value is the argument index of the first argument
 *  following the flag list, per "getopt(3)".
 */
int     optionProcess PROTO(( tOptions* pOpts, int argCt, char** argVect ));

/*
 *  optionSave saves the option state into an RC or INI file in
 *  the *LAST* defined directory in the papzHomeList.
 */
void    optionSave PROTO((   tOptions* pOpts ));

/*
 *  optionLoadLine will load an option in the text buffer as
 *  a normal (not preset) option.
 */
void    optionLoadLine PROTO(( tOptions* pOpts, char*  pzLine ));

/*
 *  putBourneShell outputs the option state to standard out in a way
 *  that the Bourne and Korn shells can interpret.
 */
void    putBourneShell PROTO(( tOptions* pOpts ));

/*
 *  export_options_to_guile will emit a series of guile commands so
 *  that the Scheme code can query the state of any of the defined options.
 */
void    export_options_to_guile PROTO(( tOptions* ));

/*
 *  doEnumerationOption will find a string in a table of strings and
 *  return the index cast as a char* value.  USAGE will be invoked if
 *  the string is not found in the table.
 */
char*   optionEnumerationVal PROTO(( tOptions*, tOptDesc*, tCC**, int ));

/*
 *  stackOptArg saves the option argument into an option-specific list.
 *  It will allocate the list the first time and extend it as needed.
 */
tOptProc stackOptArg, unstackOptArg, optionBooleanVal, optionNumericVal;

#ifdef  __cplusplus
}
#endif
#endif /* AUTOGEN_OPTIONS_H */
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * options.h ends here */
