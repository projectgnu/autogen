#! /bin/sh
##  -*- Mode: shell-script -*-
## mklibsrc.sh --   make the libopts tear-off library source tarball
##
## Time-stamp:      "2002-08-23 20:09:20 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Aug 20, 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mklibsrc.sh,v 3.12 2002/08/24 03:17:33 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

set -e -x

top_builddir=`cd $top_builddir ; pwd`
top_srcdir=`cd $top_srcdir ; pwd`

tag=libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}

cd ${top_builddir}/pkg
[ ! -d ${tag} ] || rm -rf ${tag}
mkdir ${tag}
mkdir ${tag}/compat

cd ../autoopts
cp -f autoopts.c     \
      autoopts.h     \
      boolean.c      \
      enumeration.c  \
      numeric.c      \
      options.h      \
      pgusage.c      \
      restore.c      \
      save.c         \
      stack.c        \
      streqv.h       \
      streqvcmp.c    \
      usage.c        \
      version.c      \
  ../pkg/${tag}/.

cp -f COPYING ../pkg/${tag}/COPYING.lgpl

cd ../compat
cp *.h pathfind.c ../pkg/${tag}/compat/.

cd ../pkg/${tag}

files=`ls -1 *.[ch] | \
	${top_builddir}/columns/columns -I4 --spread=1 --line='  \\' `

ag="${top_builddir}/agen5/autogen -L ${top_srcdir}/config --writable"
( echo "autogen definitions conftest.tpl;"
  echo "output-file = libopts.m4;"
  echo "group       = libopts;"
  cat ${top_srcdir}/config/libopts.def
) | $ag -DLIBOPTS=1

cat >> libopts.m4 <<-	EOMacro

	dnl @synopsis  LIBOPTS_CHECK
	dnl
	dnl If autoopts-config works, add the linking information to LIBS.
	dnl Otherwise, add \`\`libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}''
	dnl to SUBDIRS and run all the config tests that the library needs.
	dnl
	AC_DEFUN([LIBOPTS_CHECK],[
	  AC_MSG_CHECKING([whether autoopts-config can be found])
	  AC_ARG_WITH([autoopts-config],
        AC_HELP_STRING([--with-autoopts-config],
	                   [specify the config-info script]),
	    [lo_cv_with_autoopts_config=\${with_autoopts_config}],
	    AC_CACHE_CHECK([whether autoopts-config is specified],
	       lo_cv_with_autoopts_config,
	       lo_cv_with_autoopts_config=autoopts-config)
	  ) # end of AC_ARG_WITH
	  AC_CACHE_VAL([lo_cv_test_autoopts],[
	    aoconfig=\${lo_cv_with_autoopts_config}
	    lo_cv_test_autoopts=\`\${aoconfig} --libs\` 2> /dev/null
	    if test \$? -ne 0
	    then lo_cv_test_autoopts=no
	    else if test -z "\$lo_cv_test_autoopts"
	         then lo_cv_test_autoopts=yes
	    fi ; fi
	  ]) # end of CACHE_VAL
	  AC_MSG_RESULT([\${lo_cv_test_autoopts}])

	  if test "X\${lo_cv_test_autoopts}" != Xno
	  then
	    LIBOPTS_LDADD="\${lo_cv_test_autoopts}"
	    LIBOPTS_CFLAGS="\`\${aoconfig} --cflags\`"
	    build_libopts_dir=''
	  else
	    LIBOPTS_LDADD='\$(top_builddir)/libopts/libopts.la'
	    LIBOPTS_CFLAGS='-I\$(top_srcdir)/libopts'
	    INVOKE_LIBOPTS_MACROS
	    build_libopts_dir=true
	  fi
	  AM_CONDITIONAL([NEED_LIBOPTS], [test -n "\${build_libopts_dir}"])
	  AC_SUBST(LIBOPTS_LDADD)
	  AC_SUBST(LIBOPTS_CFLAGS)
	]) # end of AC_DEFUN of LIBOPTS_CHECK
	EOMacro

[ -f Makefile.am ] && rm -f Makefile.am

cat > Makefile.am <<-	EOMakefile

	MAINTAINERCLEANFILES = Makefile.in
	INCLUDES = @INCLIST@
	SRC      = \\
	$files
	lib_LTLIBRARIES = libopts.la
	libopts_la_SOURCES = \$(SRC)
	EOMakefile

cat > MakeDefs.inc <<-	EOMakeDefs

	## LIBOPTS Makefile Fragment
	if NEED_LIBOPTS
	  LIBOPTS_DIR  = libopts
	else
	  LIBOPTS_DIR  =
	endif
	LIBOPTS_LDADD  = @LIBOPTS_LDADD@
	LIBOPTS_CFLAGS = @LIBOPTS_CFLAGS@
	EOMakeDefs

sed s,'\${tag}',"${tag}",g ../libopts/README > README
cp ../libopts/COPYING* .

cd ..
tar cvf - ${tag} | gzip --best > ${top_builddir}/autoopts/${tag}.tar.gz
rm -rf ${tag}
