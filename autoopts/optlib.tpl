[= AutoGen5 Template Library -*- Mode: Text -*-

# $Id: optlib.tpl,v 3.17 2004/01/14 02:41:16 bkorb Exp $

# Automated Options copyright 1992-2003 Bruce Korb

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Emit the "#define SET_OPT_NAME ..." and "#define DISABLE_OPT_NAME ..."

=][=

DEFINE set-defines set-desc set-index opt-state =]
#define SET_[=(. opt-name)=][=
  IF (exist? "arg-type")=](a)[=ENDIF=]   STMTS( \
        [=set-desc=].optActualIndex = [=(for-index)=]; \
        [=set-desc=].optActualValue = VALUE_[=(. opt-name)=]; \
        [=set-desc=].fOptState &= OPTST_PERSISTENT; \
        [=set-desc=].fOptState |= [=opt-state=][=
  IF (exist? "arg-type")=]; \
        [=set-desc=].pzLastArg  = (tCC*)(a)[=
  ENDIF  =][=
  IF (or (exist? "call-proc")
         (exist? "flag-code")
         (exist? "extract-code")
         (exist? "flag-proc")
         (exist? "arg-range")
         (exist? "stack_arg")
         (~* (get "arg-type") "set|key|num|bool" ) )   =]; \
        (*([=(. descriptor)=].pOptProc))( &[=
                           (. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=set-index=] )[=
  ENDIF "callout procedure exists" =] )[=

  IF (exist? "disable") =]
#define DISABLE_[=(. opt-name)=]   STMTS( \
        [=set-desc=].fOptState &= OPTST_PERSISTENT; \
        [=set-desc=].fOptState |= OPTST_SET | OPTST_DISABLED; \
        [=set-desc=].pzLastArg  = NULL[=
    IF (or (exist? "call-proc")
           (exist? "flag-code")
           (exist? "extract-code")
           (exist? "flag-proc")
           (exist? "arg-range")
           (exist? "stack-arg") ) =]; \
        (*([=(. descriptor)=].pOptProc))( &[=
                  (. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=set-index=] )[=
    ENDIF "callout procedure exists" =] )[=

  ENDIF disable exists =][=

ENDDEF
=][=

  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

  Emit the copyright comment

  =][=

DEFINE Option_Copyright =][=
IF (exist? "copyright") =]
/*
 * [=(sprintf "%s copyright %s %s - all rights reserved"
     prog-name (get "copyright.date") (get "copyright.owner") ) =][=

  CASE (get "copyright.type") =][=

    =  gpl  =]
 *
[=(gpl  prog-name " * " ) =][=

    = lgpl  =]
 *
[=(lgpl prog-name (get "copyright.owner") " * " ) =][=

    =  bsd  =]
 *
[=(bsd  prog-name (get "copyright.owner") " * " ) =][=

    = note  =]
 *
[=(prefix " * " (get "copyright.text"))=][=

    *       =] * <<indeterminate copyright type>>[=

  ESAC =]
 */[=
ENDIF "copyright exists" =][=
ENDDEF

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Emit the #define's for a single option

=][=

DEFINE Option_Defines      =][=
  IF (. ifdef-ed)          =][=
    IF   (exist? "ifdef")  =]
#ifdef [=(get "ifdef")     =][=
    ELIF (exist? "ifndef") =]
#ifndef [=(get "ifndef")   =][=
    ENDIF ifdef/ifndef     =][=
  ENDIF =][=

  CASE (get "arg-type")    =][=

  =*   key                 =]
