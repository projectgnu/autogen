[= AutoGen5 Template -*- Mode: C -*-

h=options.h

##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2013 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following sha256 sums:
##
##  8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
##  4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
##  13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd

=][=

(make-tmp-dir)
(dne " *  " "/*  ")

=]
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 *
 *  Automated Options Copyright [=`date +'(C) 1992-%Y'`=] by Bruce Korb
 *
 *  [=(lgpl "AutoOpts" "Bruce Korb" " *  ")=]
 */
[=(make-header-guard "autoopts")=]
#include <sys/types.h>
#include <stdio.h>

#ifndef COMPAT_H_GUARD
/*
 * This is needed for test compilations where the "compat.h"
 * header is not usually available.
 */
#  if defined(HAVE_STDINT_H)
#    include <stdint.h>
#  elif defined(HAVE_INTTYPES_H)
#    include <inttypes.h>
#  endif /* HAVE_STDINT/INTTYPES_H */

#  if defined(HAVE_LIMITS_H)
#    include <limits.h>
#  elif defined(HAVE_SYS_LIMITS_H)
#    include <sys/limits.h>
#  endif /* HAVE_LIMITS/SYS_LIMITS_H */

#  if defined(HAVE_SYSEXITS_H)
#    include <sysexits.h>
#  endif /* HAVE_SYSEXITS_H */

#  if defined(HAVE_STDBOOL_H)
#    include <stdbool.h>
#  else
     typedef enum { false = 0, true = 1 } _Bool;
#    define bool _Bool

     /* The other macros must be usable in preprocessor directives.  */
#    define false 0
#    define true 1
#  endif /* HAVE_SYSEXITS_H */
#endif /* COMPAT_H_GUARD */
// END-CONFIGURED-HEADERS

/**
 * Defined to abnormal value of EX_USAGE.  Used to indicate that paged usage
 * was requested.  It is used to distinguish a --usage from a --help request.
 * --usage is abbreviated and --help gives as much help as possible.
 */
#define AO_EXIT_REQ_USAGE 10064

/**
 *  PUBLIC DEFINES
 *
 *  The following defines may be used in applications that need to test the
 *  state of an option.  To test against these masks and values, a pointer
 *  to an option descriptor must be obtained.  There are two ways:
 *
 *  1. inside an option processing procedure, it is the second argument,
 *     conventionally "tOptDesc* pOD".
 *
 *  2. Outside of an option procedure (or to reference a different option
 *     descriptor), use either "&DESC( opt_name )" or "&pfx_DESC( opt_name )".
 *
 *  See the relevant generated header file to determine which and what
 *  values for "opt_name" are available.
 */
#define OPTIONS_STRUCT_VERSION      [=  vers-curr    =]
#define OPTIONS_VERSION_STRING      "[= vers-sovers  =]"
#define OPTIONS_MINIMUM_VERSION     [=  vers-min     =]
#define OPTIONS_MIN_VER_STRING      "[= vers-min-str =]"
#define OPTIONS_DOTTED_VERSION      "[= display-ver  =]"
#define OPTIONS_VER_TO_NUM(_v, _r)  (((_v) * 4096) + (_r))

/**
 * Option argument types.  This must fit in the OPTST_ARG_TYPE_MASK
 * field of the fOptState field of an option descriptor (tOptDesc).
 * It will be a problem to extend beyond 4 bits.
 */
typedef enum {
    OPARG_TYPE_NONE         =  0, /**< does not take an argument         */
    OPARG_TYPE_STRING       =  1, /**< default type/ vanilla string      */
    OPARG_TYPE_ENUMERATION  =  2, /**< opt arg is an enum (keyword list) */
    OPARG_TYPE_BOOLEAN      =  3, /**< opt arg is boolean-valued         */
    OPARG_TYPE_MEMBERSHIP   =  4, /**< opt arg sets set membership bits  */
    OPARG_TYPE_NUMERIC      =  5, /**< opt arg is a long int             */
    OPARG_TYPE_HIERARCHY    =  6, /**< option arg is hierarchical value  */
    OPARG_TYPE_FILE         =  7, /**< option arg names a file           */
    OPARG_TYPE_TIME         =  8, /**< opt arg is a time duration        */
    OPARG_TYPE_FLOAT        =  9, /**< opt arg is a floating point num   */
    OPARG_TYPE_DOUBLE       = 10, /**< opt arg is a double prec. float   */
    OPARG_TYPE_LONG_DOUBLE  = 11, /**< opt arg is a long double prec.    */
    OPARG_TYPE_LONG_LONG    = 12  /**< opt arg is a long long int        */
} teOptArgType;

