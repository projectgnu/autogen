#! /bin/sh
## mkpkg.sh --      create a native package
## Copyright (c):   2003-2004 by Bruce Korb
## Time-stamp:      "2003-02-19 20:25:31 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: mkpkg.sh,v 3.2 2004/02/01 21:26:46 bkorb Exp $
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
  echo "No mkpkg script for ${pkgtype}" >&2
  exit 1
fi

## Local Variables:
## mode: shell-script
## indent-tabs-mode: nil
## tab-width: 4
## End:
## mkpkg.sh ends here
