[= AutoGen5 Template Library -*- Mode: C -*-

# $Id: optlib.tpl,v 1.15 2000/10/16 00:07:25 bkorb Exp $

=]
[=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Emit the "#define SET_OPT_NAME ..." and "#define DISABLE_OPT_NAME ..."

=][=

DEFINE set_defines set_desc set_index opt_state =]
#define SET_[=(. opt-name)=][=
  IF (exist? "flag_arg")=](a)[=ENDIF=]   STMTS( \
        [=set_desc=].optActualIndex = [=(for-index)=]; \
        [=set_desc=].optActualValue = VALUE_[=(. opt-name)=]; \
        [=set_desc=].fOptState &= OPTST_PERSISTENT; \
        [=set_desc=].fOptState |= [=opt_state=][=
  IF (exist? "flag_arg")=]; \
        [=set_desc=].pzLastArg  = [=
    IF (==* (get "flag_arg") "=") =](char*)atoi[=
    ENDIF=](a)[=
  ENDIF flag_arg-exists =][=
  IF (or (exist? "call_proc")
     (or (exist? "flag_code")
     (or (exist? "flag_proc")
         (exist? "stack_arg") ))) =]; \
        (*([=(. descriptor)=].pOptProc))( &[=
                           (. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=set_index=] )[=
  ENDIF "callout procedure exists" =] )[=

  IF (exist? "disable") =]
#define DISABLE_[=(. opt-name)=]   STMTS( \
        [=set_desc=].fOptState &= OPTST_PERSISTENT; \
        [=set_desc=].fOptState |= OPTST_SET | OPTST_DISABLED; \
        [=set_desc=].pzLastArg  = (char*)NULL[=
    IF (or (exist? "call_proc")
       (or (exist? "flag_code")
       (or (exist? "flag_proc")
           (exist? "stack_arg") ))) =]; \
        (*([=(. descriptor)=].pOptProc))( &[=
                  (. pname)=]Options, \
                [=(. pname)=]Options.pOptDesc + [=set_index=] )[=
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
     (. prog-name) (get "copyright.date") (get "copyright.owner") ) =][=

  CASE (get "copyright.type") =][=

    =  gpl  =]
 *
[=(gpl (. prog-name) " * " ) =][=

    = lgpl  =]
 *
[=(lgpl (. prog-name) (get "copyright.owner") " * " ) =][=

    =  bsd  =]
 *
[=(bsd  (. prog-name) (get "copyright.owner") " * " ) =][=

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

DEFINE Option_Defines =]
#define VALUE_[=(sprintf "%-18s" opt-name)=] [=
     IF (exist? "value") =][=
        CASE (get "value") =][=
        =   number  =]NUMBER_OPTION[=
        ==  '       =]'\''[=
        ~   .       =]'[=value=]'[=
        *           =][=ERROR (sprintf
          "Error:  value for opt %s is `%s'\nmust be single char or 'NUMBER'"
          (get "name") (get "value")) =][=
        ESAC =][=
     ELIF (<= (for-index) 32) =][= (for-index) =][=
     ELSE                 =][= (+ (for-index) 96) =][=
     ENDIF =][=
  IF (==* (get "flag_arg") "=")
=]
#define [=(. UP-prefix)=]OPT_VALUE_[=(sprintf "%-14s" UP-name)
                =] (*(long*)(&[=(. UP-prefix)=]OPT_ARG([=(. UP-name)=])))[=
  ENDIF=][=
  IF (= (string-upcase! (get "equivalence")) UP-name) =]
#define WHICH_[=(sprintf "%-18s" opt-name)
                =] ([=(. descriptor)=].optActualValue)
