#! /bin/sh
## mkpkg.sh --      create a native package
## Copyright (c):   2003-2006 by Bruce Korb
## Time-stamp:      "2006-12-10 12:14:12 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mkpkg.sh,v 4.5 2006/12/10 20:15:10 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

if [ -z "${pkgtype}" ]
then
  pkgtype=`sh ${top_srcdir}/config/config.guess | \
           sed 's,-[^-]*$,,;s,.*-,,'`
fi

if [ -f ${srcdir}/mkpkg.${pkgtype} ]
then
  . ${srcdir}/mkpkg.${pkgtype} || exit 1
else
  pkgtype=`uname -s | tr 'A-Z' 'a-z'`
  test -f ${srcdir}/mkpkg.${pkgtype} || {
    echo "No mkpkg script for ${pkgtype}" >&2
    exit 1
  }
  . ${srcdir}/mkpkg.${pkgtype} || exit 1
fi

## Local Variables:
## mode: shell-script
## indent-tabs-mode: nil
## End:
## mkpkg.sh ends here
