[= AutoGen5 Template -*- Mode: C -*-

h=options.h

# Automated Options copyright 1992-2005 Bruce Korb
# Time-stamp:      "2005-10-29 15:06:59 bkorb"
# ID:  $Id: options_h.tpl,v 4.19 2005/10/29 22:13:10 bkorb Exp $

=][=

(dne " *  " "/*  ")

=]
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 *
 *  Automated Options copyright 1992-[=`date +Y`=] Bruce Korb
 *
[=(lgpl "AutoOpts" "Bruce Korb" " *  ")=]
 */
[=(make-header-guard "autoopts")=]
#include <sys/types.h>

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif /* HAVE_STDINT/INTTYPES_H */

#if defined(HAVE_LIMITS_H)
# include <limits.h>
#elif defined(HAVE_SYS_LIMITS_H)
# include <sys/limits.h>
#endif /* HAVE_LIMITS/SYS_LIMITS_H */

/*
 *  PUBLIC DEFINES
 *
 *  The following defines may be used in applications that need to test the
 *  state of an option.  To test against these masks and values, a pointer
 *  to an option descriptor must be obtained.  There are two ways:
 *
 *  1. inside an option processing procedure, it is the second argument,
 *  conventionally "tOptDesc* pOD".
 *
 *  2.  Outside of an option procedure (or to reference a different option
 *  descriptor), use either "&DESC( opt_name )" or "&pfx_DESC( opt_name )".
 *
 *  See the relevant generated header file to determine which and what
 *  values for "opt_name" are available.
 */

typedef enum {
    OPARG_TYPE_NONE             = 0,
    OPARG_TYPE_STRING           = 1,    /* default type/ vanilla string      */
    OPARG_TYPE_ENUMERATION      = 2,    /* opt arg is an enum (keyword list) */
    OPARG_TYPE_BOOLEAN          = 3,    /* opt arg is boolean-valued         */
    OPARG_TYPE_MEMBERSHIP       = 4,    /* opt arg sets set membership bits  */
    OPARG_TYPE_NUMERIC          = 5,    /* opt arg has numeric value         */
    OPARG_TYPE_HIERARCHY        = 6     /* option arg is hierarchical value  */
} teOptArgType;

typedef struct optionValue {
    teOptArgType        valType;
    char*               pzName;
    union {
        char            strVal[1];      /* OPARG_TYPE_STRING      */
        int             enumVal;        /* OPARG_TYPE_ENUMERATION */
        int             boolVal;        /* OPARG_TYPE_BOOLEAN     */
        long            setVal;         /* OPARG_TYPE_MEMBERSHIP  */
        long            longVal;        /* OPARG_TYPE_NUMERIC     */
        void*           nestVal;        /* OPARG_TYPE_HIERARCHY   */
    } v;
} tOptionValue;

#define OPTST_SET_ARGTYPE(n) ((n) << 12)
#define OPTST_GET_ARGTYPE(f) (((f) & OPTST_ARG_TYPE_MASK) >> 12)

/*
 *  Bits in the fOptState option descriptor field.
 */
#define OPTST_INIT           0x0000000  /* Initial compiled value            */
#define OPTST_SET            0x0000001  /* Set via the "SET_OPT()" macro     */
#define OPTST_PRESET         0x0000002  /* Set via an RC/INI file            */
#define OPTST_DEFINED        0x0000004  /* Set via a command line option     */

#define OPTST_SET_MASK       0x0000007  /* mask of flags that show set state */

#define OPTST_EQUIVALENCE    0x0000010  /* selected by equiv'ed option       */
#define OPTST_DISABLED       0x0000020  /* option is in disabled state       */

#define OPTST_NO_INIT        0x0000100  /* option cannot be preset           */
#define OPTST_NUMBER_OPT     0x0000200  /* opt value (flag) is any digit     */
#define OPTST_STACKED        0x0000400  /* opt uses optionStackArg procedure */
#define OPTST_INITENABLED    0x0000800  /* option defaults to enabled        */
#define OPTST_ARG_TYPE_MASK  0x000F000  /* bits used to specify opt arg type */
#define OPTST_ARG_OPTIONAL   0x0010000  /* the option argument not required  */
#define OPTST_IMM            0x0020000  /* process option on first pass      */
#define OPTST_DISABLE_IMM    0x0040000  /* process disablement on first pass */
#define OPTST_OMITTED        0x0080000  /* compiled out of program           */
#define OPTST_MUST_SET       0x0100000  /* must be set or pre-set            */
#define OPTST_DOCUMENT       0x0200000  /* opt is for documentation only     */
#define OPTST_TWICE          0x0400000  /* process option twice - imm + reg  */
#define OPTST_DISABLE_TWICE  0x0800000  /* process disabled option twice     */

