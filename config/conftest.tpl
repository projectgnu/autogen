[= AutoGen5 template -*- Mode: M4 -*-

m4

#  Maintainer:        Bruce Korb <bkorb@gnu.org>
#  Created:           Tue Nov 24 01:07:30 1998
#  Last Modified:     $Date: 2001/10/13 18:48:48 $
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

(setenv "SHELL" "/bin/sh")

=][=
(dne "dnl " "dnl ") =]
dnl
dnl  This file defines [=(count "test")=] configuration tests
dnl[=
(define down-name  "")
(define cache-name "")
(define test-name  "")
(define group-prefix
   (if (exist? "group")
       (string-append (string-downcase! (get "group")) "_")
       "ac_" )) =][=

FOR test "\n\ndnl # # # # # # # # # # # # #\ndnl" =]
dnl [=
  (set! test-name (string-upcase! (string-append
        group-prefix "CHECK_" (get "name"))))
  (set! down-name (string-downcase! (get "name")))
  (set! cache-name (string-append group-prefix "cv_" down-name))
  (. test-name) =]
dnl
[=(shellf "sed 's,^,dnl ,' <<_EOF_\n%s\n_EOF_" (get "doc")) =][=
  # (prefix "dnl " (get "doc")) =]
dnl
AC_DEFUN([=(. test-name)=],[
  AC_MSG_CHECKING([whether [=check=]])
  AC_CACHE_VAL([[=(. cache-name)=]],[[=

  CASE (get "type") =][=

    = run =]
  AC_TRY_RUN([[=code=]],[[=(. cache-name)=]=yes],[[=(. cache-name)=]=no],[[=
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

    = link =]
  AC_TRY_LINK([[=code=]],[[=(. cache-name)=]=yes],[[=(. cache-name)=]=no]
            ) # end of TRY_LINK[=

    = compile =][=

    * =][=% type (error "`%s' is an unknown CHECK type") =][=

  ESAC

 =]]) # end of CACHE_VAL

 AC_MSG_RESULT([$[=(. cache-name)=]])
 if test x$[=(. cache-name)=] = xyes
 then
   AC_DEFINE(HAVE_[=% name (string-upcase! "%s")=], 1,
      [Define this if [=check=]])
 fi
 ]) # end of AC_DEFUN[=

ENDFOR test =][=

# end of config.tpl  =]
