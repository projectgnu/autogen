#! /bin/sh
##  -*- Mode: shell-script -*-
## mklibsrc.sh --   make the libopts tear-off library source tarball
##
## Time-stamp:      "2002-09-21 15:22:33 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Aug 20, 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mklibsrc.sh,v 3.16 2002/09/29 00:16:20 bkorb Exp $
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

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#  WORKING IN SOURCE DIRECTORY
#
cd ${top_srcdir}/autoopts
cp -f autoopts.c     \
      boolean.c      \
      enumeration.c  \
      numeric.c      \
      options.h      \
      pgusage.c      \
      restore.c      \
      save.c         \
      stack.c        \
      streqv.h       \
      usage.c        \
      version.c      \
  ${top_builddir}/pkg/${tag}/.

cp -f COPYING ${top_builddir}/pkg/${tag}/COPYING.lgpl

sed '/#ifdef AUTOGEN_BUILD/,/#endif.* AUTOGEN_BUILD/d' streqvcmp.c > \
  ${top_builddir}/pkg/${tag}/streqvcmp.c

sed '/broken printf/,/our own/d
     /include.*"snprintfv/d
     /#ifndef AUTOGEN_BUILD/d
     /#else.* AUTOGEN_BUILD/,/#endif.* AUTOGEN_BUILD/d' autoopts.h > \
  ${top_builddir}/pkg/${tag}/autoopts.h

cd ../compat
cp pathfind.c compat.h ${top_builddir}/pkg/${tag}/compat/.
#
#  END WORK IN SOURCE DIRECTORY
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

cd ${top_builddir}/pkg/${tag}

${top_builddir}/agen5/autogen -L ${top_srcdir}/config --writable <<-	'_EOF_'

	autogen definitions conftest.tpl;
	output-file = libopts.m4;
	group       = libopts;
	#define       LIBOPTS  1
	#include      libopts.def
	_EOF_

cat >> libopts.m4 <<-	EOMacro

	dnl @synopsis  LIBOPTS_CHECK
	dnl
	dnl If autoopts-config works, add the linking information to LIBS.
	dnl Otherwise, add \`\`libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}''
	dnl to SUBDIRS and run all the config tests that the library needs.
	dnl
	AC_DEFUN([LIBOPTS_CHECK],[
	`sed -n '/Check for standard headers/,/gen, pathfind/p' \
	     ${top_srcdir}/configure.in`

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
	  AC_CONFIG_FILES([libopts/Makefile])
	]) # end of AC_DEFUN of LIBOPTS_CHECK
	EOMacro

[ -f Makefile.am ] && rm -f Makefile.am

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

sed s,'\${tag}',"${tag}",g ${top_srcdir}/pkg/libopts/README > README
cp ${top_srcdir}/pkg/libopts/COPYING* .

exec 3> Makefile.am
cat >&3 <<-	EOMakefile
	## LIBOPTS Makefile
	EXTRA_DIST            = `echo COPYING*` MakeDefs.inc compat
	MAINTAINERCLEANFILES  = Makefile.in
	lib_LTLIBRARIES       = libopts.la
	libopts_la_SOURCES    = \\
	EOMakefile

ls -1 *.[ch] | \
  ${top_builddir}/columns/columns -I4 --spread=1 --line-sep='  \' >&3

exec 3>&-

cd ..
tar cvf - ${tag} | gzip --best > ${top_builddir}/autoopts/${tag}.tar.gz
rm -rf ${tag}
