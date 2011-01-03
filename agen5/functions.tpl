[= AutoGen5 template h   -*- Mode: C -*-


#  Time-stamp:        "2010-08-06 09:00:48 bkorb"

##
## This file is part of AutoGen.
## AutoGen Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
##

=]
[=
(define decl-list "")
(define temp-txt  "")
(define func-name "")
(define func-str-off 0)

(dne " *  " "/*  ")=]
 *
 *  Tables of Text Functions for AutoGen
 *
 *  copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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
 *  C - in context.  May be explicitly invoked in certain situations.
 *      For example, "ELSE" may only be specified within an "IF" block.
 *      Their load procedures are enabled by the block macro (e.g. IF),
 *      and disabled by the block ending macro (e.g. ENDIF).
 *      While disabled, the load procedure is the "Bogus" method.
 *
 *      If a function is neither has a special load procedure nor is
 *      situational, then the "Unknown" load method is applied.
 *
 *  R - has a special remove (unload) procedure
 *
 *  H - has a handler procedure defined.  Only these procedures should
 *      be encountered by the dispatcher during processing.
 *[=
FOR macfunc =]
 *  [=

  IF (exist? "alias")          =]A[=
    IF (exist? "unnamed") =][=
      ERROR % name
         "The %s function is unnamed, but has aliases?!?!" =][=
    ENDIF =][=
  ELIF (exist? "unnamed")      =]U[=
  ELSE                         =] [=
  ENDIF =] [=

  IF   (exist? "load-proc")    =]L[=
  ELIF (exist? "in-context")   =]C[=
    IF (exist? "handler-proc") =][=
      ERROR % name
         "The %s function is situational and has a handler" =][=
    ENDIF =][=
  ELSE                         =] [=
  ENDIF =] [=

  (if (exist? "unload-proc") "R" " ") =][=

  IF (exist? "handler-proc")   =]H[=
  ELSE                         =] [=
  ENDIF =] - [=

  % name (string-upcase! "%-12s") =][=what=][=
ENDFOR macfunc =]
 */
[= (make-header-guard "autogen") =]

#define FUNC_CT    [= (count "macfunc") =]

extern char const * const apzFuncNames[ FUNC_CT ];

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

    FTYP_SELECT_MATCH_ANYTHING        = 0x801C,  /*  *   */
    FTYP_SELECT_MATCH_EXISTENCE       = 0x801D,  /*  @   */
    FTYP_SELECT_MATCH_NONEXISTENCE    = 0x801E   /* !@   */
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

/* tpParse.c use only * * * * * * * * * * * * * * * *
 *
 *  Parsing function tables for load processing (template scanning phase)
 */
tpLoadProc apLoadProc[ FUNC_CT ] = {[=
FOR macfunc "," =]
    /* [=% name "%-10s" =]*/ mLoad_[=
  IF   (> (len "load_proc") 0)=][=% load_proc (string-capitalize! "%s") =][=
  ELIF (exist? "load_proc")   =][=% name (string-capitalize! "%s") =][=
  ELIF (exist? "in-context")  =]Bogus   /*dynamic*/[=
  ELSE                        =]Unknown /*default*/[=
  ENDIF =][=
ENDFOR macfunc =]
};

/*
 *  This global pointer is used to switch parsing tables.
 *  The block functions (CASE, DEFINE, FOR, and IF) change this to point
 *  to their tables that include relevant additional functions.
 */
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
};

