[= AutoGen5 template h   -*- Mode: C -*-

# $Id: functions.tpl,v 3.1 2004/10/11 18:23:01 bkorb Exp $

=]
[=(dne " *  " "/*  ")=]
 *
 *  Tables of Text Functions for AutoGen
 *
 *  copyright 1992-1999 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 *
 *  The [=(count "macfunc")
         =] AutoGen functions are tagged with special attributes:
 *
 *  A - is invoked via an alias
 *
 *  U - is unnamed.  May *not* be explicitly invoked.  May not have
 *      have an alias.  These three are used by AutoGen for its purposes.
 *
 *  L - has a special load procedure defined
 *
 *  S - situational.  May be explicitly invoked in certain situations.
 *      For example, "ELSE" may only be specified within an "IF" block.
 *      Their load procedures are enabled by the block macro (e.g. IF),
 *      and disabled by the block ending macro (e.g. ENDIF).
 *      While disabled, the load procedure is the "Bogus" method.
 *
 *      If a function is neither has a special load procedure nor is
 *      situational, then the "Unknown" load method is applied.
 *
 *  H - has a handler procedure defined.  Only these procedures should
 *      be encountered by the dispatcher during processing.
 *[=
FOR macfunc =]
 *  [=

  IF (exist? "alias")     =]A[=
    IF (exist? "unnamed") =][=
      ERROR % name
         "The %s function is unnamed, but has aliases?!?!" =][=
    ENDIF =][=
  ELIF (exist? "unnamed") =]U[=
  ELSE                    =] [=
  ENDIF =] [=

  IF   (exist? "load_proc")    =]L[=
  ELIF (exist? "situational")  =]S[=
    IF (exist? "handler_proc") =][=
      ERROR % name
         "The %s function is situational and has a handler" =][=
    ENDIF =][=
  ELSE                         =] [=
  ENDIF =] [=

  IF (exist? "handler_proc")    =]H[=
  ELSE                          =] [=
  ENDIF =] - [=

  % name (string-upcase! "%-12s") =][=what=][=
ENDFOR macfunc =]
 */

#ifndef AUTOGEN_FUNCTIONS_H
#define AUTOGEN_FUNCTIONS_H

#define FUNC_CT    [= (count "macfunc") =]

extern const char*  apzFuncNames[ FUNC_CT ];

/*
 *  Enumerate all the function types, whether they have
 *  implementation functions or not.
 */
typedef enum {[=
FOR macfunc =]
    FTYP_[=% name (sprintf "%%-13s" (string-upcase! "%s,"))=] /* [=what=] */[=
ENDFOR macfunc =]

    FTYP_SELECT_COMPARE_FULL          = 0x8000,  /* *==* */
    FTYP_SELECT_COMPARE_SKP_START     = 0x8001,  /* *==  */
    FTYP_SELECT_COMPARE_SKP_END       = 0x8002,  /*  ==* */
    FTYP_SELECT_COMPARE               = 0x8003,  /*  ==  */

    FTYP_SELECT_EQUIVALENT_FULL       = 0x8004,  /* *=*  */
    FTYP_SELECT_EQUIVALENT_SKP_START  = 0x8005,  /* *=   */
    FTYP_SELECT_EQUIVALENT_SKP_END    = 0x8006,  /*  =*  */
    FTYP_SELECT_EQUIVALENT            = 0x8007,  /*  =   */

    FTYP_SELECT_MATCH_FULL            = 0x8008,  /* *~~* */
    FTYP_SELECT_MATCH_SKP_START       = 0x8009,  /* *~~  */
    FTYP_SELECT_MATCH_SKP_END         = 0x800A,  /*  ~~* */
    FTYP_SELECT_MATCH                 = 0x800B,  /*  ~~  */

    FTYP_SELECT_EQV_MATCH_FULL        = 0x800C,  /* *~*  */
    FTYP_SELECT_EQV_MATCH_SKP_START   = 0x800D,  /* *~   */
    FTYP_SELECT_EQV_MATCH_SKP_END     = 0x800E,  /*  ~*  */
    FTYP_SELECT_EQV_MATCH             = 0x800F,  /*  ~   */

    FTYP_SELECT_MATCH_ANYTHING        = 0x801C   /*  *   */
} teFuncType;

/*
 *  The function processing procedures.
 */[=

FOR macfunc =][=
  IF (exist? "handler_proc") =]
tHdlrProc mFunc_[=% name (string-capitalize! "%s") =];[=
  ENDIF =][=
ENDFOR macfunc =]

/*
 *  Template Loading Functions
 */
tLoadProc mLoad_Ending; /* generic block loading ending */[=

FOR macfunc =][=
  IF (and (exist? "load_proc")
          (= (string-length (get "load_proc")) 0) ) =]
tLoadProc mLoad_[=% name (string-capitalize! "%s")=];[=
  ENDIF =][=
ENDFOR macfunc =]

extern tpLoadProc   apLoadProc[ [=(count "macfunc") =] ];
extern tpLoadProc*  papLoadProc;
#endif /* AUTOGEN_FUNCTIONS_H */

#ifdef LOAD_FUNCTIONS  /* tpParse.c use only * * * * * * * * * * * * * * * *
 *
 *  Pointers for the load processing (template scanning phase)
 */
