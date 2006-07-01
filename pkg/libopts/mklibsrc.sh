#! /bin/sh
##  -*- Mode: shell-script -*-
## mklibsrc.sh --   make the libopts tear-off library source tarball
##
## Time-stamp:      "2006-07-01 14:45:02 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Aug 20, 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mklibsrc.sh,v 4.23 2006/07/01 21:57:23 bkorb Exp $
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
mkdir ${tag} ${tag}/compat ${tag}/autoopts ${tag}/m4

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#  WORKING IN SOURCE DIRECTORY
#
cd ${top_builddir}/autoopts
files=`fgrep '#include' libopts.c | \
       sed -e 's,"$,,;s,#.*",,' \
           -e '/HAVE_LIBSNPRINTFV$/d' \
           -e '/^streqvcmp\.c$/d' \
           -e '/^compat\/compat\.h$/d' \
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
cp compat.h pathfind.c snprintf.c strdup.c strchr.c \
   ${top_builddir}/pkg/${tag}/compat/.

#
#  END WORK IN SOURCE DIRECTORY
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

cd ${top_builddir}/pkg/${tag}

cp ${top_srcdir}/config/libopts*.m4 m4/.
chmod u+w m4/libopts.m4
cat ${top_srcdir}/pkg/libopts/libopts-add.m4 >> m4/libopts.m4
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