typedef enum {[=
         IF (not (exist? "arg-default")) =] [=
            (string-append UP-prefix UP-name)=]_UNDEFINED = 0,[=
         ENDIF  =]
[=(shellf "for f in %s ; do echo %s_${f} ; done | \
          ${COLUMNS_EXE} -I4 --spread=3 --sep=','"
          (string-upcase! (string->c-name! (join " " (stack "keyword"))))
          (string-append UP-prefix UP-name) )=]
} te_[=(string-append Cap-prefix cap-name)=];[=

  =*   set                 =][=
    (define setmember-fmt (string-append "\n#define %-24s 0x%0"
       (shellf "expr '(' %d + 3 ')' / 4" (count "keyword")) "XUL"
       (if (> (count "keyword") 32) "L" "")  ))
    (define full-prefix (string-append UP-prefix UP-name) )  =][=

    FOR    keyword   =][=

      (sprintf setmember-fmt
         (string->c-name! (string-append
             full-prefix "_" (string-upcase! (get "keyword")) ))
         (ash 1 (for-index))  ) =][=

    ENDFOR keyword   =][=

  ESAC  (get "arg-type")

=]
#define VALUE_[=(sprintf "%-18s" opt-name)=] [=
     IF (exist? "value") =][=
        CASE (get "value") =][=
        ==  '       =]'\''[=
        ~   .       =]'[=value=]'[=
        *           =][=(error (sprintf
          "Error:  value for opt %s is `%s'\nmust be single char or 'NUMBER'"
          (get "name") (get "value")))=][=
        ESAC =][=
     ELIF (<= (for-index) 32) =][= (for-index) =][=
     ELSE                 =][= (+ (for-index) 96) =][=
     ENDIF =][=

  CASE arg-type  =][=

  ~*  num        =]
#define [=(. OPT-pfx)=]VALUE_[=(sprintf "%-14s" UP-name)
                 =] (*(unsigned long*)(&[=(. OPT-pfx)=]ARG([=(. UP-name)=])))[=

  =*  key        =]
#define [=(. OPT-pfx)=]VALUE_[=(sprintf "%-14s" UP-name)
                 =] (*(te_[=(string-append Cap-prefix cap-name)
                          =]*)(&[= (. OPT-pfx) =]ARG([=(. UP-name)=])))[=

  =*  set        =]
#define [=(sprintf
   "%1$sVALUE_%2$-14s (uintptr_t)(%3$sDESC(%2$s).optCookie)"
   OPT-pfx UP-name UP-prefix)
                 =][=

  =*  bool       =]
#define [=(. OPT-pfx)=]VALUE_[=(sprintf "%-14s" UP-name)
                 =] (*(ag_bool*)(&[=(. OPT-pfx)=]ARG([=(. UP-name)=])))[=

  ESAC           =][=
  IF (== (up-c-name "equivalence") UP-name) =]
#define WHICH_[=(sprintf "%-18s" opt-name)
                =] ([=(. descriptor)=].optActualValue)
#define WHICH_[=(. UP-prefix)=]IDX_[=(sprintf "%-14s" UP-name)
                =] ([=(. descriptor)=].optActualIndex)[=
  ENDIF =][=
  IF (exist? "settable") =][=

    IF  (or (not (exist? "equivalence"))
            (== (up-c-name "equivalence") UP-name)) =][=

      set-defines
           set-desc  = (string-append UP-prefix "DESC(" UP-name ")" )
           set-index = (for-index)
           opt-state = OPTST_SET =][=

    ELSE "not equivalenced"   =][=
      set-defines
           set-desc  = (string-append UP-prefix "DESC("
                           (up-c-name "equivalence") ")" )
           set-index = (index-name "equivalence")
           opt-state = "OPTST_SET | OPTST_EQUIVALENCE" =][=

    ENDIF is/not equivalenced =][=

  ENDIF settable            =][=
  IF (. ifdef-ed)       =][=
    IF (exist? "ifdef")     =]
#endif /* [=    ifdef       =][=
    ELIF (exist? "ifndef")  =]
#endif /* ! [=  ifndef      =][=
    ENDIF =] */[=
  ENDIF =][=

ENDDEF
=][=

# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Define the arrays of values associated with an option (strings, etc.) =][=

IF (exist? "preserve-case")     =][=
  (define optname-from "_^")
  (define optname-to   "--")    =][=
ELSE                            =][=
  (define optname-from "A-Z_^")
  (define optname-to   "a-z--") =][=
ENDIF                           =][=

DEFINE   emit-nondoc-option     =][=
  #
  #  This is *NOT* a documentation option: =]
tSCC    z[= (sprintf "%-25s" (string-append cap-name
                    "_NAME[]" )) =] = "[=(. UP-name)=]";[=

    #  IF this option can be disabled,
    #  THEN we must create the string for the disabled version
    #  =][=
    IF (> (len "disable") 0) =]
tSCC    zNot[= (sprintf "%-23s" (string-append cap-name "_Name[]"))
             =]= "[=
       (string-tr! (string-append (get "disable") "-" flg-name)
                   optname-from optname-to) =]";
