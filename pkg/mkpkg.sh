#! /bin/sh

## mkpkg.sh --      create a native package
## Copyright (c):   2003-2007 by Bruce Korb
## Time-stamp:      "2007-02-04 10:27:47 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mkpkg.sh,v 4.6 2007/02/07 01:57:59 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

test -f pkg-env && . pkg-env

if [ -z "${pkgtype}" ]
then
  if fakeroot dh_testroot 2>/dev/null 2>&1
  then pkgtype=debian
  else pkgtype=`sh ${top_srcdir}/config/config.guess | \
                sed 's,-[^-]*$,,;s,.*-,,'`
fi

if test ! -f ${srcdir}/mkpkg.${pkgtype}
then
  pkgtype=`uname -s | tr '[A-Z]' '[a-z]'`
  test -f ${srcdir}/mkpkg.${pkgtype} || {
    echo "No mkpkg script for ${pkgtype}" >&2
    exit 1
  }
fi

. ${srcdir}/mkpkg.${pkgtype}

## Local Variables:
## mode: shell-script
## indent-tabs-mode: nil
## sh-indentation: 2
## sh-basic-offset: 2
## End:
## mkpkg.sh ends here
