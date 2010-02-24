#! /bin/bash
#  mk-opt-table.sh
#  $Id$
#
#  Time-stamp:        "2010-02-24 08:43:43 bkorb"
#  Last Committed:    $Date: 2009/08/01 14:05:00 $
#
#  This file is part of char-mapper.
#  char-mapper Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
#
# char-mapper is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# char-mapper is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

test "X${1}" = X--clean -o "X${1}" = X--clobber && {
    shift
    base_name=${1%%.*}
    echo "rm -f ${base_name}*"
    rm -f ${base_name}*
    exit 0
}
base_name=${1%%.*}
base_name=--base-name=${base_name}

set -e
progdir=$(\cd $(dirname $0) && pwd -P)
prognam=$(basename $0)
program=${progdir}/${prognam}

list=$(
    egrep '^handle_[a-z_]+\(' char-mapper.c | \
        sed '/^handle_invalid(/d;s/^handle_//;s/(.*//')

case "$-" in ( *x* ) dashx=-x ;; ( * ) dashx='' ;; esac
dispfmt=--dispatch="char * handle_%s(char * scan)"
BOILERPLATE=$'/*\n'$(sed '1d;/^$/,$d;s/^#/ */' $0)$'\n */\n'

export BOILERPLATE

PS4='>mt> ' bash ${dashx} -e \
    ${progdir}/mk-str2enum.sh "${dispfmt}" ${base_name} ${list}