/**
 * value descriptor for sub options
 */
typedef struct optionValue {
    teOptArgType        valType;        /**< which argument type    */
    char *              pzName;         /**< name of the sub-option */
    union {
        char            strVal[1];      /**< OPARG_TYPE_STRING      */
        unsigned int    enumVal;        /**< OPARG_TYPE_ENUMERATION */
        unsigned int    boolVal;        /**< OPARG_TYPE_BOOLEAN     */
        unsigned long   setVal;         /**< OPARG_TYPE_MEMBERSHIP  */
        long            longVal;        /**< OPARG_TYPE_NUMERIC     */
        void*           nestVal;        /**< OPARG_TYPE_HIERARCHY   */
    } v;
} tOptionValue;

/**
 * file argument state and handling.
 */
typedef enum {
    FTYPE_MODE_MAY_EXIST        = 0x00, /**< may or may not exist */
    FTYPE_MODE_MUST_EXIST       = 0x01, /**< must pre-exist       */
    FTYPE_MODE_MUST_NOT_EXIST   = 0x02, /**< must *not* pre-exist */
    FTYPE_MODE_EXIST_MASK       = 0x03, /**< mask for these bits  */
    FTYPE_MODE_NO_OPEN          = 0x00, /**< leave file closed    */
    FTYPE_MODE_OPEN_FD          = 0x10, /**< call open(2)         */
    FTYPE_MODE_FOPEN_FP         = 0x20, /**< call fopen(3)        */
    FTYPE_MODE_OPEN_MASK        = 0x30  /**< open/fopen/not open  */
} teOptFileType;

/**
 * the open flag bits or the mode string, depending on the open type.
 */
typedef union {
    int             file_flags;  /**< open(2) flag bits    */
    char const *    file_mode;   /**< fopen(3) mode string */
} tuFileMode;

typedef struct argList tArgList;
#define MIN_ARG_ALLOC_CT   6
#define INCR_ARG_ALLOC_CT  8
struct argList {
    int             useCt;
    int             allocCt;
    char const *    apzArgs[MIN_ARG_ALLOC_CT];
};

/**
 *  Bits in the fOptState option descriptor field.
 */
[= `
src_dir=\`pwd\`
mk_mask() {
    hdr=${1%.def}.h
    cd ${tmp_dir}
    ${AGexe} -L ${src_dir}/tpl ${src_dir}/${1} || \
        die "Cannot process ${hdr}"
    sed -e '1,/#define.*GUARD *1/d;/^#include/d;/^#endif/,$d' $hdr
}

mk_mask opt-state.def` =]

#ifdef NO_OPTIONAL_OPT_ARGS
# undef  OPTST_ARG_OPTIONAL
# define OPTST_ARG_OPTIONAL   0
#endif

#define VENDOR_OPTION_VALUE   'W'

#define SELECTED_OPT(_od)     ((_od)->fOptState  & OPTST_SELECTED_MASK)
#define UNUSED_OPT(  _od)     (((_od)->fOptState & OPTST_SET_MASK) == 0)
#define DISABLED_OPT(_od)     ((_od)->fOptState  & OPTST_DISABLED)
#define OPTION_STATE(_od)     ((_od)->fOptState)
#define OPTST_SET_ARGTYPE(_n) ((_n) << OPTST_ARG_TYPE_SHIFT)
#define OPTST_GET_ARGTYPE(_f) \
    (((_f)&OPTST_ARG_TYPE_MASK) >> OPTST_ARG_TYPE_SHIFT)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  PRIVATE INTERFACES
 *
 *  The following values are used in the generated code to communicate
 *  with the option library procedures.  They are not for public use
 *  and may be subject to change.
 */

