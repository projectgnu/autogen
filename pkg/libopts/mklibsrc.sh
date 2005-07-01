#! /bin/sh
##  -*- Mode: shell-script -*-
## mklibsrc.sh --   make the libopts tear-off library source tarball
##
## Time-stamp:      "2005-06-28 19:01:10 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Aug 20, 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mklibsrc.sh,v 4.13 2005/07/01 16:34:02 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

set -e -x

top_builddir=`cd $top_builddir ; pwd`
top_srcdir=`cd $top_srcdir ; pwd`
export top_srcdir top_builddir

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

cd ${top_builddir}/doc
sh ${top_srcdir}/mk-libopts-texi.sh
mv -f libopts.texi ../pkg/${tag}
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
	  AC_ARG_ENABLE([local-libopts],
	    AC_HELP_STRING([--enable-local-libopts],
	       [Force using the supplied libopts tearoff code]),[
	    if test x$enableval = xyes ; then
	       AC_MSG_NOTICE([Using supplied libopts tearoff])
	       LIBOPTS_LDADD='$(top_builddir)/libopts/libopts.la'
	       LIBOPTS_CFLAGS='-I$(top_srcdir)/libopts'
	       INVOKE_LIBOPTS_MACROS
	       NEED_LIBOPTS_DIR=true
	    fi])

	  if test -z "${NEED_LIBOPTS_DIR}" ; then
	    AC_MSG_CHECKING([whether autoopts-config can be found])
	    AC_ARG_WITH([autoopts-config],
	       AC_HELP_STRING([--with-autoopts-config],
	            [specify the config-info script]),
	       [lo_cv_with_autoopts_config=${with_autoopts_config}],
	       AC_CACHE_CHECK([whether autoopts-config is specified],
	            lo_cv_with_autoopts_config,
	            lo_cv_with_autoopts_config=autoopts-config)
	     ) # end of AC_ARG_WITH
	     AC_CACHE_VAL([lo_cv_test_autoopts],[
	         aoconfig=${lo_cv_with_autoopts_config}
	         lo_cv_test_autoopts=`${aoconfig} --libs` 2> /dev/null
	         if test $? -ne 0 -o -z "${lo_cv_test_autoopts}"
	        then lo_cv_test_autoopts=no ; fi
	    ]) # end of CACHE_VAL
	    AC_MSG_RESULT([${lo_cv_test_autoopts}])

	    if test "X${lo_cv_test_autoopts}" != Xno
	    then
	      LIBOPTS_LDADD="${lo_cv_test_autoopts}"
	      LIBOPTS_CFLAGS="`${aoconfig} --cflags`"
	    else
	      LIBOPTS_LDADD='$(top_builddir)/libopts/libopts.la'
	      LIBOPTS_CFLAGS='-I$(top_srcdir)/libopts'
	      INVOKE_LIBOPTS_MACROS
	      NEED_LIBOPTS_DIR=true
	    fi
	  fi # end of if test -z "${NEED_LIBOPTS_DIR}"

	  AM_CONDITIONAL([NEED_LIBOPTS], [test -n "${NEED_LIBOPTS_DIR}"])
	  AC_SUBST(LIBOPTS_LDADD)
	  AC_SUBST(LIBOPTS_CFLAGS)
	  AC_CONFIG_FILES([libopts/Makefile])
	]) # end of AC_DEFUN of LIBOPTS_CHECK
	EOMacro

