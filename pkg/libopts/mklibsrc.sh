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

cd ../pkg
files=`cd ${tag} ; ls -1 *.[ch] | \
	${top_builddir}/columns/columns -I4 --spread=1 --line='  \\'
	`

cat > ${tag}/Makefile.am <<-	EOMakefile

	MAINTAINERCLEANFILES = Makefile.in
	INCLUDES = @INCLIST@
	SRC      = \\
	$files
	lib_LTLIBRARIES = libopts.la
	libopts_la_SOURCES = \$(SRC)
	EOMakefile

tar cvf - ${tag} | gzip --best > ${top_builddir}/autoopts/${tag}.tar.gz
rm -rf ${tag}