/**
 *  Define the processing state flags
 */
[= `mk_mask proc-state.def` =]

#define STMTS(s)  do { s; } while (false)

/*
 *  The following must be #defined instead of typedef-ed
 *  because "char const" cannot both be applied to a type,
 *  tho each individually can...so they all are
 */
#define tCC         char const

/*
 *  Define a structure that describes each option and
 *  a pointer to the procedure that handles it.
 *  The argument is the count of this flag previously seen.
 */
typedef struct options  tOptions;
typedef struct optDesc  tOptDesc;
typedef struct optNames tOptNames;
#define OPTPROC_EMIT_USAGE      ((tOptions *)0x01UL)
#define OPTPROC_EMIT_SHELL      ((tOptions *)0x02UL)
#define OPTPROC_RETURN_VALNAME  ((tOptions *)0x03UL)
#define OPTPROC_EMIT_LIMIT      ((tOptions *)0x0FUL)

/*
 *  The option procedures do the special processing for each
 *  option flag that needs it.
 */
typedef void (tOptProc)(tOptions * pOpts, tOptDesc * pOptDesc);
typedef tOptProc * tpOptProc;

/*
 *  The usage procedure will never return.  It calls "exit(2)"
 *  with the "exitCode" argument passed to it.
 */
// coverity[+kill]
typedef void (tUsageProc)(tOptions* pOpts, int exitCode);
typedef tUsageProc * tpUsageProc;

/*
 *  Special definitions.  "NOLIMIT" is the 'max' value to use when
 *  a flag may appear multiple times without limit.  "NO_EQUIVALENT"
 *  is an illegal value for 'optIndex' (option description index).
 */
#define NOLIMIT          USHRT_MAX
#define OPTION_LIMIT     SHRT_MAX
#define NO_EQUIVALENT    (OPTION_LIMIT+1)

/**
 * Option argument value.  Which is valid is determined by:
 * (fOptState & OPTST_ARG_TYPE_MASK) >> OPTST_ARG_TYPE_SHIFT
 * which will yield one of the teOptArgType values.
 */
typedef union {
    char const *    argString;
    uintptr_t       argEnum;
    uintptr_t       argIntptr;
    long            argInt;
    unsigned long   argUint;
    unsigned int    argBool;
    FILE *          argFp;
    int             argFd;
} opt_arg_union_t;

#define             pzLastArg       optArg.argString
#define             optArgBucket_t  opt_arg_union_t

/*
 *  Descriptor structure for each option.
 *  Only the fields marked "PUBLIC" are for public use.
 */
struct optDesc {
    unsigned short const    optIndex;         /* PUBLIC */
    unsigned short const    optValue;         /* PUBLIC */
    unsigned short          optActualIndex;   /* PUBLIC */
    unsigned short          optActualValue;   /* PUBLIC */

    unsigned short const    optEquivIndex;    /* PUBLIC */
    unsigned short const    optMinCt;
    unsigned short const    optMaxCt;
    unsigned short          optOccCt;         /* PUBLIC */

    opt_state_mask_t        fOptState;        /* PUBLIC */
    unsigned int            reserved;
    opt_arg_union_t         optArg;           /* PUBLIC */
    void *                  optCookie;        /* PUBLIC */

    int const  * const      pOptMust;
    int const  * const      pOptCant;
    tpOptProc    const      pOptProc;
    char const * const      pzText;

    char const * const      pz_NAME;
    char const * const      pz_Name;
    char const * const      pz_DisableName;
    char const * const      pz_DisablePfx;
};

/*
 *  Some options need special processing, so we store their
 *  indexes in a known place:
 */
typedef struct optSpecIndex tOptSpecIndex;
struct optSpecIndex {
    const unsigned short        more_help;
    const unsigned short        save_opts;
    const unsigned short        number_option;
    const unsigned short        default_opt;
};

/*
 *  The procedure generated for translating option text
 */
