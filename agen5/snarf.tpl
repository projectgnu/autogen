[= AutoGen5 template  -*- Mode: Text -*-

# $Id: snarf.tpl,v 4.2 2005/01/09 04:05:39 bkorb Exp $

(setenv "SHELL" "/bin/sh")

ini =]
[=#

This template will emit the code necessary for registering callout routines
for Guile/Scheme.  The name of the output file will be ``basename.ini''
where ``basename'' may be of your choosing.

The following definitions are used:

group      A module prefix that preceeds the "scm_" prefix to all symbols
init       the name of the created initialization routine.  This defaults
           to "scm_init" or "group_init", if "group" is specified.
           You must specify this for shared libraries.
init_code  code to put at the start of the init routine
fini_code  code to put at the end of the init routine

gfunc      this is a compound definition containing the following definitions
  name     the name of the function.  The Scheme string name will normally be
           derived from this name, but it may be over-ridden with the "string"
           attribute.  The transforming sed expression is:

                sed -e's/_p$/?/;s/_x$/!/;s/_/-/g;s/-to-/->/'

  string   the Scheme name for the function, if not derivable from the name.
  static   If defined, then the function will not be exported.

  exparg   "EXPression ARGument"  for each argument your routine handles,
           you must specify one of these.  This is a compound definition
           with the following "attributes" that may be defined:

    arg_name      The name of the argument.  required
    arg_desc      A very brief description of the argument.  required
    arg_optional  Specify this if the argument is only optional
    arg_list      Specify this for the last argument if the last argument
                  may be an SCM list (i.e. an SCM-flavor of var args).

syntax     This defines a Guile syntax element.  Read the Guile doc for
           descriptions of the following attributes:
  name       the name of the C variable to hold the value
  type
  cfn
  string     the Scheme name for the syntax element, if not derivable.

symbol     This defines a Guile symbol.
  name       the name of the C variable to hold the value
  init_val   initial scm value for object
  const_val  initial integer value for object (signed long)
  string     the Scheme name for the symbol, if not derivable.

If you are using a definitions file, these are defined in the normal
way.  If you are extracting them from `getdefs(1AG)' comments, then:

1.  `group' and `init' should be defined on the command line thus:
        getdefs assign=group=XX  assign=init=init_proc_name

2.  `init_code' and `fini_code' should be defined in a traditional
    definitions file and be incorporated from a command line option:
        getdefs copy=file-name.def

3.  `gfunc', `syntax', and `symbol' are getdefs' entry types.
    The `name' attributes come from the getdefs entry name.
    The remaining attributes are specified in the comment, per
    the getdefs documentation.

=][=
(define scm-prefix
        (if (exist? "group")
            (string-append (get "group") "_scm_")
            "scm_" ))
(out-push-new (string-append (base-name) ".h"))
(dne " *  " "/*  ")=]
 *
 *  copyright 1992-2004 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 *
 *  Guile Implementation Routines[=% group " - for the %s group" =]
 */
[=(make-header-guard  "GUILE_PROCS")=]
#include <guile/gh.h>

typedef enum {
    GH_TYPE_UNDEFINED = 0,
    GH_TYPE_BOOLEAN,
    GH_TYPE_SYMBOL,
    GH_TYPE_CHAR,
    GH_TYPE_VECTOR,
    GH_TYPE_PAIR,
    GH_TYPE_NUMBER,
    GH_TYPE_STRING,
    GH_TYPE_PROCEDURE,
    GH_TYPE_LIST,
    GH_TYPE_INEXACT,
    GH_TYPE_EXACT
} teGuileType;
[=
FOR gfunc       =]
extern SCM [=(. scm-prefix)=][=name=]( [=
  IF (exist? "exparg") =][=
    FOR exparg ", " =]SCM[=
    ENDFOR      =][=
  ELSE          =]void[=
  ENDIF         =] );[=
ENDFOR gfunc    =][=

FOR symbol      =][=
  IF (exist? "global") =]
extern SCM [=(. scm-prefix)=]sym_[=name=];[=
  ENDIF         =][=
ENDFOR symbol   =]

#endif /* [=(. header-guard)=] */
[=
(out-pop)
=][=

(dne " *  " "/*  ")=]
 *
 *  copyright 1992-2004 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 *
 *  Guile Initializations - [=% group (string-capitalize! "%s ")
                            =]Global Variables
 */
#include "[= (. header-file) =]"[=

DEFINE string-content   =]
static const char s_[=% name (sprintf "%%-26s" "%s[]") =] = [=
    (c-string (if (exist? "string") (get "string") (shellf
"echo '%s' | sed -e's/_p$/?/' -e's/_x$/!/' -e's/_/-/g' -e's/-to-/->/'"
      (get "name")  )))  =];[=
ENDDEF  string-content   =][=

  FOR gfunc  =][= INVOKE string-content =][= ENDFOR =][=
  FOR syntax =][= INVOKE string-content =][= ENDFOR =][=
  FOR symbol =][= INVOKE string-content =][= ENDFOR =][=

  FOR symbol =]
[=  IF (exist? "global") =]      [=ELSE=]static[=ENDIF
    =] SCM [=(. scm-prefix)=]sym_[=% name %-18s =] = SCM_BOOL_F;[=
  ENDFOR symbol =]
[=
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
=][=

DEFINE mk-new-proc =]
    NEW_PROC( [= % name (sprintf "%%-24s" "%s,") =][=

      IF (not (exist? "exparg"))
       =]0, 0, 0[=

      #  Count of all the arguments:       (count "exparg")
         Of these, some may be optional:   (count "exparg.arg_optional")
         Of the optional, one may be an arg_list.
         The sum of the three numbers must be:  (count "exparg")  =][=

      ELIF (not (exist? "exparg.arg_list")) =][=
       (- (count "exparg") (count "exparg.arg_optional")) =], [=
       (count "exparg.arg_optional" ) =], 0[=

      ELIF (not (exist? "exparg.arg_optional")) =][=
       (- (count "exparg") 1) =], 0, 1[=

      ELSE =][=
       (- (count "exparg") (count "exparg.arg_optional")) =], [=
       (- (count "exparg.arg_optional" ) 1) =], 1[=
      ENDIF =] );[=
ENDDEF =][=

# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
=]
[= (define init-proc
      (if (exist? "init")
          (get "init")
          (if (exist? "group")
              (string-append (get "group") "_init")
              "scm_init"))) =]
void [=(. init-proc)=]( void );
#define NEW_PROC( _An, _Ar, _Ao, _Ax ) \
  gh_new_procedure( (char*)s_ ## _An, ag_scm_ ## _An, _Ar, _Ao, _Ax )

/*
 * [=group=] Initialization procedure.
 */
void
[=(. init-proc)=]( void )
{[=

  (if (exist? "init-code")
      (prefix "    " (get "init-code")) "") =][=

  FOR  gfunc            =][=
      mk-new-proc       =][=
  ENDFOR gfunc          =][=

  FOR  syntax           =]
    scm_make_synt( s_[=% name (sprintf "%%-16s" "%s,")=] [=type=], [=cfn=] );[=
  ENDFOR syntax         =][=

  FOR symbol =]
    [=(. scm-prefix)=]sym_[=name=] = scm_permanent_object [=
    IF (not (and (exist? "init_val") (exist? "const_val")))
         =](SCM_CAR (scm_intern0 (s_[=name=])));[=

    ELSE =](scm_intern0 (s_[=name=]));
    SCM_SETCDR ([=(. scm-prefix)=]sym_[=name=], [=
      ?% init_val "%s" (sprintf "scm_long2num(%s)" (get "const_val"))=]);[=
    ENDIF =][=
  ENDFOR symbol =][=
    
  (if (exist? "fini-code")
      (prefix "    " (get "fini-code")) "") =]
}[= #

end of snarf.tpl =]