#define OPTST_PERSISTENT     0xFFFFF00  /* mask of flags that do not change  */

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
#define OPTPROC_NONE        0x000000
#define OPTPROC_LONGOPT     0x000001 /* Process long style options      */
#define OPTPROC_SHORTOPT    0x000002 /* Process short style "flags"     */
#define OPTPROC_ERRSTOP     0x000004 /* Stop on argument errors         */
#define OPTPROC_DISABLEDOPT 0x000008 /* Current option is disabled      */
#define OPTPROC_NO_REQ_OPT  0x000010 /* no options are required         */
#define OPTPROC_NUM_OPT     0x000020 /* there is a number option        */
#define OPTPROC_INITDONE    0x000040 /* have initializations been done? */
#define OPTPROC_NEGATIONS   0x000080 /* any negation options?           */
#define OPTPROC_ENVIRON     0x000100 /* check environment?              */
#define OPTPROC_NO_ARGS     0x000200 /* Disallow remaining arguments    */
#define OPTPROC_ARGS_REQ    0x000400 /* Require arguments after options */
#define OPTPROC_REORDER     0x000800 /* reorder arguments after options */
#define OPTPROC_GNUUSAGE    0x001000 /* emit usage in GNU style         */
#define OPTPROC_TRANSLATE   0x002000 /* Translate strings in tOptions   */
#define OPTPROC_HAS_IMMED   0x004000 /* program defines immed options   */
#define OPTPROC_PRESETTING  0x800000 /* opt processing in preset state  */

#define STMTS(s)  do { s; } while (0)

/*
 *  The following must be #defined instead of typedef-ed
 *  because "static const" cannot both be applied to a type,
 *  tho each individually can...so they all are
 */
#define tSCC        static const char
#define tCC         const char
#define tAoSC       static char
#define tAoUC       unsigned char
#define tAoUI       unsigned int
#define tAoUL       unsigned long
#define tAoUS       unsigned short

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
#define NOLIMIT          USHRT_MAX
#define OPTION_LIMIT     SHRT_MAX
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
    int             useCt;
    int             allocCt;
    tCC*            apzArgs[ MIN_ARG_ALLOC_CT ];
};

/*
 *  Descriptor structure for each option.
 *  Only the fields marked "PUBLIC" are for public use.
 */
struct optDesc {
    tAoUS           optIndex;         /* PUBLIC */
    tAoUS           optValue;         /* PUBLIC */
    tAoUS           optActualIndex;   /* PUBLIC */
    tAoUS           optActualValue;   /* PUBLIC */

    tAoUS           optEquivIndex;    /* PUBLIC */
    tAoUS           optMinCt;
    tAoUS           optMaxCt;
    tAoUS           optOccCt;         /* PUBLIC */

    tAoUI           fOptState;        /* PUBLIC */
    tAoUI           reserved;
    tCC*            pzLastArg;        /* PUBLIC */
    void*           optCookie;        /* PUBLIC */

    const int *     pOptMust;
    const int *     pOptCant;
    tpOptProc       pOptProc;
    const char*     pzText;

    const char*     pz_NAME;
    const char*     pz_Name;
    const char*     pz_DisableName;
    const char*     pz_DisablePfx;
};

/*
 *  Some options need special processing, so we store their
 *  indexes in a known place:
 */