typedef void (tOptionXlateProc)(void);

/*
 * Everything marked "PUBLIC" is also marked "const".  Public access is not
 * a license to modify.  Other fields are used and modified by the library.
 * They are also subject to change without any notice.
 * Do not even look at these outside of libopts.
 */
struct options {
    int const                   structVersion;
    unsigned int                origArgCt;
    char **                     origArgVect;
    proc_state_mask_t           fOptSet;
    unsigned int                curOptIdx;
    char *                      pzCurOpt;

    char const * const          pzProgPath;         /* PUBLIC */
    char const * const          pzProgName;         /* PUBLIC */
    char const * const          pzPROGNAME;         /* PUBLIC */
    char const * const          pzRcName;           /* PUBLIC */
    char const * const          pzCopyright;        /* PUBLIC */
    char const * const          pzCopyNotice;       /* PUBLIC */
    char const * const          pzFullVersion;      /* PUBLIC */
    char const * const *        const papzHomeList;
    char const * const          pzUsageTitle;
    char const * const          pzExplain;
    char const * const          pzDetail;
    tOptDesc   * const          pOptDesc;           /* PUBLIC */
    char const * const          pzBugAddr;          /* PUBLIC */

    void*                       pExtensions;
    void*                       pSavedState;

    // coverity[+kill]
    tpUsageProc                 pUsageProc;
    tOptionXlateProc*           pTransProc;

    tOptSpecIndex               specOptIdx;
    int const                   optCt;
    int const                   presetOptCt;
    char const *                pzFullUsage;
    char const *                pzShortUsage;
    /* PUBLIC: */
    opt_arg_union_t const * const originalOptArgArray;
    void * const * const        originalOptArgCookie;
    char const * const          pzPkgDataDir;
    char const * const          pzPackager;
};

/*
 *  Versions where in various fields first appear:
 *  ($AO_CURRENT * 4096 + $AO_REVISION, but $AO_REVISION must be zero)
 */
#define originalOptArgArray_STRUCT_VERSION  131072 /* AO_CURRENT = 32 */
#define HAS_originalOptArgArray(_opt) \
    ((_opt)->structVersion >= originalOptArgArray_STRUCT_VERSION)

#define pzPkgDataDir_STRUCT_VERSION  139264 /* AO_CURRENT = 34 */
#define HAS_pzPkgDataDir(_opt) \
    ((_opt)->structVersion >= pzPkgDataDir_STRUCT_VERSION)

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

/**
 *  Everything needed to be known about an mmap-ed file.
 *
 *  This is an output only structure used by text_mmap and text_munmap.
 *  Clients must not alter the contents and must provide it to both
 *  the text_mmap and text_munmap procedures.  BE ADVISED: if you are
 *  mapping the file with PROT_WRITE the NUL byte at the end MIGHT NOT
 *  BE WRITABLE.  In any event, that byte is not be written back
 *  to the source file.  ALSO: if "txt_data" is valid and "txt_errno"
 *  is not zero, then there *may* not be a terminating NUL.
 */
typedef struct {
    void *      txt_data;      /*@< text file data   */
    size_t      txt_size;      /*@< actual file size */
    size_t      txt_full_size; /*@< mmaped mem size  */
    int         txt_fd;        /*@< file descriptor  */
    int         txt_zero_fd;   /*@< fd for /dev/zero */
    int         txt_errno;     /*@< warning code     */
    int         txt_prot;      /*@< "prot" flags     */
    int         txt_flags;     /*@< mapping type     */
} tmap_info_t;

#define TEXT_MMAP_FAILED_ADDR(a)  ((void*)(a) ==  (void*)MAP_FAILED)

#ifdef  __cplusplus
#define CPLUSPLUS_OPENER extern "C" {
CPLUSPLUS_OPENER
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

FOR export-func         =][=

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
extern [= ?% ret-type "%s" "void"  =] [=name=]([=
   IF (not (exist? "arg"))
          =]void[=
   ELSE   =][=(join ", " (stack "arg.arg-type")) =][=
   ENDIF  =]);
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
