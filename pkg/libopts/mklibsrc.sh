#! /bin/sh

set -e

tag=libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}
cd ${top_builddir}/pkg
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
  ${PKGDIR}/${tag}/.

cd ../compat
cp *.h pathfind.c ${tag}/compat/.

cd ../pkg
cp libopts/* ${tag}/.

tar cvf - ${tag} | gzip --best > ${top_builddir}/${tag}.tar.gz
rm -rf ${tag}