#define WHICH_[=(. UP-prefix)=]IDX_[=(sprintf "%-14s" UP-name)
                =] ([=(. descriptor)=].optActualIndex)[=
  ENDIF =][=
  IF (exist? "settable") =][=

    IF  (or (not (exist? "equivalence"))
            (= (get "equivalence") UP-name)) =][=

      set_defines
           set_desc  = (string-append UP-prefix "DESC(" UP-name ")" )
           set_index = (for-index)
           opt_state = OPTST_SET =][=

    ELSE "not equivalenced"   =][=
      set_defines
           set_desc  = (string-append UP-prefix "DESC("
                           (string-upcase! (get "equivalence")) ")" )
           set_index = (string-append "INDEX_" UP-prefix "OPT_"
                           (string-upcase! (get "equivalence")) )
           opt_state = "OPTST_SET | OPTST_EQUIVALENCE" =][=

    ENDIF is/not equivalenced =][=

  ENDIF settable =][=

ENDDEF
=][=

# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Define the arrays of values associated with an option (strings, etc.) =][=

DEFINE Option_Strings =][=

(set! cap-name (string-capitalize! (get "name")))
=]
/*
 *  [=(. cap-name)=] option description[=
  IF (or (exist? "flags_must") (exist? "flags_cant")) =] with
 *  "Must also have options" and "Incompatible options"[=
  ENDIF =]:
 */
tSCC    z[=(. cap-name)=]Text[] =
        [=(kr-string (get "descrip"))=];[=

  IF (not (exist? "documentation")) =][=
  #
  #  This is *NOT* a documentation option: =]
tSCC    z[= (sprintf "%-25s" (string-append cap-name
                    "_NAME[]" )) =] = "[=(string-upcase! (get "name"))=]";[=

    #  IF this option can be disabled,
    #  THEN we must create the string for the disabled version
    #  =][=
    IF (> (len "disable") 0) =]
tSCC    zNot[= (sprintf "%-23s" (string-append cap-name "_Name[]"))
             =]= "[=(string-downcase! (get "disable"))=]-[=
          (shell (sprintf "echo '%s'|tr '_\^' '\\-\\-'"
                        (string-downcase! (get "name")) )) =]";
tSCC    zNot[= (sprintf "%-23s" (string-append cap-name "_Pfx[]"))
             =]= "[=(string-downcase! (get "disable"))=]";[=


      #  See if we can use a substring for the option name:
      #  =][=
      IF (> (len "enable") 0) =]
tSCC    z[=    (sprintf "%-26s" (string-append cap-name "_Name[]")) =]= "[=
         (shell (sprintf "echo '%s-%s'|tr '_\^' '\\-\\-'"
                        (string-downcase! (get "enable"))
                        (string-downcase! (get "name")) )) =]";[=
      ELSE =]
#define z[=(sprintf "%-28s" (string-append cap-name
        "_Name")) =](zNot[= (. cap-name) =]_Name + [=
        (+ (string-length (get "disable")) 1 ) =])[=
      ENDIF =][=


    ELSE  No disablement of this option:
    =]
#define zNot[= (sprintf "%-24s" (string-append cap-name "_Pfx"))
             =] (const char*)NULL
#define zNot[= (sprintf "%-24s" (string-append cap-name "_Name"))
             =] (const char*)NULL
