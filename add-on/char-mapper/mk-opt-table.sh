#! /bin/bash
#  mk-opt-table.sh
#
#  Time-stamp:        "2012-01-29 11:30:50 bkorb"
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

die() {
    echo "mk-opt-table error:  $*"
    exit 1
} >&2

usage_file=cm-usage.c
test "X${1}" = X--clean -o "X${1}" = X--clobber && {
    shift
    base_name=${1%%.*}
    echo "rm -f ${base_name}* $usage_file"
    rm -f ${base_name}* $usage_file
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

tmp_text=$(mktemp ./cm-usage-XXXXXX)
test -f "$tmp_text" || die "cannot mktemp"
{
    echo "[= AutoGen5 Template c=$usage_file =]"

    for d in $list
    do
        printf '' "$d"
    done

    cat <<- _EOF_
	[= (out-push-new) \=]
	USAGE:  char-mapper [ -h | --help | <input-file> ]
	If the '<input-file>' is not specified, it is read from standard input.
	Input may not be from a TTY device.  Various directives affecting the
	output are embedded in the input text.  These are:

	_EOF_

    for d in $list
    do
        printf '  %s' "%$d"
        x="\\* *handle $d directive"
        sed -n "/$x/,/@param/{
		/$x/d"$'
		/@param/d
		s/^ *\* *//
		s/^/\t/
		p
	}' char-mapper.c | \
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
	[= (define usage-text (out-pop #t))
	(sprintf "static char const usage_txt[%d] =\n       %s;"
	         (+ 1 (string-length usage-text)) (c-string usage-text) ) =]
	_EOF_

} > $tmp_text

autogen --no-def -T ./$tmp_text || die "autogen failed"
rm -f $tmp_text
