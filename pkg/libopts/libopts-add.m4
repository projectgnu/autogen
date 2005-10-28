
dnl @synopsis  LIBOPTS_CHECK
dnl
dnl If autoopts-config works, add the linking information to LIBS.
dnl Otherwise, add ``libopts-${ao_rev}'' to SUBDIRS and run all
dnl the config tests that the library needs.  Invoke the
dnl "INVOKE_LIBOPTS_MACROS" macro iff we are building libopts.
dnl
dnl Default to system libopts
NEED_LIBOPTS_DIR=''

AC_DEFUN([LIBOPTS_CHECK],[
  m4_pushdef([AO_Libopts_Dir],
	    [ifelse($1, , [libopts], [$1])])
  AC_SUBST(LIBOPTS_DIR, AO_Libopts_Dir)
  AC_ARG_ENABLE([local-libopts],
    AC_HELP_STRING([--enable-local-libopts],
       [Force using the supplied libopts tearoff code]),[
    if test x$enableval = xyes ; then
       AC_MSG_NOTICE([Using supplied libopts tearoff])
       LIBOPTS_LDADD='$(top_builddir)/AO_Libopts_Dir/libopts.la'
       LIBOPTS_CFLAGS='-I$(top_srcdir)/AO_Libopts_Dir'
       NEED_LIBOPTS_DIR=true
    fi])

  [if test -z "${NEED_LIBOPTS_DIR}" ; then]
     AC_MSG_CHECKING([whether autoopts-config can be found])
     AC_ARG_WITH([autoopts-config],
        AC_HELP_STRING([--with-autoopts-config],
             [specify the config-info script]),
        [lo_cv_with_autoopts_config=${with_autoopts_config}],
        AC_CACHE_CHECK([whether autoopts-config is specified],
             [lo_cv_with_autoopts_config],
             [if autoopts-config --help 2>/dev/null 1>&2
        then lo_cv_with_autoopts_config=autoopts-config
        elif libopts-config --help 2>/dev/null 1>&2
        then lo_cv_with_autoopts_config=libopts-config
        else lo_cv_with_autoopts_config=no ; fi])
     ) # end of AC_ARG_WITH

     AC_CACHE_VAL([lo_cv_test_autoopts],[
        if test -z "${lo_cv_with_autoopts_config}" \
                -o X"${lo_cv_with_autoopts_config}" = Xno
        then
           if autoopts-config --help 2>/dev/null 1>&2
           then lo_cv_with_autoopts_config=autoopts-config
           elif libopts-config --help 2>/dev/null 1>&2
           then lo_cv_with_autoopts_config=libopts-config
           else lo_cv_with_autoopts_config=false ; fi
        fi
        lo_cv_test_autoopts=`
            ${lo_cv_with_autoopts_config} --libs` 2> /dev/null
        if test $? -ne 0 -o -z "${lo_cv_test_autoopts}"
        then lo_cv_test_autoopts=no ; fi
     ]) # end of CACHE_VAL
     AC_MSG_RESULT([${lo_cv_test_autoopts}])

     [if test "X${lo_cv_test_autoopts}" != Xno
     then
        LIBOPTS_LDADD="${lo_cv_test_autoopts}"
        LIBOPTS_CFLAGS="`${lo_cv_with_autoopts_config} --cflags`"
     else
        LIBOPTS_LDADD='$(top_builddir)/]AO_Libopts_Dir[/libopts.la'
        LIBOPTS_CFLAGS='-I$(top_srcdir)/]AO_Libopts_Dir'
        INVOKE_LIBOPTS_MACROS[
     fi
  fi # end of if test -z "${NEED_LIBOPTS_DIR}"]

  AM_CONDITIONAL([NEED_LIBOPTS], [test -n "${NEED_LIBOPTS_DIR}"])
  AC_SUBST(LIBOPTS_LDADD)
  AC_SUBST(LIBOPTS_CFLAGS)
  AC_SUBST(LIBOPTS_DIR, AO_Libopts_Dir)
  AC_CONFIG_FILES(AO_Libopts_Dir/Makefile)
  m4_popdef([AO_Libopts_Dir])

  [if test -n "${NEED_LIBOPTS_DIR}" ; then]
    INVOKE_LIBOPTS_MACROS[
  elif "X${ag_boring_checks_complete}" = Xyes ; then : ; else]
    ag_BORING_CHECKS
  [fi
# end of AC_DEFUN of LIBOPTS_CHECK]
])