tSCC    zNot[= (sprintf "%-23s" (string-append cap-name "_Pfx[]"))
             =]= "[=(string-downcase! (get "disable"))=]";[=


      #  See if we can use a substring for the option name:
      #  =][=
      IF (> (len "enable") 0) =]
tSCC    z[=(sprintf "%-26s" (string-append cap-name "_Name[]")) =]= "[=
        (string-tr! (string-append (get "enable") "-" flg-name)
                    optname-from optname-to) =]";[=
      ELSE =]
#define z[=(sprintf "%-27s " (string-append cap-name
        "_Name")) =](zNot[= (. cap-name) =]_Name + [=
        (+ (string-length (get "disable")) 1 ) =])[=
      ENDIF =][=


    ELSE  No disablement of this option:
    =]
#define zNot[= (sprintf "%-24s" (string-append cap-name "_Pfx"))
             =] NULL
#define zNot[= (sprintf "%-24s" (string-append cap-name "_Name"))
             =] NULL
tSCC    z[=    (sprintf "%-26s" (string-append cap-name "_Name[]"))
             =]= "[= (string-tr! (string-append
        (if (exist? "enable") (string-append (get "enable") "-") "")
        (get "name"))   optname-from optname-to) =]";[=

    ENDIF (> (len "disable") 0) =][=

    #  Check for special attributes:  a default value
    #  and conflicting or required options
    =][=
    IF (define def-arg-name (sprintf "z%-27s "
                 (string-append cap-name "DefaultArg" )))
       (define def-arg-array (sprintf "z%-27s "
                 (string-append cap-name "DefaultArg[]" )))
       (exist? "arg-default")   =][=
       CASE arg-type            =][=
       =* num                   =]
#define [=(. def-arg-name)=]((tCC*)[= arg-default =])[=

       =* bool                  =][=
          CASE arg-default      =][=
          ~ n.*|f.*|0           =]
#define [=(. def-arg-name)=]((tCC*)((tCC*)AG_FALSE)[=
          *                     =]
#define [=(. def-arg-name)=]((tCC*)AG_TRUE)[=
          ESAC                  =][=

       =* key                   =]
#define [=(. def-arg-name)=]((tCC*)[=
          IF (=* (get "arg-default") (string-append Cap-prefix cap-name))
            =][= arg-default    =][=
          ELSE  =][=(string-append UP-prefix UP-name "_"
                    (up-c-name "arg-default")) =][=
          ENDIF =])[=

       =* set                   =]
#define [=(. def-arg-name)=]NULL
#define [=(sprintf "%-28s " (string-append cap-name "CookieBits"))=](void*)([=
         IF (not (exist? "arg-default")) =]0[=
         ELSE =][=
           FOR    arg-default | =][=
             (string->c-name! (string-append UP-prefix UP-name "_"
                   (string-upcase! (get "arg-default")) ))  =][=
           ENDFOR arg-default   =][=
         ENDIF =])[=

       =* str                   =]
tSCC    [=(. def-arg-array)=]= [=(kr-string (get "arg-default"))=];[=

       *                        =][=
          (error (string-append cap-name
                 " has arg-default, but no valid arg-type"))  =][=
       ESAC                     =][=
    ENDIF                       =][=


    IF (exist? "flags_must") =]
static const int
    a[=(. cap-name)=]MustList[] = {[=
      FOR flags-must =]
    [= (index-name "flags-must") =],[=
      ENDFOR flags_must =] NO_EQUIVALENT };[=
    ENDIF =][=


    IF (exist? "flags-cant") =]
static const int
    a[=(. cap-name)=]CantList[] = {[=
      FOR flags-cant =]
    [= (index-name "flags-cant") =],[=
      ENDFOR flags-cant =] NO_EQUIVALENT };[=
    ENDIF =]
#define [=(. UP-name)=]_FLAGS       ([=
         CASE arg-type  =][=
         =*   num       =]OPTST_NUMERIC | [=
         =*   bool      =]OPTST_BOOLEAN | [=
         =*   key       =]OPTST_ENUMERATION | [=
         =*   set       =]OPTST_MEMBER_BITS | [=
         ESAC           =][=
         stack-arg      "OPTST_STACKED | "     =][=
         immediate      "OPTST_IMM | "         =][=
         immed-disable  "OPTST_DISABLE_IMM | " =][=
         must-set       "OPTST_MUST_SET | "    =][=
         ? enabled      "OPTST_INITENABLED"
                        "OPTST_DISABLED"       =][=
         no-preset      " | OPTST_NO_INIT"     =])[=
ENDDEF   emit-nondoc-option     =][=

# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Define the arrays of values associated with an option (strings, etc.) =][=

DEFINE   Option_Strings

=]
/*
 *  [=(. cap-name)=] option description[=
  IF (or (exist? "flags_must") (exist? "flags_cant")) =] with
 *  "Must also have options" and "Incompatible options"[=
  ENDIF =]:
 */[=

  IF (. ifdef-ed)           =][=
    IF (exist? "ifdef")     =]
