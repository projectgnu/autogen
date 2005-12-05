
/*
 *  autogen.h
 *  $Id: autogen.h,v 4.11 2005/12/05 20:46:43 bkorb Exp $
 *  Global header file for AutoGen
 */

/*
 *  AutoGen copyright 1992-2005 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */
#ifndef AUTOGEN_HDR_H
#define AUTOGEN_HDR_H

#include "config.h"
#include "compat/compat.h"

#include REGEX_HEADER
#include <guile/gh.h>

#include "opts.h"
#include "directive.h"
#include "snprintfv/printf.h"

#ifndef STR
#  define _STR(s) #s
#  define STR(s)  _STR(s)
#endif

#define ISNAMECHAR( c ) (isalnum(c) || ((c) == '_') || ((c) == '-'))

#define STRSIZE( s )  (sizeof(s)-1)

#ifdef DEFINING
#  define VALUE( s )  = s
#  define MODE
#else
#  define VALUE( s )
#  define MODE extern
#endif

#define YYSTYPE t_word

#define ag_offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

/*
 *  Dual pipe opening of a child process
 */
typedef struct {
    int     readFd;
    int     writeFd;
}  tFdPair;

typedef struct {
    FILE*   pfRead;  /* parent read fp  */
    FILE*   pfWrite; /* parent write fp */
}  tpfPair;

#define NOPROCESS   ((pid_t)-1)
#define NULLPROCESS ((pid_t)0)

typedef unsigned char* tpChar;

#include "expr.h"
#include "autoopts/autoopts.h"
#include "cgi-fsm.h"
#include "defParse-fsm.h"

typedef union {
    tpChar      pzStr;
    char        ch;
} def_token_u_t;

#define STATE_TABLE           /* set up `atexit' and load Guile   */  \
    _State_( INIT )           /* processing command line options  */  \
    _State_( OPTIONS )        /* Loading guile at option time     */  \
    _State_( GUILE_PRELOAD )  /* Loading value definitions        */  \
    _State_( LOAD_DEFS )      /* Loading library template         */  \
    _State_( LIB_LOAD )       /* Loading primary template         */  \
    _State_( LOAD_TPL )       /* processing templates             */  \
    _State_( EMITTING )       /* loading an included template     */  \
    _State_( INCLUDING )      /* end of processing before exit()  */  \
    _State_( CLEANUP )        /* Clean up code in error response  */  \
    _State_( ABORTING )       /* `exit' has been called           */  \
    _State_( DONE )

#define _State_(n)  PROC_STATE_ ## n,
typedef enum { STATE_TABLE COUNT_PROC_STATE } teProcState;
#undef _State_

#define EXPORT

typedef struct fpStack       tFpStack;
typedef struct outSpec       tOutSpec;
typedef struct scanContext   tScanCtx;
typedef struct defEntry      tDefEntry;
typedef struct macro_desc    tMacro;
typedef struct template_desc tTemplate;
typedef struct for_info      tForInfo;
typedef struct for_state     tForState;
typedef struct template_lib_marker tTlibMark;

#define MAX_SUFFIX_LEN    8  /* maximum length of a file name suffix */
#define MAX_HEREMARK_LEN 64  /* max length of a here mark */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Template Library Layout
 *
 *  Procedure for loading a template function
 */
typedef tMacro* (tLoadProc)( tTemplate*, tMacro*, const char** ppzScan );
typedef tLoadProc* tpLoadProc;

typedef void (tUnloadProc)( tMacro* );
typedef tUnloadProc* tpUnloadProc;

/*
 *  Procedure for handling a template function
 *  during the text emission phase.
 */
typedef tMacro* (tHdlrProc)( tTemplate*, tMacro* );
typedef tHdlrProc* tpHdlrProc;

/*
 *  This must be included after the function prototypes
 *  (the prototypes are used in the generated tables),
 *  but before the macro descriptor structure (the function
 *  enumeration is generated here).
 */
#include "functions.h"

#define TEMPLATE_REVISION     1
#define TEMPLATE_MAGIC_MARKER {{{'A', 'G', 'L', 'B'}}, \
                               TEMPLATE_REVISION, FUNCTION_CKSUM }

struct template_lib_marker {
    union {
        char        str[4];  /* {'A', 'G', 'L', 'B'} */
        long        i[1];
    }           magic;
    short       revision;    /* TEMPLATE_REVISION    */
    short       funcSum;     /* FUNCTION_CKSUM       */
};

