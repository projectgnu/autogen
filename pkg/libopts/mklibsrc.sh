#! /bin/sh

set -e -x

top_builddir=`cd $top_builddir ; pwd`
top_srcdir=`cd $top_srcdir ; pwd`

tag=libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}

cd ${top_builddir}/pkg
[ ! -d ${tag} ] || rm -rf ${tag}
mkdir ${tag}
mkdir ${tag}/compat

cd ../autoopts
cp -f COPYING        \
      autoopts.c     \
      autoopts.h     \
      autoopts.m4    \
      boolean.c      \
      enumeration.c  \
      genshell.c     \
      genshell.h     \
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

cd ../compat
cp *.h pathfind.c ../pkg/${tag}/compat/.

cd ../pkg/${tag}

files=`ls -1 *.[ch] | \
	${top_builddir}/columns/columns -I4 --spread=1 --line='  \\' `

( echo "autogen definitions conftest.tpl;"
  echo "output-file = libopts.m4;"
  echo "group       = libopts;"
  cat ${top_srcdir}/config/libopts.def
) | ${top_builddir}/agen5/autogen -L ${top_srcdir}/config -- -

[ -f Makefile.am ] && rm -f Makefile.am

cat > Makefile.am <<-	EOMakefile

	MAINTAINERCLEANFILES = Makefile.in
	INCLUDES = @INCLIST@
	SRC      = \\
	$files
	lib_LTLIBRARIES = libopts.la
	libopts_la_SOURCES = \$(SRC)
	EOMakefile

sed s,'\${tag}',"${tag}",g ../libopts/README > README
cp ../libopts/COPYING* .

cd ..
tar cvf - ${tag} | gzip --best > ${top_builddir}/${tag}.tar.gz
rm -rf ${tag}
