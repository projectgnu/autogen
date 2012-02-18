#! /bin/bash
#  mk-opt-table.sh
#
#  Time-stamp:        "2012-02-18 09:53:25 bkorb"
#
#  This file is part of char-mapper.
#  char-mapper Copyright (c) 1992-2012 by Bruce Korb - all rights reserved
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

declare -r progdir=$(\cd $(dirname $0) && pwd -P)
declare -r prognam=$(basename $0)
declare -r program=${progdir}/${prognam}

die() {
    echo "mk-opt-table error:  $*"
    exit 1
} >&2

init() {
    declare -g base_name=${1%%.*}
    declare -g exe_name=char-mapper
    set -e

    declare -g hdl_list=$(
        egrep '^handle_[a-z_]+\(' ${exe_name}.c | \
            sed '/^handle_invalid(/d;s/^handle_//;s/(.*//' | \
            sort)
}

mk_enum() {
    test -f ${base_name}.c \
        -a ${base_name}.c -nt ${exe_name}.c \
        -a -f ${base_name}.h \
        -a ${base_name}.h -nt ${exe_name}.c \
        -a ${base_name}.h -nt ${progdir}/mk-str2enum.sh && {
        echo "${base_name}.[ch] are up to date"
        return 0
    }

    declare opts=--base-name=${base_name}
    declare dashx=
    [[ X$- =~ .*x.* ]] && dashx=-x
    declare dispfmt=--dispatch="char * handle_%s(char * scan)"
    declare BOILERPLATE=$'/*\n'$(
        sed '1d;/^$/,$d;s/^#/ */' $program)$'\n */\n'

    echo "${base_name}.[ch]" >&5
    PS4='>mt> ' bash ${dashx} -e \
        ${progdir}/mk-str2enum.sh "${dispfmt}" ${opts} ${hdl_list}
}

assemble_usage() {
    cat <<- _EOF_
	str = <<_EOUsage_
	USAGE:  ${exe_name} [ -h | --help | <input-file> ]
	If the '<input-file>' is not specified, it is read from standard input.
	Input may not be from a TTY device.  Various directives affecting the
	output are embedded in the input text.  These are:

	_EOF_

    for d in $hdl_list
    do
        printf '  %s' "%$d"
        x="\\* *handle $d directive"
        sed -n "/$x/,/@param/{
		/$x/d"$'
		/@param/d
		s/^ *\* *//
		s/^/\t/
		p
	}' ${exe_name}.c | \
            fmt
    done

    cat <<- _EOF_
	Otherwise, input lines that are blank or start with a hash ('#') are
	ignored.

	The lines that are not ignored must begin with a name and be followed
	by space separated character membership specifications.  A quoted
	string is "cooked" and the characters found added to the set of
	member characters for the given name.  If the string is preceded by a
	hyphen ('-'), then the characters are removed from the set.  Ranges of
	characters are specified by a hyphen between two characters.  If you
	want a hyphen in the set, use it at the start or end of the string.
	You may also add or remove members of previously defined sets by name,
	preceded with a plus ('+') or hyphen ('-'), respectively.

	If the input file can be rewound and re-read, then the input text will
	also be inserted into the output as a comment.

	_EOUsage_;
	_EOF_
}

mk_text_def() {
    declare ag=$(command -v autogen)
    test -x "$ag" || {
        test -f map-text.c -a -f map-text.h || \
            die "map-text.c and .h are missing and cannot be recreated"
        echo "warning: map-text.c and .h are out of date and cannot be rebuilt"
        return 0
    } 1>&2

    test -f map-text.c -a -f map-text.h && {
        test map-text.c -nt ${exe_name}.c \
            -a map-text.h -nt ${exe_name}.c \
            -a map-text.h -nt ${program} && {
            echo 'map-text.[ch] are up to date'
            return 0
        }
    }

    declare tmp_text=$(awk '/^#include/{print $2}' map-text.def)
    assemble_usage > $tmp_text
    autogen map-text.def
    echo 'map-text.[ch]' >&5
    rm -f $tmp_text
}

trap "die failure" EXIT
exec 5> ${1}-temp
date >&5

init "$1"
mk_enum
mk_text_def

exec 5>&-
mv -f $1-temp $1
trap '' EXIT