/*
 *  Defines for conditional expressions.
 *  The first four are an enumeration that appear in the
 *  low four bits and the next-to-lowest four bits.
 *  "PRIMARY_TYPE" and "SECONDARY_TYPE" are masks for
 *  extracting this enumeration.  The rest are flags.
 */
#define EMIT_VALUE          0x0000  /* emit value of variable  */
#define EMIT_EXPRESSION     0x0001  /* Emit Scheme result      */
#define EMIT_SHELL          0x0002  /* emit shell output       */
#define EMIT_STRING         0x0003  /* emit content of expr    */
#define EMIT_PRIMARY_TYPE   0x0007
#define EMIT_SECONDARY_TYPE 0x0070
#define EMIT_SECONDARY_SHIFT     4
#define EMIT_IF_ABSENT      0x0100
#define EMIT_ALWAYS         0x0200  /* emit one of two exprs   */
#define EMIT_FORMATTED      0x0400  /* format, if val present  */
#define EMIT_NO_DEFINE      0x0800  /* don't get defined value */

struct macro_desc {
    teFuncType  funcCode;  /* Macro function         */
    int         lineNo;    /* of macro def           */
    int         endIndex;  /* End of block macro     */
    int         sibIndex;  /* Sibling macro (ELIF or SELECT) */

    uintptr_t   ozName;    /* macro name (sometimes) */
    uintptr_t   ozText;    /* associated text        */
    long        res;       /* some sort of result    */
    void*       funcPrivate;
};