typedef struct optSpecIndex tOptSpecIndex;
struct optSpecIndex {
    tAoUS           more_help;
    tAoUS           save_opts;
    tAoUS           number_option;
    tAoUS           default_opt;
};
[=

 ;;;
 ;;; These "vers" values are manipulated by the contents of ../VERSION
 ;;;

(shellf "
sed '/define AO_TEMPLATE_VERSION/s/SION .*/SION %s/' opthead.tpl > XXXX
mv -f XXXX opthead.tpl" (get "vers-template"))

=]
#define  OPTIONS_STRUCT_VERSION  [=  vers-curr    =]
#define  OPTIONS_VERSION_STRING  "[= vers-info    =]"
#define  OPTIONS_MINIMUM_VERSION [=  vers-min     =]
#define  OPTIONS_MIN_VER_STRING  "[= vers-min-str =]"

/*
 *  The procedure generated for translating option text
 */
typedef void (tOptionXlateProc)(void);

struct options {
    const int         structVersion;
    int               origArgCt;
    char**            origArgVect;
    unsigned int      fOptSet;
    unsigned int      curOptIdx;
    char*             pzCurOpt;

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
    tOptDesc*         pOptDesc;
    const char*       pzBugAddr;

    void*             pExtensions;
    void*             pSavedState;

    tpUsageProc       pUsageProc;
    tOptionXlateProc* pTransProc;

    tOptSpecIndex     specOptIdx;
    const int         optCt;
    const int         presetOptCt;
};

/*
 *  "token list" structure returned by "string_tokenize()"
 */
typedef struct {
    unsigned long   tkn_ct;
    unsigned char*  tkn_list[1];
} token_list_t;

/*
 *  Hide the interface - it pollutes a POSIX claim, but leave it for
 *  anyone #include-ing this header
 */
#define strneqvcmp      option_strneqvcmp
#define streqvcmp       option_streqvcmp
#define streqvmap       option_streqvmap
#define strequate       option_strequate
#define strtransform    option_strtransform

/*
 *  This is an output only structure used by text_mmap and text_munmap.
 *  Clients must not alter the contents and must provide it to both
 *  the text_mmap and text_munmap procedures.  BE ADVISED: if you are
 *  mapping the file with PROT_WRITE the NUL byte at the end MIGHT NOT
 *  BE WRITABLE.  In any event, that byte is not be written back
 *  to the source file.  ALSO: if "txt_data" is valid and "txt_errno"
 *  is not zero, then there *may* not be a terminating NUL.
 */
typedef struct {
    void*       txt_data;      /* text file data   */
    size_t      txt_size;      /* actual file size */
    size_t      txt_full_size; /* mmaped mem size  */
    int         txt_fd;        /* file descriptor  */
    int         txt_zero_fd;   /* fd for /dev/zero */
    int         txt_errno;     /* warning code     */
    int         txt_prot;      /* "prot" flags     */
    int         txt_flags;     /* mapping type     */
    int         txt_alloc;     /* if we malloced memory */
} tmap_info_t;

#define TEXT_MMAP_FAILED_ADDR(a)  ((void*)(a) == MAP_FAILED)

/*
 *  When loading a line (or block) of text as an option, the value can
 *  be processed in any of several modes:
 *
 *  @table @samp
 *  @item keep
 *  Every part of the value between the delimiters is saved.
 *
 *  @item uncooked
 *  Even if the value begins with quote characters, do not do quote processing.
 *
 *  @item cooked
 *  If the value looks like a quoted string, then process it.
 *  Double quoted strings are processed the way strings are in "C" programs,
 *  except they are treated as regular characters if the following character
 *  is not a well-established escape sequence.
 *  Single quoted strings (quoted with apostrophies) are handled the way
 *  strings are handled in shell scripts, *except* that backslash escapes
 *  are honored before backslash escapes and apostrophies.
 *  @end table
 */
typedef enum {
    OPTION_LOAD_COOKED,
    OPTION_LOAD_UNCOOKED,
    OPTION_LOAD_KEEP
} tOptionLoadMode;

#ifdef  __cplusplus
extern "C" {
#define CPLUSPLUS_CLOSER }
#else
#define CPLUSPLUS_CLOSER
#endif

/*
 *  The following routines may be coded into AutoOpts client code:
 */[=
 (define if-text   "")
 (define note-text "")
 (define end-text  "")
 (define note-fmt
    "\n *\n * the %s function is available only if %s is%s defined")

 (out-push-new)
 (out-suspend "priv")   =][=

FOR export_func         =][=

  IF

  (if (exist? "ifndef")
      (begin
        (set! if-text    (string-append "\n#ifndef " (get "ifndef")))
        (set! note-text  (sprintf note-fmt (get "name") (get "ifndef") " not"))
        (set! end-text   (sprintf "#endif /* %s */\n" (get "ifndef")))
      )

  (if (exist? "ifdef")
      (begin
        (set! if-text    (string-append "\n#ifdef " (get "ifdef")))
        (set! note-text  (sprintf note-fmt (get "name") (get "ifdef") ""))
        (set! end-text   (sprintf "#endif /* %s */\n" (get "ifdef")))
      )

  (begin
        (set! if-text    "")
        (set! note-text  "")
        (set! end-text   "")
  )  ))

 (not (exist? "private")) =]

/* From: [=srcfile=] line [=linenum=]
 *
 * [=name=] - [=what=][=
    IF (exist? "arg") =]
 *
 * Arguments:[=
       FOR arg
=]
 *   [=(sprintf "%-12s " (get "arg-name"))=][=arg-desc=][=
      ENDFOR arg  =][=
    ENDIF =][=

    IF (exist? "ret-type") =]
 *
 * Returns: [=ret-type=] - [=ret-desc=][=

    ENDIF  =][=(. note-text)=]
 *
[=(prefix " *  " (get "doc"))=]
 */[=

  ENDIF =][=

  (if (exist? "private") (out-resume "priv"))

  if-text

=]
extern [= ?% ret-type "%s" "void"  =] [=name=]( [=
   IF (not (exist? "arg"))
          =]void[=
   ELSE   =][=(join ", " (stack "arg.arg-type")) =][=
   ENDIF  =] );
[=

  (if (exist? "ifndef")
      (sprintf "\n#endif /* %s */" (get "ifndef"))
  (if (exist? "ifdef")
      (sprintf "\n#endif /* %s */"  (get "ifdef"))  ))

  (if (exist? "private") (out-suspend "priv"))
  end-text

=][=

ENDFOR export-func

=]
/*  AutoOpts PRIVATE FUNCTIONS:  */
tOptProc optionStackArg, optionUnstackArg, optionBooleanVal, optionNumericVal;
[= (out-resume "priv") (out-pop #t) =]
CPLUSPLUS_CLOSER
#endif /* [=(. header-guard)=] */
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * [=(out-name)=] ends here */[=
## optexport.tpl ends here  =]
