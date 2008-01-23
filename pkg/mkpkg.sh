#! /bin/sh
set -x
## mkpkg.sh --      create a native package
## Time-stamp:      "2007-07-04 11:58:16 bkorb"
##
##  This file is part of AutoGen.
##  AutoGen copyright (c) 1992-2008 Bruce Korb - all rights reserved
##  AutoGen copyright (c) 1992-2008 Bruce Korb - all rights reserved
##
##  AutoGen is free software: you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by the
##  Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  AutoGen is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##  See the GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with this program.  If not, see <http://www.gnu.org/licenses/>.
## ---------------------------------------------------------------------
## $Id: mkpkg.sh,v 4.9 2008/01/23 00:35:27 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

test -f pkg-env && . pkg-env

if test -z "${pkgtype}"
then
  if fakeroot dh_testroot 2>/dev/null 2>&1
  then pkgtype=debian
  else pkgtype=`sh ${top_srcdir}/config/config.guess | \
                sed 's,-[^-]*$,,;s,.*-,,'`
  fi
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