struct template_desc {
    tTlibMark   magic;       /* TEMPLATE_MAGIC_MARKER    */
    int         fd;          /* mmap file descriptor     */
    size_t      descSize;    /* Structure Size           */
    char*       pNext;       /* Next Pointer             */
    int         macroCt;     /* Count of Macros          */
    tCC*        pzFileName;  /* Name of Macro File       */
    char*       pzTplName;   /* Template Name Pointer    */
    char*       pzTemplText; /* offset of the text       */
    char        zStartMac[MAX_SUFFIX_LEN];
    char        zEndMac[MAX_SUFFIX_LEN];
    tMacro      aMacros[1];  /* Array of Macros          */
/*  char        text[...];    * strings at end of macros */
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Name/Value Definitions Layout
 */
typedef enum {
    VALTYP_UNKNOWN = 0,
    VALTYP_TEXT,
    VALTYP_BLOCK
} teValType;


#define NO_INDEX ((short)0x80DEAD)

typedef struct sDefCtx tDefCtx;
struct sDefCtx {
    tDefEntry* pDefs;           /* ptr to current def set     */
    tDefCtx*   pPrev;           /* ptr to previous def set    */
};

typedef union {
    tDefEntry*  pDefEntry;
    char*       pzText;
} uDefValue;

struct defEntry {
    tDefEntry* pNext;           /* next member of same level  */
    tDefEntry* pTwin;           /* next member with same name */
    tDefEntry* pPrevTwin;       /* previous memb. of level    */
    tDefEntry* pEndTwin;        /* head of chain to end ptr   */
    char*      pzDefName;       /* name of this member        */
    long       index;           /* index among twins          */
    uDefValue  val;             /* string or list of children */
    char*      pzSrcFile;       /* definition file name       */
    int        srcLineNum;      /* def file source line       */
    teValType  valType;         /* text/block/not defined yet */
};

struct scanContext {
    tScanCtx*   pCtx;
    char*       pzScan;
    tCC*        pzCtxFname;
    char*       pzData;
    int         lineNo;
};

struct outSpec {
    tOutSpec*   pNext;
    const char* pzFileFmt;
    char        zSuffix[ 1 ];
};

#define FPF_FREE       0x0001   /* free the fp structure   */
#define FPF_UNLINK     0x0002   /* unlink file (temp file) */
#define FPF_NOUNLINK   0x0004   /* do not unlink file      */
#define FPF_STATIC_NM  0x0008   /* name statically alloced */
#define FPF_NOCHMOD    0x0010   /* do not chmod(2) file    */

struct fpStack {
    int         flags;
    tFpStack*   pPrev;
    FILE*       pFile;
    tCC*        pzOutName;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  FOR loop processing state
 */
struct for_info {
    int          fi_depth;
    int          fi_alloc;
    tForState*   fi_data;
};

struct for_state {
    ag_bool      for_loading;
    int          for_from;
    int          for_to;
    int          for_by;
    int          for_index;
    char*        for_pzSep;
    char*        for_pzName;
    ag_bool      for_lastFor;
    ag_bool      for_firstFor;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Parsing stuff
 */
typedef struct {
    int     entryCt;
    int     allocCt;
    char*   entries[1];
} tList;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  GLOBAL VARIABLES
 *
 *  General Processing Globals
 */
#define pzProg   autogenOptions.pzProgName
MODE teProcState procState        VALUE( PROC_STATE_INIT );
MODE tTemplate*  pNamedTplList    VALUE( NULL );
MODE tCC*        pzOopsPrefix     VALUE( "" );

/*
 *  Template Processing Globals
 */
MODE tCC*        pzCurSfx         VALUE( NULL );
MODE time_t      outTime          VALUE( 0 );
MODE tFpStack*   pCurFp           VALUE( NULL );
MODE tOutSpec*   pOutSpecList     VALUE( NULL );
MODE jmp_buf     fileAbort;
MODE char*       pzCurStart       VALUE( NULL );
MODE uintptr_t   curStartOff      VALUE( 0 );
MODE tForInfo    forInfo          VALUE( { 0 } );
MODE FILE*       pfTrace          VALUE( NULL );
MODE char*       pzTmpStderr      VALUE( NULL );

MODE tCC*        serverArgs[2]    VALUE( { NULL } );
MODE tCC*        pzShellProgram   VALUE( NULL );

/*
 *  AutoGen definiton and template context
 *
 *  currDefCtx is the current, active list of name/value pairs.
 *  Points to its parent list for full search resolution.
 *
 *  pCurTemplate the template (and DEFINE macro) from which
 *  the current set of macros is being extracted.
 *
 *  These are set in exactly ONE place:
 *  On entry to the dispatch routine (generateBlock)
 *  Two routines, however, must restore the values:  mFunc_Define
 *  and mFunc_For.  They are the only routines that dynamically
 *  push name/value pairs on the definition stack.
 */
MODE tDefCtx     currDefCtx       VALUE( { NULL } );
MODE tDefCtx     rootDefCtx       VALUE( { NULL } );
MODE tTemplate*  pCurTemplate     VALUE( NULL );
MODE tCC*        pzLastScheme     VALUE( NULL );
#ifdef DAEMON_ENABLED
/*
 *  When operating as a daemon, autogen can be told to reload
 *  its options the next time it wakes up (send it a SIGHUP).
 */
MODE ag_bool     redoOptions      VALUE( AG_TRUE );
#endif

/*
 *  Current Macro
 *
 *  This may be set in exactly three places:
 *  1.  The dispatch routine (generateBlock) that steps through
 *      a list of macros
 *  2.  mFunc_If may transfer to one of its 'ELIF' or 'ELSE'
 *      alternation macros
 *  3.  mFunc_Case may transfer to one of its selection clauses.
 */
MODE tMacro*     pCurMacro        VALUE( NULL );

/*
 *  Template Parsing Globals
 */
MODE int         templLineNo      VALUE( 1 );
MODE tScanCtx*   pBaseCtx         VALUE( NULL );
MODE tScanCtx*   pCurCtx          VALUE( NULL );
MODE tScanCtx*   pDoneCtx         VALUE( NULL );
MODE int         endMacLen        VALUE( 0  );
MODE char        zEndMac[   8 ]   VALUE( "" );
MODE int         startMacLen      VALUE( 0  );
MODE char        zStartMac[  8 ]  VALUE( "" );
MODE int         guileFailure     VALUE( 0 );

/*
 *  Definition Parsing Globals
 */
MODE char*       pzDefineData     VALUE( NULL );
MODE size_t      defineDataSize   VALUE( 0 );
MODE char*       pz_token         VALUE( NULL );
MODE te_dp_event lastToken        VALUE( DP_EV_INVALID );

MODE int         stackDepth       VALUE( 0 );
MODE int         stackSize        VALUE( 16 );
MODE tDefEntry*  parseStack[16]   VALUE( { 0 } );
MODE tDefEntry** ppParseStack     VALUE( parseStack );
MODE tDefEntry*  pCurrentEntry    VALUE( NULL );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  GLOBAL STRINGS
 */
#define MKSTRING( name, val ) \
        MODE const char z ## name[ sizeof( val )] VALUE( val )

MKSTRING( AllocWhat,  "Could not allocate a %d byte %s\n" );
MKSTRING( AllocErr,   "Allocation Failure" );
MKSTRING( Cannot,     "fserr %d: cannot %s %s:  %s\n" );
MKSTRING( TplWarn,    "Warning in template %s, line %d\n\t%s\n" );
MKSTRING( FileLine,   "\tfrom %s line %d\n" );
MKSTRING( ShDone,     "ShElL-OuTpUt-HaS-bEeN-cOmPlEtEd" );
MKSTRING( NotStr,     "ERROR: %s is not a string\n" );
MKSTRING( DevNull,    "/dev/null" );
MKSTRING( ShellEnv,   "SHELL" );

/*
 *  It may seem odd that there are two 'nil' strings.  It is used by
 *  evalExpression() to distinguish between an actual value of a zero-
 *  length string, and a defaulted value because there was no value
 *  available.  (In fact, that routine will always select 'zDefaultNil'
 *  except when "runShell()" returns NULL.  A NULL result means an empty
 *  string that is valid (zNil) as opposed to an empty string due to
 *  an unknown value (zDefaultNil).  This distinction is utilized in
 *  the Select_Match_Existence() and Select_Match_NonExistence() functions.
 */
MKSTRING( Nil,        "" );
MKSTRING( DefaultNil, "" );

extern void unloadTemplate( tTemplate* pT );
extern void unloadDefs( void );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  GLOBAL PROCEDURES
 */
#ifndef HAVE_STRLCPY
extern size_t strlcpy( char* dest, const char* src, size_t n );
#endif
#ifdef ENABLE_FMEMOPEN
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  IF we have fopencookie or funopen, then we also have our own fmemopen...
 *
 *  Special ioctl/"seek" requests
 */
#define FMEM_IOCTL_BUF_ADDR  0x80000001

/*
 *  Save the buffer on close
 */
#define FMEM_IOCTL_SAVE_BUF  0x80000002

extern FILE * ag_fmemopen (void *buf, ssize_t len, const char *mode);
#endif
#include "proto.h"

typedef union {
    const void*  cp;
    void*        p;
} v2c_t;
MODE v2c_t p2p VALUE( { NULL } );

#ifdef DEBUG_ENABLED
# define AG_ABEND(s)  ag_abend_at(s,__FILE__,__LINE__)
#else
# define AG_ABEND(s)  ag_abend_at(s)
#endif
#ifdef DEBUG_FSM
# define DEBUG
#else
# undef  DEBUG
#endif

#define AG_ABEND_IN(t,m,s) \
    STMTS( pCurTemplate=(t); pCurMacro=(m); AG_ABEND(s);)

extern void  ag_scmStrings_init(   void );
extern void  ag_scmStrings_deinit( void );
extern void  ag_scmStrings_free(   void );
extern char* ag_scribble( size_t size );

/*
 *  Code variations based on the version of Guile:
 */
#if GUILE_VERSION < 106000

# define AG_SCM_STRLEN(_s)       SCM_LENGTH(_s)
# define AG_SCM_STRING_P(_s)     gh_string_p(_s)
# define AG_SCM_BOOL_P(_b)       gh_boolean_p(_b)
# define AG_SCM_SYM_P(_s)        gh_symbol_p(_s)
# define AG_SCM_IS_PROC(_p)      gh_procedure_p(_p)
# define AG_SCM_CHAR_P(_c)       gh_char_p(_c)
# define AG_SCM_VEC_P(_v)        gh_vector_p(_v)
# define AG_SCM_PAIR_P(_p)       gh_pair_p(_p)
# define AG_SCM_NUM_P(_n)        gh_number_p(_n)
# define AG_SCM_LIST_P(_l)       gh_list_p(_l)

# define AG_SCM_MKSTR(_s, _ch)   MUST NOT USE

# define AG_SCM_EVAL_STR(_s)     gh_eval_str(_s)
# define AG_SCM_CHARS(_s)        SCM_CHARS(_s)
# define AG_SCM_STR2SCM(_st,_sz) gh_str2scm(_st,_sz)
# define AG_SCM_STR02SCM(_s)     gh_str02scm(_s)
# define AG_SCM_INT2SCM(_i)      gh_int2scm(_i)
# define AG_SCM_SCM2INT(_i)      gh_scm2int(_i)
# define AG_SCM_LONG2SCM(_i)     gh_long2scm(_i)
# define AG_SCM_SCM2LONG(_i)     gh_scm2long(_i)

static inline char* ag_scm2zchars( SCM s, tCC* type )
{
    if (! AG_SCM_STRING_P( s ))
        AG_ABEND( aprf( zNotStr, type ));

    if (SCM_SUBSTRP(s))
        s = scm_makfromstr( SCM_CHARS(s), SCM_LENGTH(s), 0 );
    return SCM_CHARS(s);  /* pre-Guile 1.7.x */
}

#elif GUILE_VERSION < 107000

# define AG_SCM_STRLEN(_s)       SCM_LENGTH(_s)
# define AG_SCM_STRING_P(_s)     SCM_STRINGP(_s)
# define AG_SCM_BOOL_P(_b)       SCM_BOOLP(_b)
# define AG_SCM_SYM_P(_s)        SCM_SYMBOLP(_s)
# define AG_SCM_IS_PROC(_p)      SCM_NFALSEP( scm_procedure_p(_p))
# define AG_SCM_CHAR_P(_c)       SCM_CHARP(_c)
# define AG_SCM_VEC_P(_v)        SCM_VECTORP(_v)
# define AG_SCM_PAIR_P(_p)       SCM_NFALSEP( scm_pair_p(_p))
# define AG_SCM_NUM_P(_n)        SCM_NUMBERP(_n)
# define AG_SCM_LIST_P(_l)       SCM_NFALSEP( scm_list_p(_l))

# define AG_SCM_MKSTR(_s, _ch)   MUST NOT USE

# define AG_SCM_EVAL_STR(_s)     scm_c_eval_string(_s)
# define AG_SCM_CHARS(_s)        SCM_CHARS(_s)
# define AG_SCM_STR2SCM(_st,_sz) scm_mem2string(_st,_sz)
# define AG_SCM_STR02SCM(_s)     scm_makfrom0str(_s)
# define AG_SCM_INT2SCM(_i)      gh_int2scm(_i)
# define AG_SCM_SCM2INT(_i)      gh_scm2int(_i)
# define AG_SCM_LONG2SCM(_i)     gh_long2scm(_i)
# define AG_SCM_SCM2LONG(_i)     gh_scm2long(_i)

static inline char* ag_scm2zchars( SCM s, tCC* type )
{
    if (! AG_SCM_STRING_P( s ))
        AG_ABEND( aprf( zNotStr, type ));

    if (SCM_SUBSTRP(s))
        s = scm_makfromstr( SCM_CHARS(s), SCM_LENGTH(s), 0 );
    return SCM_CHARS(s);  /* pre-Guile 1.7.x */
}


#else
  extern char* ag_scm2zchars( SCM s, tCC* type );

# define AG_SCM_STRLEN(_s)       SCM_STRING_LENGTH(_s)
# define AG_SCM_STRING_P(_s)     scm_is_string(_s)
# define AG_SCM_BOOL_P(_b)       SCM_BOOLP(_b)
# define AG_SCM_SYM_P(_s)        SCM_SYMBOLP(_s)
# define AG_SCM_IS_PROC(_p)      scm_is_true( scm_procedure_p(_p))
# define AG_SCM_CHAR_P(_c)       SCM_CHARP(_c)
# define AG_SCM_VEC_P(_v)        SCM_VECTORP(_v)
# define AG_SCM_PAIR_P(_p)       scm_is_true( scm_pair_p(_p))
# define AG_SCM_NUM_P(_n)        SCM_NUMBERP(_n)
# define AG_SCM_LIST_P(_l)       SCM_NFALSEP( scm_list_p(_l))

# define AG_SCM_MKSTR(_s, _ch)   MUST NOT USE

# define AG_SCM_EVAL_STR(_s)     scm_c_eval_string(_s)
# define AG_SCM_CHARS(_s)        SCM_STRING_CHARS(_s)
# define AG_SCM_STR2SCM(_st,_sz) scm_from_locale_stringn(_st,_sz)
# define AG_SCM_STR02SCM(_s)     scm_from_locale_string(_s)
# define AG_SCM_INT2SCM(_i)      gh_int2scm(_i)
# define AG_SCM_SCM2INT(_i)      gh_scm2int(_i)
# define AG_SCM_LONG2SCM(_i)     gh_long2scm(_i)
# define AG_SCM_SCM2LONG(_i)     gh_scm2long(_i)
#endif

static inline SCM ag_eval( tCC* pzStr )
{
    SCM res;
    tCC* pzSaveScheme = pzLastScheme; /* Watch for nested calls */
    pzLastScheme = pzStr;

    res = ag_scm_c_eval_string_from_file_line(
        pzStr, pCurTemplate->pzFileName, pCurMacro->lineNo );

    pzLastScheme = pzSaveScheme;
    return res;
}

#endif /* AUTOGEN_HDR */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/autogen.h */