cat > configure.ac <<- EOConfig
	# This configure.ac file is for use when "libopts" is an independent project.
	#
	AC_INIT([libopts],[${ao_rev}],[autogen-users@lists.sf.net])
	AC_CONFIG_SRCDIR(libopts/autoopts.c)
	AC_CONFIG_AUX_DIR(m4)
	AC_CONFIG_MACRO_DIR(m4)
	AC_CANONICAL_TARGET
	AM_INIT_AUTOMAKE([gnu check-news 1.5 dist-bzip2])
	AC_LIBTOOL_WIN32_DLL    m4_define(AC_PROVIDE_AC_LIBTOOL_WIN32_DLL)
	AC_PROG_LIBTOOL
	ifdef([AC_REVISION],AC_REVISION($Revision: 4.13 $),)dnl
	AC_SUBST(AO_CURRENT)
	AC_SUBST(AO_REVISION)
	AC_SUBST(AO_AGE)
	AC_DEFINE_UNQUOTED(AO_CURRENT,${AO_CURRENT},
                   [Define this to the autoopts current interface number])
	AC_DEFINE_UNQUOTED(AO_REVISION,${AO_REVISION},
                   [Define this to the autoopts interface revision number])
	AC_DEFINE_UNQUOTED(AO_AGE,${AO_AGE},
                   [Define this to the autoopts interface age number])

	# ----------------------------------------------------------------------
	# If 'configure' is invoked (in)directly via 'make', ensure that it
	# encounters no 'make' conflicts.  Ignore error if shell does not have
	# unset, but at least set these to empty values.
	# ----------------------------------------------------------------------
	MFLAGS=
	MAKEFLAGS=
	MAKELEVEL=
	unset MFLAGS MAKEFLAGS MAKELEVEL 2>/dev/null
	AC_PROG_CC
	AC_PROG_CC_STDC
	AC_EXEEXT
	AC_PROG_INSTALL
	AC_PROG_LIBTOOL
	AC_C_CONST
	AC_C_INLINE
	AC_TYPE_MODE_T
	AC_TYPE_PID_T
	AC_TYPE_SIZE_T
	AC_TYPE_UID_T
	AC_C_LONG_DOUBLE
	AC_CHECK_TYPES([u_int, long long, uintmax_t])
	AC_CHECK_TYPES([uintptr_t], ,
	               [AC_DEFINE(uintptr_t, unsigned long,
	                          [Alternate uintptr_t for systems without it.])])
	AC_CHECK_SIZEOF(char*, 4)
	AC_CHECK_SIZEOF(int,   4)
	AC_CHECK_SIZEOF(long,  4)
	AC_CHECK_FUNCS(strchr setjmp sigsetjmp strsignal strlcpy snprintf realpath \
	               dlopen)
	INVOKE_LIBOPTS_MACROS
	AC_CONFIG_FILES([Makefile libopts/Makefile])
	AM_CONFIG_HEADER(config.h:config-h.in)
	AC_OUTPUT
	EOConfig

exec 3> bootstrap
cat >&3 <<- EObootstrap
	#! /bin/sh
	#  Run this script to convert this tear-off/add-in library into a separately
	#  configurable package.  Once you do this, you cannot undo it.  Unroll the
	#  original tarball again.
	set -x -e
	mkdir libopts doc
	mv -f *.c *.h libopts/.
	mv -f libopts.texi doc/.
	( cd doc
	  makeinfo -I../autoopts -I . -o libopts.info libopts.texi
	) || exit 1
	( sed '/EXTRA_DIST/,\$d' Makefile.am
	  rm -f Make*
	  echo 'EXTRA_DIST = \\'
	  # '
	  cd libopts
	  ls -C *.[ch] | sed 's/libopts\\.c//;s/\$/ \\\\/;\$s/ .\$//'
	) > libopts/Makefile.am
	( echo SUBDIRS = libopts
	  echo nobase_include_HEADERS = autoopts/options.h autoopts/usage-txt.h
	  echo EXTRA_DIST = COPYING COPYING.mbsd AUTHORS README ChangeLog NEWS '\\'
	  # '
	  echo '           ' compat/*.* '\\'
	  # '
	  echo '           ' doc/*.*
	) > Makefile.am
	mkdir m4
	sed '/LIBOPTS_CHECK/,\$d' libopts.m4 > m4/libopts.m4
	rm -f libopts.m4
	mv COPYING.lgpl COPYING
	libtoolize --force
	aclocal -I m4
	autoheader
	automake --add-missing
	autoconf
	EObootstrap

echo 'cat > README <''<- _EOreadme_' >&3

cat >&3 <<- _EObootstrap
	This is an unmodifiable source package for libopts.
	It is not modifiable because it gets regenerated.  Edit the source.
	_EOreadme_
	_EObootstrap

exec 3>&-
chmod +x bootstrap

cat > ChangeLog <<- EOChangeLog
	This entire project was extracted from and generated by the AutoGen project
	on   $(date)
	with ${top_srcdir}/pkg/libopts/mklibsrc.sh
	by   $(logname 2>/dev/null).
	EOChangeLog

cat > AUTHORS <<- EOAuthors
	Bruce Korb:
	    The entirety of libopts.
	EOAuthors

( echo New in ${ao_rev} - $(date "%m, %Y")
  echo
  cat ChangeLog
) > NEWS

test -f Makefile.am && rm -f Makefile.am

sed s,'\${tag}',"${tag}",g ${top_srcdir}/pkg/libopts/README > README
cp ${top_srcdir}/pkg/libopts/COPYING* .

cat > MakeDefs.inc <<- EODefs
	if NEED_LIBOPTS
	LIBOPTS_DIR = libopts
	else
	LIBOPTS_DIR =
	endif
	EODefs

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