#ifdef [=  ifdef            =][=
    ELSE                    =]
#ifndef [= ifndef           =][=
    ENDIF ifdef/ifndef      =][=
  ENDIF =]
tSCC    z[=(. cap-name)=]Text[] =
        [=(set! tmp-text (kr-string (get "descrip")))  tmp-text=];[=

  IF (exist? "documentation")     =]
#define [=(. UP-name)=]_FLAGS       (OPTST_DOCUMENT | OPTST_NO_INIT)[=
  ELSE  NOT a doc option:         =][=
     emit-nondoc-option           =][=
  ENDIF  (exist? "documentation") =][=

  IF (. ifdef-ed)   =]

#else   /* disable [=(. cap-name)=] */
#define VALUE_[=(string-append OPT-pfx UP-name)=] NO_EQUIVALENT
#define [=(. UP-name)=]_FLAGS       (OPTST_OMITTED | OPTST_NO_INIT)[=

    IF (exist? "arg-default") =]
#define z[=(. cap-name)=]DefaultArg NULL[=
    ENDIF =][=

    IF (exist? "flags-must")  =]
#define a[=(. cap-name)=]MustList   NULL[=
    ENDIF =][=

    IF (exist? "flags-cant")  =]
#define a[=(. cap-name)=]CantList   NULL[=
    ENDIF =]
