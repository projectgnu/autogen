[= AutoGen5 template -*- Mode: M4 -*-

m4

=]
[= #
#  Maintainer:	      Bruce Korb <bkorb@gnu.org>
#  Created:	      Tue Nov 24 01:07:30 1998
#  Last Modified:     Mon Jun 14 13:56:37 1999				      
#             by:     Bruce Korb <bkorb@gnu.org>			      
=][=
(dne "dnl ") =]
dnl
dnl  This file defines [=(count "test")=] code tests
dnl[=
(define down-name "")
(define cache-name "")
(define group-prefix
   (if (exist? "group")
       (string-append (string-downcase! (get "group")) "_")
       "" )) =][=

FOR test "\n\ndnl # # # # # # # # # # # # #\n" =]
dnl [=% group (string-upcase! "%s_") =]CHECK_[=% name (string-upcase! "%s") =]

[=% name
    (set! down-name  (string-downcase! "%s"))
    (set! cache-name (string-append group-prefix "cv_" down-name))
    ( prefix "dnl " (get "comment")) =]

AC_DEFUN([=% group (string-upcase! "%s_")
         =]CHECK_[=% name (string-upcase! "%s") =],[
  AC_MSG_CHECKING([whether [=check=]])
  AC_CACHE_VAL([[=(. cache-name)=]],[[=

  CASE (get "type") =][=

    = run =]
  AC_TRY_RUN([[=code=]],[[=(. cache-name)=]=yes],[[=(. cache-name)=]=no],[[=
            (. cache-name)=]=no]
           ) # end of TRY_RUN
   for f in core*
   do
     if test -f $f && test $f -nt conftest${ac_exeext}
     then rm -f $f ; fi
   done[=

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

ENDFOR test=]