/*
 *  Define all the strings that are used to determine the function enumeration
 *  number.  These are used in a table separated by aliases and sorted by these
 *  ASCII values.
 */[=
 (set! func-name "")
 (set! decl-list "") =][=

FOR macfunc =][=
  IF (not (exist? "unnamed")) =][=
    IF (exist? "alias")       =][=
      FOR alias

=][=

        (set! func-name (string-append func-name
		 "\"" (if (== (get "alias") "\"") "\\\""
		       (get "alias")) "\\0\"\n"))

        (set! decl-list (string-append decl-list
              (get "alias") " ::    "
              (sprintf "{  1, zFnStrg +%3d, FTYP_%s },\n"
                  func-str-off (string-upcase! (get "name"))  )  ))

        (set! func-str-off (+ func-str-off 2)) =][=

      ENDFOR alias            =][=

    ELSE                      =][=
      (set! temp-txt (string-upcase! (get "name")))
      (set! func-name (string-append func-name "\"" temp-txt "\\0\"\n"))
      (set! decl-list (string-append decl-list
            temp-txt " ::    "
            (sprintf "{ %2d, zFnStrg +%3d, FTYP_%s },\n" (len "name")
                     func-str-off temp-txt  )  ))
      (shellf "%s=%d" temp-txt func-str-off)
      (set! func-str-off (+ func-str-off (len "name") 1))

      =][=

    ENDIF                     =][=
  ENDIF                       =][=
ENDFOR macfunc

=][=

 (shellf "file=%s.tmp ; cat > ${file} <<\\_EOF_\n%s_EOF_" (out-name) decl-list)
 (emit (sprintf "\ntSCC zFnStrg[%d] =\n" func-str-off))
 (shellf "columns -I4 --spread=1<<\\_EOF_\n%s_EOF_" func-name)

=];

/*
 *  The number of names by which the macros go.
 *  Some have multiple names (aliases, e.g. selection clauses).
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

/* * * * * * * * tpParse.c use only * * * * * * * * * * * * * * *
 *
 *  And now, the table separated by aliasing and then sorted by string content
 */
tNameType nameTypeTable[ FUNCTION_NAME_CT ] = {
_EOF_
egrep -v '^[A-Z]' $file | sort | sed -e 's/^.*:://'
echo
egrep    '^[A-Z]' $file | sort | sed -e 's/^.*:://' -e '$s/,$//'

rm -f $file ` =] };

char const * const apzFuncNames[ FUNC_CT ] = {
[=(out-push-new) =][=

FOR macfunc "\n"    =]echo [=

  (if (exist? "unnamed")
      (string-append "\\\"" (string-capitalize! (get "name")) "\\\"")
      (if (exist? "alias")
          (string-append "\\\"" (string-upcase! (get "name")) "\\\"")
          (sprintf "zFnStrg+${%1$s:-%s}" (string-upcase! (get "name")))
  )   )

=][=

ENDFOR macfunc      =][=

(shell (string-append "( " (out-pop #t) " ) | columns -I4 -S, --spread=1"))

=] };

[=

(shellf
"set -- `sum %s`
sum=`printf '((unsigned short)0x%%04X)' $1`
echo \"#define FUNCTION_CKSUM ${sum}\"
rm -f $file"
(out-name)) =]

/* * * * * * * * tpProcess.c use only * * * * * * * * * * * * * *
 *
 *  Template Processing Function Table
 *
 *  Pointers to the procedure to call when the function code
 *  is encountered.
 */
static tpHdlrProc  apHdlrProc[ FUNC_CT ] = {[=
FOR macfunc "," =]
    /* [=% name "%-10s"=]*/ mFunc_[=
  IF (exist? "handler_proc")
     =][=% name (string-capitalize! "%s") =][=
  ELSE
     =]Bogus[=
  ENDIF =][=
ENDFOR macfunc =]
};

/* * * * * * * * * * tpLoad.c use only * * * * * * * * * * * * * *
 *
 *  Template Unloading Function Table
 *
 *  Pointers to the procedure to call when the function code
 *  is encountered in a template being unloaded.
 */[=
    (set! decl-list "") (set! temp-txt "") =][=

FOR macfunc =][=
    (set! func-name (string-append
          "mUnload_" (string-capitalize (get "name"))))

    (if (exist? "unload-proc")
        (begin
	  (set! decl-list (string-append decl-list func-name ", "))
	  (set! temp-txt  (string-append temp-txt  func-name "\n"))
	)
        (set! temp-txt (string-append temp-txt "NULL\n"))
    ) =][=
ENDFOR =]

tUnloadProc [= (shellf "echo '%s'|sed 's/, $//'" decl-list) =];

static tpUnloadProc  apUnloadProc[ FUNC_CT ] = {
[= (shellf "columns -I4 --sep=, --spread=1<<_EOF_\n%s_EOF_" temp-txt) =]
};[=

 #

end of functions.tpl =]
#endif /* [= (. header-guard) =] */