#define z[=(. cap-name)=]Text       NULL
#define z[=(. cap-name)=]_NAME      NULL
#define z[=(. cap-name)=]_Name      NULL
#define zNot[=(. cap-name)=]_Name   NULL
#define zNot[=(. cap-name)=]_Pfx    NULL
#endif  /* [= ifdef =][= ifndef =] */[=
  ENDIF (. ifdef-ed)   =][=

ENDDEF Option_Strings =][=


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Define the values for an option descriptor   =][=

DEFINE Option_Descriptor =][=

  IF (exist? "documentation") =]
  {  /* entry idx, value */ 0, 0,
     /* equiv idx, value */ 0, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 0, 0,
     /* opt state flags  */ [=(. UP-name)=]_FLAGS,
     /* last opt argumnt */ NULL,
     /* arg list/cookie  */ NULL,
     /* must/cannot opts */ NULL, NULL,
     /* option proc      */ [=
         IF   (exist? "call-proc")        =][=call-proc=][=
         ELIF (or (exist? "extract-code")
                  (exist? "flag-code"))   =]doOpt[=(. cap-name)=][=
         ELSE                             =]NULL[=
         ENDIF =],
     /* desc, NAME, name */ z[=(. cap-name)=]Text, NULL, NULL,
     /* disablement strs */ NULL, NULL },[=

  ELSE

=]
  {  /* entry idx, value */ [=(for-index)=], VALUE_[=
                              (string-append OPT-pfx UP-name)=],
     /* equiv idx, value */ [=
          IF (== (up-c-name "equivalence") UP-name)
              =]NO_EQUIVALENT, 0,[=
          ELIF (exist? "equivalence")
              =]NOLIMIT, NOLIMIT,[=
          ELSE
              =][=(for-index)=], VALUE_[=(string-append OPT-pfx UP-name)=],[=
          ENDIF=]
     /* option argument  */ ARG_[=
         IF (not (exist? "arg-type"))  =]NONE[=
         ELIF (exist? "arg-optional")  =]MAY[=
         ELSE                          =]MUST[=
         ENDIF =],
     /* equivalenced to  */ [=
         (if (and (exist? "equivalence")
                  (not (== (up-c-name "equivalence") UP-name)) )
             (index-name "equivalence")
             "NO_EQUIVALENT"
         ) =],
     /* min, max, act ct */ [=(if (exist? "min") (get "min") "0")=], [=
         (if (=* (get "arg-type") "set") "NOLIMIT"
             (if (exist? "max") (get "max") "1") ) =], 0,
     /* opt state flags  */ [=(. UP-name)=]_FLAGS,
     /* last opt argumnt */ [=
         IF (exist? "arg-default")
              =]z[=(. cap-name)=]DefaultArg[=
         ELSE =]NULL[= ENDIF =],
     /* arg list/cookie  */ [=
            (if (and (=* (get "arg-type") "set") (exist? "arg-default"))
                (string-append cap-name "CookieBits") "NULL") =],
     /* must/cannot opts */ [=
         (if (exist? "flags-must")
             (string-append "a" cap-name "MustList, ")
             "NULL, " ) =][=
         (if (exist? "flags-cant")
             (string-append "a" cap-name "CantList")
             "NULL" ) =],
     /* option proc      */ [=
         IF   (exist? "call-proc")        =][=call-proc=][=
         ELIF (or (exist? "extract-code")
                  (exist? "flag-code")
                  (exist? "arg-range"))   =]doOpt[=(. cap-name)=][=

         ELIF (exist? "flag-proc") =]doOpt[= (string-capitalize!
                                             (get "flag-proc")) =][=

         ELIF (exist? "stack-arg") =][=
           IF (or (not (exist? "equivalence"))
                  (== (up-c-name "equivalence") UP-name) )

                          =]stackOptArg[=
           ELSE           =]unstackOptArg[=
           ENDIF          =][=

         ELSE             =][=
           CASE arg-type  =][=
           =*   bool      =]optionBooleanVal[=
           =*   num       =]optionNumericVal[=
           ~*   key|set   =]doOpt[=(. cap-name)=][=
           *              =]NULL[=
           ESAC           =][=
         ENDIF=],
     /* desc, NAME, name */ z[=(. cap-name)=]Text,  z[=(. cap-name)=]_NAME,
                            z[=(. cap-name)=]_Name,
     /* disablement strs */ zNot[=(. cap-name)
                            =]_Name, zNot[=(. cap-name)=]_Pfx },[=
  ENDIF =][=

ENDDEF Option_Descriptor =][=


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Compute the usage line.  It is complex because we are trying to
encode as much information as we can and still be comprehensible.

The rules are:  If any options have a "value" attribute, then
there are flags allowed, so include "-<flag>" on the usage line.
If the program has the "long_opts" attribute set, then we must
have "<option-name>" or "--<name>" on the line, depending on
whether or not there are flag options.  If any options take 
arguments, then append "[<val>]" to the flag description and
"[{=| }<val>]" to the option-name/name descriptions.  We won't
worry about being correct if every option has a required argument.
Finally, if there are no minimum occurrence counts (i.e. all
options are optional), then we put square brackets around the
syntax. =][=

DEFINE USAGE_LINE   =][=

  ;;  Compute the option arguments
  ;;
  (if (exist? "flag.arg-type")
      (begin
        (define flag-arg " [<val>]")
        (define  opt-arg "[{=| }<val>]") )
      (begin
        (define flag-arg "")
        (define  opt-arg "") )  )

  (define usage-line (string-append "USAGE:  %s "

    ;; If at least one option has a minimum occurrence count
    ;; we use curly brackets around the option syntax.
    ;;
    (if (not (exist? "flag.min")) "[ " "{ ")

    (if (exist? "flag.value")
        (string-append "-<flag>" flag-arg
           (if (exist? "long-opts") " | " "") )
        (if (not (exist? "long-opts"))
           (string-append "<option-name>" opt-arg) "" )  )

    (if (exist? "long-opts")
        (string-append "--<name>" opt-arg) "" )

    (if (not (exist? "flag.min")) " ]..." " }...")
  ) )

  (if (exist? "argument")

    (set! usage-line (string-append usage-line

          ;; the USAGE line plus the program name plus the argument goes
          ;; past 80 columns, then break the line, else separate with space
          ;;
          (if (< 80 (+ (string-length usage-line)
                (len "argument")
                (string-length prog-name) ))
              " \\\n\t\t"  " ")
          (get "argument")  ))
  )

  (set! tmp-text
  (kr-string (string-append prog-name " - " (get "prog-title")
           (if (exist? "version") (string-append " - Ver. " (get "version"))
               "" )
           "\n" usage-line "\n" )) )

  tmp-text =][=

ENDDEF

=]
