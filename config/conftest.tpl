[= AutoGen5 template -*- Mode: M4 -*-

null

#  Maintainer:        Bruce Korb <bkorb@gnu.org>
#  Created:           Tue Nov 24 01:07:30 1998
#  Last Modified:     $Date: 2001/12/09 19:46:03 $
#             by:     Bruce Korb <bkorb@gnu.org>
#
# This template uses the following definitions:
#
# 1.  group  - defines a prefix for the names.  The default is "ac".
# 2.  test   - an autoconf test to perform:
# 2.a  name  - name of the test
# 2.b  type  - "run", "link" or "compile"
# 2.c  check - short display name for user entertainment
# 2.d  code  - the test code to compile, link and/or run.
# 2.e  doc   - useful explanitory text
# 2.f  require - if there are conftest prerequisites
# 2.g  author  - [optional] name of test's author

(setenv "SHELL" "/bin/sh")

=][=

(define true-txt   "")
(define false-txt  "")
(define both-txt   "")

(define down-name  "")
(define up-name    "")
(define cache-name "")
(define test-name  "")
(define c-code     "")
(define subst-name "")
(define fcn-name   "")
(define group-prefix
   (if (exist? "group")
       (string-append (string-downcase! (get "group")) "_")
       "ac_" )) =][=

FOR test  =][=
  (set! up-name    (string-upcase! (get "name")))
  (set! test-name  (string-upcase! (string-append
        group-prefix "CHECK_" up-name)))
  (set! down-name  (string-downcase! (get "name")))
  (set! cache-name (string-append group-prefix "cv_" down-name))
  (set! subst-name (string-upcase! (string-append group-prefix up-name)))
  (set! c-code (if (exist? "bracket-imbalance")
        (string-append "changequote(,)"
                       (get "code")
                       "changequote([,])")
        (string-append "[" (get "code") "]") ))
  (out-switch (string-downcase! (string-append test-name ".m4")))
  (dne "dnl " "dnl ") =]
dnl
dnl @synopsis [= (. test-name) =][=
    IF (exist? "arg") =]( [=
       FOR arg ", "   =][=arg=][=
       ENDFOR         =] )[=
    ENDIF             =]
dnl
[=(prefix "dnl " (get "doc")) =][=
  IF (or (exist? "version") (exist? "author")) =]
dnl[= (if (exist? "version")
          (sprintf "\ndnl @version %s"  (get "version")) ) =][=
      (if (exist? "author")
          (sprintf "\ndnl @author %s"   (get "author" )) ) =]
dnl[=
  ENDIF =]
