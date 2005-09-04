#! /bin/sh
##  -*- Mode: shell-script -*-
## mklibsrc.sh --   make the libopts tear-off library source tarball
##
## Time-stamp:      "2005-09-04 12:20:53 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Aug 20, 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mklibsrc.sh,v 4.17 2005/09/04 19:26:03 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

set -e -x

top_builddir=`cd $top_builddir ; pwd`
top_srcdir=`cd $top_srcdir ; pwd`

[ -x ${top_builddir}/agen5/autogen ] || exit 0
[ -x ${top_builddir}/columns/columns ] || exit 0

ao_rev=${AO_CURRENT}.${AO_REVISION}.${AO_AGE}
tag=libopts-${ao_rev}

cd ${top_builddir}/pkg
[ ! -d ${tag} ] || rm -rf ${tag}
mkdir ${tag} ${tag}/compat ${tag}/autoopts

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#  WORKING IN SOURCE DIRECTORY
#
cd ${top_builddir}/autoopts
files=`fgrep '#include' libopts.c | \
       sed -e 's,"$,,;s,#.*",,' \
           -e '/^streqvcmp\.c$/d' \
           -e '/^config\.h$/d' \
           -e '/^autoopts\.h$/d'`
for f in libopts.c ${files}
do
  if test -f ${f}
  then cp -f ${f} ${top_builddir}/pkg/${tag}/${f}
  else cp -f ${top_srcdir}/autoopts/${f} ${top_builddir}/pkg/${tag}/${f}
  fi
done

cd ${top_srcdir}/autoopts
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

cp ${top_srcdir}/config/libopts.m4 .
chmod u+w libopts.m4
cat >> libopts.m4 <<-	\EOMacro

	dnl @synopsis  LIBOPTS_CHECK
	dnl
	dnl If autoopts-config works, add the linking information to LIBS.
	dnl Otherwise, add \`\`libopts-${ao_rev}''
	dnl to SUBDIRS and run all the config tests that the library needs.
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
	       INVOKE_LIBOPTS_MACROS
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
	    [aoconfig=${lo_cv_with_autoopts_config}]
	    AC_CACHE_VAL([lo_cv_test_autoopts],[
	         if test -z "${aoconfig}" -o X"${aoconfig}" = Xno
	         then
	           if autoopts-config --help 2>/dev/null 1>&2
	           then aoconfig=autoopts-config
	           elif libopts-config --help 2>/dev/null 1>&2
	           then aoconfig=libopts-config
	           else aoconfig=false ; fi
	         fi
	         lo_cv_test_autoopts=`${aoconfig} --libs` 2> /dev/null
	         if test $? -ne 0 -o -z "${lo_cv_test_autoopts}"
	         then lo_cv_test_autoopts=no ; fi
	    ]) # end of CACHE_VAL
	    AC_MSG_RESULT([${lo_cv_test_autoopts}])

	    [if test "X${lo_cv_test_autoopts}" != Xno
	    then
	      LIBOPTS_LDADD="${lo_cv_test_autoopts}"
	      LIBOPTS_CFLAGS="`${aoconfig} --cflags`"
	    else
	      LIBOPTS_LDADD='$(top_builddir)/]AO_Libopts_Dir[/libopts.la'
	      LIBOPTS_CFLAGS='-I$(top_srcdir)/]AO_Libopts_Dir'
	      INVOKE_LIBOPTS_MACROS
	      [NEED_LIBOPTS_DIR=true
	    fi
	  fi # end of if test -z "${NEED_LIBOPTS_DIR}"]

	  AM_CONDITIONAL([NEED_LIBOPTS], [test -n "${NEED_LIBOPTS_DIR}"])
	  AC_SUBST(LIBOPTS_LDADD)
	  AC_SUBST(LIBOPTS_CFLAGS)
	  AC_SUBST(LIBOPTS_DIR, AO_Libopts_Dir)
	  AC_CONFIG_FILES(AO_Libopts_Dir/Makefile)
	  m4_popdef([AO_Libopts_Dir])
	]) # end of AC_DEFUN of LIBOPTS_CHECK
	EOMacro

test -f Makefile.am && rm -f Makefile.am

sed s,'\${tag}',"${tag}",g ${top_srcdir}/pkg/libopts/README > README
cp ${top_srcdir}/pkg/libopts/COPYING* .

touch MakeDefs.inc

vers=${AO_CURRENT}:${AO_REVISION}:${AO_AGE}
exec 3> Makefile.am
cat >&3 <<-	EOMakefile
	## LIBOPTS Makefile
	MAINTAINERCLEANFILES  = Makefile.in
	lib_LTLIBRARIES       = libopts.la
	libopts_la_SOURCES    = libopts.c
	libopts_la_CPPFLAGS   = -I\$(top_srcdir)
	libopts_la_LDFLAGS    = -version-info ${AM_LDFLAGS} ${vers}
	EXTRA_DIST            = \\
	EOMakefile

find * -type f \
  | egrep -v '^(libopts\.c|Makefile\.am)$' \
  | ${CLexe} -I4 --spread=1 --line-sep="  \\" >&3
exec 3>&-

if gzip --version > /dev/null 2>&1
then
  gz='gzip --best'
  sfx=tar.gz
else
  gz=compress
  sfx=tar.Z
fi

cd ..
echo ! cd `pwd`
echo ! tar cvf ${tag}.${sfx} ${tag}
tar cvf - ${tag} | $gz > ${top_builddir}/autoopts/${tag}.${sfx}
rm -rf ${tag}

## Local Variables:
## Mode: shell-script
## tab-width: 4
## indent-tabs-mode: nil
## sh-indentation: 2
## sh-basic-offset: 2
## End:

# eval: (add-hook 'write-file-hooks 'time-stamp)
# time-stamp-start: "timestamp='"
# time-stamp-format: "%:y-%02m-%02d"
# time-stamp-end: "'"

## end of mklibsrc.sh