tpLoadProc apLoadProc[ [=(count "macfunc") =] ] = {[=
FOR macfunc "," =]
    /* [=% name "%-10s" =]*/ &mLoad_[=
  IF   (> (len "load_proc") 0)=][=% load_proc (string-capitalize! "%s") =][=
  ELIF (exist? "load_proc")   =][=% name (string-capitalize! "%s") =][=
  ELIF (exist? "situational") =]Bogus   /*dynamic*/[=
  ELSE                        =]Unknown /*default*/[=
  ENDIF =][=
ENDFOR macfunc =]
};
tpLoadProc* papLoadProc = apLoadProc;

/*
 *  name-to-function type mapping table.
 *  This table must be sorted alphabetically by the content
 *  of the naming string.
 */
typedef struct name_type tNameType;
struct name_type {
    size_t      cmpLen;  /* compare length (sans NUL) */
    tCC*        pName;   /* ptr to name */
    teFuncType  fType;   /* function type enum */
    int         unused;
};

/*
 *  Define all the strings that are used to determine
 *  the function enumeration number.  Sort by string content.
 */
[=

FOR macfunc =][=
  IF (not (exist? "unnamed")) =][=
    IF (exist? "alias") =][=
      FOR alias
=]tSCC [= (sprintf "%-15s" (sprintf "zFn_%s_%d[]"
                   (string-capitalize! (get "name")) (for-index)))
          =] = [=(c-string (get "alias"))=];
[=
      ENDFOR alias =][=
    ELSE
=]tSCC [= (sprintf "%-15s" (sprintf "zFn_%s[]"
                   (string-capitalize! (get "name"))))
          =] = "[=% name (string-upcase! "%s")=]";
[=
    ENDIF =][=
  ENDIF =][=
ENDFOR macfunc =][=

(out-push-new (string-append (out-name) ".tmp"))
(shell (sprintf "file=%s" (out-name))) =][=

FOR macfunc =][=
  IF (not (exist? "unnamed"))  =][=
    IF (exist? "alias") =][=
      FOR alias
=][=alias
=] ::    { [= (sprintf "%2d, zFn_%-12s FTYP_%s"  (len "alias")
              (sprintf "%s_%d," (string-capitalize! (get "name")) (for-index))
              (string-upcase! (get "name"))  )
=] },
[=    ENDFOR alias =][=

    ELSE  alias does not exist
=][=% name (string-upcase! "%s")
=] ::    { [= (sprintf "%2d, zFn_%-12s FTYP_%s" (len "name")
              (string-append (string-capitalize! (get "name")) ",")
              (string-upcase! (get "name"))  )
=] },
[=  ENDIF =][=
  ENDIF =][=
ENDFOR macfunc=][=

(out-pop) =]

/*
 *  The number of names by which the macros go.
 *  Some have multiple names (e.g. selection clauses).
 */
[= `
hict=\`egrep '^[A-Z]' $file | wc -l\`
loct=\`egrep -v '^[A-Z]' $file | wc -l\`
cat <<_EOF_
#define FUNC_ALIAS_LOW_INDEX    0
#define FUNC_ALIAS_HIGH_INDEX   \`expr $loct - 1\`
#define FUNC_NAMES_LOW_INDEX    \`echo $loct\`
#define FUNC_NAMES_HIGH_INDEX   \`expr $hict + $loct - 1\`
#define FUNCTION_NAME_CT        \`expr $hict + $loct\`

/*
 *  And now, the table sorted by string content
 */
tNameType nameTypeTable[ FUNCTION_NAME_CT ] = {
_EOF_
egrep -v '^[A-Z]' $file | sort | sed -e 's/^.*:://'
egrep    '^[A-Z]' $file | sort | sed -e 's/^.*:://' -e '$s/,$//'

rm -f $file ` =] };

const char*  apzFuncNames[ FUNC_CT ] = {
[=(out-push-new (string-append (out-name) ".tmp")) =][=
FOR macfunc "\n" =]"[= % name
  (if (exist? "unnamed") (string-capitalize! "%s")
                         (string-upcase! "%1$s")) =]"[=
ENDFOR macfunc=][=
(out-pop) =][=
`columns --sep=, -c6 --indent=4 --input=$file
set -- \`sum $file\`
cat <<_EOF_
};
#endif /* LOAD_FUNCTIONS */

#define FUNCTION_CKSUM \`echo $1 | sed 's/^0*//'\`
_EOF_
rm -f $file` =]

#ifdef DEFINE_LOAD_FUNCTIONS
/*
 *  Source in each file that contains load functions
 *  This is used by the binary template library builder.
 */[=
FOR infile=]
#include "[=% infile `echo %s|sed 's;^\\./;;'`=]"[=
ENDFOR infile=]
#endif /* DEFINE_LOAD_FUNCTIONS */

#ifdef HANDLE_FUNCTIONS /* tpProcess.c use only * * * * * * * * * * * * * *

 *  Template Processing Function Table
 *
 *  Pointers to the procedure to call when the function code
 *  is encountered.
 */
static tpHdlrProc  apHdlrProc[ FUNC_CT ] = {[=
FOR macfunc "," =]
    /* [=% name "%-10s"=]*/ &mFunc_[=
  IF (exist? "handler_proc") 
     =][=% name (string-capitalize! "%s") =][=
  ELSE
     =]Bogus[=
  ENDIF =][=
ENDFOR macfunc =]
};

#endif /* HANDLE_FUNCTIONS */[=
 #

end of functions.tpl =]