AC_DEFUN([[=(. test-name)=]],[[= % require AC_REQUIRE([%s])=]
  AC_MSG_CHECKING([whether [=check=]])[=

  IF (set! fcn-name (string-append "try-" (get "type")))
     (ag-function? fcn-name) =][=
    INVOKE (. fcn-name) =][=

  ELSE   =][=
    (error (string-append "invalid conftest function:  " fcn-name)) =][=

  ENDIF  =]
  if test x$[=(. cache-name)=] = xyes
  then[=
  (. true-txt) =][=
  (if (> (string-length false-txt) 0)
      (string-append "\n  else" false-txt))      =]
  fi[=
  (if (> (string-length both-txt) 0)
      both-txt)
=]
]) # end of AC_DEFUN of [=(. test-name)=]
[=

ENDFOR test       =][=

# # # # # # # # # # C-Feature # # # # # # # # # #

Stash the result of a C/C++ feature test =][=

DEFINE  c-feature

 =][=

  IF (exist? "language") =]
  AC_LANG_POP([=language=])[=
  ENDIF  =]]) # end of CACHE_VAL

  AC_MSG_RESULT([$[=(. cache-name)=]])[=

  (set! true-txt  (if (exist? "ok-do")  (get "ok-do") ""))
  (set! false-txt (if (exist? "bad-do")
                  (prefix "    " (string-append "\n" (get "bad-do"))) ""))
  (set! both-txt  "")
  (set! fcn-name  (string-append "_" (string-upcase! (get "name")) )) =][=

  FOR action      =][=

    CASE action   =][=

    =*   sub      =][=
    (set! true-txt  (string-append true-txt  "\n    NEED" fcn-name "=false" ))
    (set! false-txt (string-append false-txt "\n    NEED" fcn-name "=true"  ))
    (set! both-txt  (string-append both-txt  "\n  AC_SUBST(NEED"
                    fcn-name ")" ))  =][=

    =*   def      =][=
    (set! true-txt  (string-append true-txt "\n    AC_DEFINE(HAVE" 
                    fcn-name ", 1,
       [Define this if " (get "check") "])" ))  =][=

    *             =][=
    (error (string-append (get "action") " is an undefined action")) =][=

    ESAC action   =][=

  ENDFOR

=][=

ENDDEF  c-feature =][=

# # # # # # # # # SET-LANGUAGE # # # # # # # # =][=

DEFINE  set-language  =]
  AC_CACHE_VAL([[=(. cache-name)=]],[[=
  IF (exist? "language") =]
  AC_LANG_PUSH([=language=])[=
  ENDIF  =][=

ENDDEF  set-language  =][=

# # # # # # # # # # RUN # # # # # # # # # =][=

DEFINE  try-run       =][=

  set-language    =]
  AC_TRY_RUN([=
     (. c-code)=],[[=(. cache-name)=]=yes],[[=(. cache-name)=]=no],[[=
            (. cache-name)=]=no]
  ) # end of TRY_RUN[=

   #  This _should_ be done, but TRY_RUN blindly obliterates "core.c"
   #  before we get here, so this is now obsolete:
   #
   for f in *core*
   do
     if test -f $f && test $f -nt conftest${ac_exeext}
     then rm -f $f ; fi
   done=][=
   c-feature      =][=

ENDDEF  try-run       =][=

# # # # # # # # # # LINK # # # # # # # # # # =][=

DEFINE  try-link      =][=

  set-language    =]
  AC_TRY_LINK([=
     (. c-code)=],[[=(. cache-name)=]=yes],[[=(. cache-name)=]=no]
  ) # end of TRY_LINK[=
  c-feature       =][=

ENDDEF  try-link      =][=

# # # # # # # # # # COMPILE # # # # # # # # # # =][=

DEFINE  try-compile   =][=

  set-language    =]
  AC_TRY_COMPILE([= % includes "[%s]" =],[=
     (. c-code)=],[[=(. cache-name)=]=yes],[[=(. cache-name)
     =]=no]) # end of TRY_COMPILE[=
  c-feature       =][=

ENDDEF  try-compile   =][=

# # # # # # # # # # SHELL TEST # # # # # # # # # # =][=

DEFINE  try-test      =][=
  (set! subst-name (string-append up-name "_$2"))
  (set! cache-name (string-append cache-name "_$2")) =]
  AC_CACHE_VAL([=(. cache-name)=],
    [=(. down-name)=]_[=arg[0]=]="$1"
    [result="[=(shellf
     "sed '1s,^\",`,;$s,\"$,`,' <<'_EOF_'\n%s\n_EOF_"
       (shell-str (get "script")) ) =]"]
    if test -z "${result}"[=

  IF (exist? "invert-sense") =]
    then [=(. cache-name)=]=""
    else [=(. cache-name)=]="$1" ; fi[=
  ELSE  =]
    then [=(. cache-name)=]="$1"
    else [=(. cache-name)=]="" ; fi[=
  ENDIF =]
  rm -f conftest* ) # end of AC_CACHE_VAL

  if test -z "$[=(. cache-name)=]"[=

  IF (exist? "invert-sense") =]
  then result=no
  else result=yes[=
  ELSE  =]
  then result=yes
  else result=no[=
  ENDIF =] ; fi
  AC_MSG_RESULT([$result])
  [=(. subst-name)=]="$[=(. cache-name)=]"
  AC_SUBST([=(. subst-name)=])[=

ENDDEF  try-test             =][=

(out-push-new "Makefile.am") =][=
(dne "#  " "#  ")            =]      
## ---------------------------------------------------------------------
## $Id: conftest.tpl,v 3.0 2001/12/09 19:46:03 bkorb Exp $
## ---------------------------------------------------------------------

GENERATED_M4 = \
[=
(out-push-new) =][=
FOR test "\n"  =][=
  (string-downcase! (string-append
        group-prefix "check_" (get "name"))) =].m4[=
ENDFOR =]
[=

(shellf "columns -I'\t' --spread=2 --line-sep=' \\' <<_EOF_\n%s\n_EOF_\n"
          (out-pop #t) )=]

EXTRA_DIST	= byacc.m4 libregex.m4 openmode.m4 $(GENERATED_M4) autogen.spec \
		missing release bootstrap config.tpl misc.def bootstrap.local

pkgdata_DATA = config.tpl

MAINTAINERCLEANFILES = Makefile.in config.guess config.sub install-sh \
		ltconfig ltmain.sh missing mkinstalldirs $(GENERATED_M4)
all:
	:
[=

(out-pop)

;; end config.tpl  =]