tSCC    z[=    (sprintf "%-26s" (string-append cap-name "_Name[]"))
             =]= "[=enable 
         (string-append (string-downcase! (get "enable")) "-") =][=
         (string-tr! (string-downcase! (get "name")) "_^" "--" ) =]";[=

    ENDIF (> (len "disable") 0) =][=

    #  Check for special attributes:  a default value
    #  and conflicting or required options
    =][=
    CASE (get "flag_arg") =][=
    ~~  ^[:= ]{0,1}$ =][= # No initial value   =][=
    ~~* =[0-9]       =][= # Numeric init value =]
#define z[=(sprintf "%-28s" (string-append cap-name "DefaultArg" ))
         =]((tCC*)[=(shell (sprintf "echo %s | sed 's@^=@@'"
                         (get "flag_arg") ))=])[=

    ==*  :         =][= # String init value =]
tSCC    z[=(sprintf "%-28s" (string-append cap-name "DefaultArg[]" ))
         =]= [=(kr-string (shell (sprintf "echo '%s' | sed 's@^:@@'"
                         (get "flag_arg") ))) =];[=

    *              =][= # String init value =]
tSCC    z[=(sprintf "%-28s" (string-append cap-name "DefaultArg[]" ))
         =]= [=(kr-string (get "flag_arg"))=];[=
    ESAC =][=


    IF (exist? "flags_must") =]
static const int
    a[=(. cap-name)=]MustList[] = {[=
      FOR flags_must =]
    INDEX_[= (. UP-prefix) =]OPT_[= (string-upcase! (get "flags_must")) =],[=
      ENDFOR flags_must =] NO_EQUIVALENT };[=
    ENDIF =][=


    IF (exist? "flags_cant") =]
static const int
    a[=(. cap-name)=]CantList[] = {[=
      FOR flags_cant =]
    INDEX_[= (. UP-prefix) =]OPT_[= (string-upcase! (get "flags_cant")) =],[=
      ENDFOR flags_cant =] NO_EQUIVALENT };[=
    ENDIF =][=
  ENDIF (not (exist? "documentation")) =][=

ENDDEF Option_Strings =][=


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Define the values for an option descriptor   =][=

DEFINE Option_Descriptor =][=
    (set! up-name  (string-upcase!     (get "name")))
    (set! cap-name (string-capitalize! (get "name"))) =][=

  IF (exist? "documentation") =]
  {  /* entry idx, value */ 0, 0,
     /* equiv idx, value */ 0, 0,
     /* option argument  */ ARG_NONE,
     /* equivalenced to  */ NO_EQUIVALENT,
     /* min, max, act ct */ 0, 0, 0,
     /* opt state flags  */ OPTST_DOCUMENT | OPTST_NO_INIT,
     /* last opt argumnt */ (char*)NULL,
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ (const int*)NULL,  (const int*)NULL,
     /* option proc      */ [=
         IF   (exist? "call_proc") =][=(get "call_proc")=][=
         ELIF (exist? "flag_code") =]doOpt[=(. cap-name)=][=
         ELSE                      =](tpOptProc)NULL[=
         ENDIF =],
     /* desc, NAME, name */ z[=(. cap-name)=]Text, (const char*)NULL,
                            (const char*)NULL,
     /* disablement strs */ (const char*)NULL, (const char*)NULL },[=

  ELSE

=]
  {  /* entry idx, value */ [=(for-index)=], VALUE_[=
                              (. UP-prefix)=]OPT_[=(. up-name)=],
     /* equiv idx, value */ [=
          IF (== (string-upcase! (get "equivalence")) up-name)
              =]NO_EQUIVALENT, 0,[=
          ELIF (exist? "equivalence")
              =]~0, ~0,[=
          ELSE
              =][=(for-index)=], VALUE_[=(. UP-prefix)=]OPT_[=(. up-name)=],[=
          ENDIF=]
     /* option argument  */ [=
         IF   (== (get "value") "NUMBER")  =]ARG_MUST[=
         ELIF (not (exist? "flag_arg"))    =]ARG_NONE[=
         ELIF (> (len "flag_arg") 0)       =]ARG_MUST[=
         ELSE                              =]ARG_MAY[=
         ENDIF =],
     /* equivalenced to  */ [=
         IF (and (exist? "equivalence")
                 (not (string-ci=? (string-upcase! (get "equivalence"))
                                   (. up-name)) ) )
               =]INDEX_[=(. UP-prefix)=]OPT_[=(string-upcase!
                         (get "equivalence"))=][=
         ELSE  =]NO_EQUIVALENT[=
         ENDIF =],
     /* min, max, act ct */ [=(if (exist? "min") (get "min") "0")=], [=
         (if (exist? "max") (get "max") "1")=], 0,
     /* opt state flags  */ [=
         IF (==* (get "flag_arg") "=")=]OPTST_NUMERIC | [=     ENDIF=][=
         IF (exist? "disable")        =]OPTST_DISABLEOK | [=   ENDIF=][=
         IF (exist? "stack_arg")      =]OPTST_STACKED | [=     ENDIF=][=
         IF (not (exist? "enabled"))  =]OPTST_DISABLED | [=
         ELSE                         =]OPTST_INITENABLED | [= ENDIF=][=
         IF (exist? "no_preset")      =]OPTST_NO_INIT[=
         ELSE                         =]OPTST_INIT[=           ENDIF=],
     /* last opt argumnt */ (char*)[=

         CASE (get "flag_arg") =][=
         ~~ ^[:= ]{0,1}$ =]NULL[=
         *               =]z[=(. cap-name)=]DefaultArg[=
         ESAC =],
     /* arg list/cookie  */ (void*)NULL,
     /* must/cannot opts */ [=
         IF (exist? "flags_must")=]a[=(. cap-name)=]MustList[=
         ELSE                    =](const int*)NULL[=
         ENDIF=], [=
         IF (exist? "flags_cant")=]a[=(. cap-name)=]CantList[=
         ELSE                    =](const int*)NULL[=
         ENDIF=],
     /* option proc      */ [=
         IF   (exist? "call_proc") =][=call_proc=][=
         ELIF (exist? "flag_code") =]doOpt[=(. cap-name)=][=
         ELIF (exist? "flag_proc") =]doOpt[=
              (string-capitalize! (get "flag_proc")) =][=

         ELIF (exist? "stack_arg") =][=
               IF (or (not (exist? "equivalence"))
                      (= (get "equivalence") (get "name")) )
                      =]stackOptArg[=
               ELSE  =]unstackOptArg[=
               ENDIF=][=

         ELSE =](tpOptProc)NULL[=
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
Finally, if there are no minimum occurrance counts (i.e. all
options are optional), then we put square brackets around the
syntax. =][=

DEFINE USAGE_LINE   =][=

  #  Compute the option arguments           =][=

  (out-push-new ".usAGe_line")

=][=(. prog-name)=] - [=prog_title=][= % version " - Ver. %s" =]
USAGE:  %s [=
  IF  (exist? "flag.flag_arg")              =][=
      (define flag-arg " [<val>]")
      (define  opt-arg "[{=| }<val>]")      =][=
  ELSE                                      =][=
      (define flag-arg "")
      (define  opt-arg "")                  =][=
  ENDIF                                     =][=

  IF (not (exist? "flag.min")) =][[= ELSE =]{[= ENDIF =] [=

  #  At least one option has a minimum occurrance count.
     Therefore, we omit the square brackets around the option
     syntax.
  =][=
  IF   (exist? "flag.value")
     =]-<flag>[=(. flag-arg)=][=

     IF (exist? "long_opts")
        =] | [=ENDIF=][=
  ELIF (not (exist? "long_opts")) =]<option-name>[=(. opt-arg)=][=
  ENDIF                                     =][=

  IF (exist? "long_opts")
     =]--<name>[=(. opt-arg)                =][=
  ENDIF                                     =] [=

  IF (not (exist? "flag.min")) =]][= ELSE =]}[= ENDIF =]...[=

  IF (exist? "argument")                    =] [=

    #  IF the USAGE line plus the program name plus the argument
    #  goes past 80 columns, then break the line.  =][=

    `f=\`(cat .usAGe_line ; echo) | sed -n -e 2p -e 2q | wc -c\`` =][=

    IF (< 82 (+ (string->number (shell "echo $f"))
                (len "argument")
                (len "prog_name") )) =]\
		[=
    ENDIF

    =][=argument=]
[=
  ENDIF=][=

(out-pop)
(kr-string (string-append (shell
"cat .usAGe_line ; rm -f .usAGe_line") "\n" )) =][=

ENDDEF
=]
