#! /bin/bash

#  Time-stamp:        "2010-07-17 09:59:33 bkorb"
#
#  This file is part of char-mapper.
#  char-mapper Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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


basedir=$(pwd -P)
progdir=$(\cd $(dirname $0) && pwd -P)
prognam=$(basename $0)
program=${progdir}/${prognam}
progrev=$(echo '$Revision: 1.3 $' | sed 's/.*: *//;s/ .*//')

die() {
    echo "${prognam} error: $*" >&2
    exit 1
}

usage() {
    test $# -gt 0 && {
        exec 1>&2
        echo "${prognam} error:  $*"
    }
    cat <<- _EOF_
	USAGE: ${progname} [ <option> ] word1 word2 [...]
	       create a string-to-enumeration mapper function using \`\`gperf''
	WHERE: <option> may be:
	   -b, --base-name=base-name-for-output
	       this will set names used for the output files and the
	       various symbols used.  The default is:  cmd-name
	       The first "segment" is used for a structure field prefix.
	       It must be of alphanumeric characters, plus underscores and
	       hyphens (for segmentation).  No dotted suffix.

	   -d, --dispatch='<dispatch-proc-proto-format>'
	       Specifies that a dispatch procedure is to be emitted and that
	       the prototype of these routines is as specified with this
	       argument.  The procedure "name" must contain a \`\`%s'' that
	       will be replaced with the name of each word in the word list.

	   -n, --no-name
	       suppresses the emission of an enum-to-string-name procedure

	   --prefix=<pfx>
	       The generated enumeration, structure fields, etc. all should
	       have a short prefix.  This is normally the first segment
	       of the "base-name".  This overrides that.

	   -t, --test
	       causes a main procedure to be emitted that will enable testing.

	   -v, --version
	       prints the current program CVS revision number and exits

	   -h, --help  produces this usage text.
	There must be at least two keywords on the command line.
	_EOF_
    exit $#
}

init() {
    base_name=cmd_name
    make_test_main=false
    make_id2name=true
    dispatch_fmt=''
    make_id2name=true
    nl='
'

    while :
    do
        opt=${1}
        case "${1}" in
        ( --base*=* )
            base_name=${opt#*=}
            ;;

        ( -b | --base* )
            shift
            base_name=${1}
            ;;

        ( -b* )
            base_name=${opt#-b}
            ;;

        ( --disp*=* )
            dispatch_fmt=$(set -f; echo ${opt#*=})
            ;;

        ( -d | --disp* )
            shift
            dispatch_fmt=$(set -f; echo ${1})
            ;;

        ( -d* )
            dispatch_fmt=$(set -f; echo ${opt#-b})
            ;;

        ( --prefix=* )
            prefix=${opt#*=}
            ;;

        ( -p | --prefix )
            shift
            prefix=${1}
            ;;

        ( -p* )
            prefix=${opt#-b}
            ;;

        ( -n | --no-name )
            make_id2name=false
            ;;

        ( -t | --test )
            make_test_main=true
            ;;

        ( -v | --ver* )
            echo ${prognam} Version ${progrev}
            exit 0
            ;;

        ( --help | -[h\?] )
            usage
            ;;

        ( -- )
            shift
            break
            ;;

        ( -* )
            usage "unknown option:  ${opt}"
            ;;

        ( * )
            break
            ;;
        esac

        shift || usage "missing argument for ${opt} option"
    done

    test $# -ge 2 || usage "not enough keywords"
    case " $* " in
    ( *' invalid '* ) wordlist="$*" ;;
    ( * )             wordlist="invalid $*" ;;
    esac

    namelist=$(echo ${wordlist} | tr A-Z- a-z_ | sed 's/[^a-z0-9_]/ /g')

    base_name=$(echo ${base_name} | \
        tr A-Z- a-z_ | sed 's/[^a-z0-9_].*//')
    base_file_name=$(echo ${base_name} | sed 's/_/-/g')

    if which mktemp > /dev/null 2>&1
    then
        tmpdir=$(mktemp -d ${TMPDIR:-/tmp}/${base_file_name}-XXXXXX) || \
            die "cannot make temp dir"
    else
        tmpdir=${TMPDIR:-/tmp}/${base_file_name}-$$
        mkdir ${tmpdir}
    fi

    test -d ${tmpdir} || die "did not make temp directory ${tmpdir}"

    case "$-" in
    ( *x* ) eval "trap 'echo ${tmpdir} left undisturbed' EXIT" ;;
    (  *  ) eval "trap 'cd ${HOME} ; rm -rf ${tmpdir}' EXIT" ;;
    esac
}

mk_header() {
    header=${base_file_name}.h
    guard=$(echo $(basename ${PWD})/${header} | \
        tr a-z A-Z | sed 's/[^a-zA-Z0-9]/_/g')_GUARD

    exec 5> ${header}
    test ${#BOILERPLATE} -gt 0 && echo "${BOILERPLATE}" >&5

    : ${prefix:=$(echo ${base_name} | sed 's/_.*//')}
    PREFIX=$(echo ${prefix} | tr a-z A-Z)

    mapper_proc=find_${base_name}_id
    mapper_proto=$(cat <<- _EOF_
	${base_name}_enum_t
	${mapper_proc}(char const * str, unsigned int len)
	_EOF_
    )

    cat >&5 <<- _EOF_
	/*
	 *  Generated header for gperf generated source $(date)
	 *  This file enumerates the list of names and declares the
	 *  procedure for mapping string names to the enum value.
	 */
	#ifndef ${guard}
	#define ${guard} 1

	typedef enum {
	_EOF_

    for word in $(echo ${namelist} | tr a-z A-Z)
    do
        echo '    '${PREFIX}_KWD_${word},
    done >&5

    cat >&5 <<- _EOF_
	    ${PREFIX}_COUNT_KWD
	} ${base_name}_enum_t;

	extern ${mapper_proto};
	_EOF_

    ${make_id2name} && {
        id2name_proc=${base_name}_id_to_name
        id2name_proto=$(cat <<- _EOF_
	char const *
	${id2name_proc}(${base_name}_enum_t id)
	_EOF_
        )
        echo "${nl}extern ${id2name_proto};" >&5
    }

    test ${#dispatch_fmt} -gt 0 && {
        set -f
        dispatch_proc=disp_${base_name}
        dispatch_args=$(echo ${dispatch_fmt} | sed 's/[^(]*(/(/')
        dispatch_fmt=$( echo ${dispatch_fmt} | sed 's/(.*//')
        dispatch_ret=$( echo ${dispatch_fmt} | sed 's/ *[^ ]*$//')
        dispatch_fmt=$( echo ${dispatch_fmt} | sed 's/^.* //')
        word=$(echo ${dispatch_args} | \
            sed "s/^(/(char * str, unsigned int len, /;s/,  *void *)$/)/")
        dispatch_proto=$(cat <<- _EOF_
	${dispatch_ret}
	${dispatch_proc}${word}
	_EOF_
        )
        cat >&5 <<- _EOF_

	typedef ${dispatch_ret} (${base_name}_handler_t)${dispatch_args};

	extern ${dispatch_proto};
	_EOF_
        set +f
    }

    echo "#endif /* ${guard} */" >&5
    exec 5>&-
}

run_gperf() {
    cd ${tmpdir}
    exec 4> ${base_file_name}.gp

    opt_table=$(cat <<-_EOF_
	%struct-type
	%language=ANSI-C
	%includes
	%global-table
	%omit-struct-type
	%readonly-tables
	%compare-strncmp

	%define slot-name               ${prefix}_name
	%define hash-function-name      ${base_name}_hash
	%define lookup-function-name    find_${base_name}_name
	%define word-array-name         ${base_name}_table
	%define initializer-suffix      ,${PREFIX}_COUNT_KWD
	_EOF_
    )

    cat >&4 <<- _EOF_
	%{
	#if 0 /* gperf build options: */
	$(echo "${opt_table}" | sed 's@^@// @')
	#endif /* gperf build options: */

	#include "${header}"

	typedef struct {
	    char const *    ${prefix}_name;
	    ${base_name}_enum_t   ${prefix}_id;
	} ${base_name}_map_t;
	%}

	${opt_table}

	${base_name}_map_t;
	%%
	_EOF_

    for word in ${wordlist}
    do
        printf "%-16s ${PREFIX}_KWD_%s\\n" \
            ${word}, $(echo ${word} | tr a-z- A-Z_)
    done >&4

    cat >&4 <<- _EOF_
	%%

	${mapper_proto}
	{
	    const ${base_name}_map_t * p =
	        find_${base_name}_name(str, len);
	    return (p == 0) ? ${PREFIX}_KWD_INVALID : p->${prefix}_id;
	}
	_EOF_

    ${make_id2name} && \
        cat >&4 <<- _EOF_

	${id2name_proto}
	{
	    ${base_name}_map_t const * p = ${base_name}_table;
	    unsigned int ct = MAX_HASH_VALUE + 1;
	    while (ct-- > 0) {
	        if (p->${prefix}_id == id)
	            return p->${prefix}_name;
	        p++;
	    }
	    return "* undefined *";
	}
	_EOF_

    test ${#dispatch_fmt} -gt 0 && mk_dispatch_proc

    exec 4>&-

    gperf ${base_file_name}.gp | \
        sed -e '2,/^#endif/d' \
            -e '/^_*inline$/d' \
            -e '/^#line /d' \
            -e 's/^\(static unsigned int\)$/inline \1/' \
            -e "s/^\\(const ${base_name}_map_t\\)/static inline \\1/" \
        > baseline

    sedcmd=$(
        egrep '^#define ' baseline | \
            while read _ nm val
            do
                echo "s@${nm}@${val}@g"
            done )

    sed "/^#define/d;${sedcmd}" baseline > ${basedir}/${base_file_name}.c
}

mk_dispatch_proc() {
    cat >&4 <<- _EOF_

	${dispatch_proto}
	{
	    ${base_name}_handler_t
	_EOF_

    for word in ${namelist}
    do
        printf "${dispatch_fmt}\\n" ${word}
    done | columns --spread=1 -I8 -S, | sed '$s/$/;/' >&4
    echo "${nl}    static ${base_name}_handler_t * const dispatch[] = {" >&4

    for word in ${namelist}
    do
        printf "${dispatch_fmt}\\n" ${word}
    done | columns --spread=1 -I8 -S, | sed '$s/$/ };/' >&4

    if test -n "$(echo ${dispatch_args} | egrep '^\( *void *\)$')"
    then arglist='()'
    else
        arglist='('$(
        lastarg=''

        # each argument name is followed by a comma.  However, a space may
        # separate the name from its comma.  Whatever immediately follows a
        # comma is not going to be an argument name, so we can ignore it.
        #
        for arg in $(
            echo "${dispatch_args}" | \
                sed 's/^(//;s/)$/,/;s/\*/ /g')
        do
            case "${arg}" in
            ( ,* ) echo ${lastarg}, ; lastarg='' ;;
            ( *, ) echo ${arg}      ; lastarg='' ;;
            ( *  ) lastarg=${arg} ;; # this might be followed by a comma
            esac
        done | sed 's/\*//g;$s/,$//' )')'
        arglist=$(echo $arglist)
    fi

    case "${dispatch_ret}" in
    ( *[!a-zA-Z0-9_]void )
          return='' ;;
    ( * ) return='return ' ;;
    esac

    cat >&4 <<- _EOF_
	    ${base_name}_enum_t id = ${mapper_proc}(str, len);
	    ${return}dispatch[id]${arglist};
	}
	_EOF_
}

mk_main() {
    cd ${basedir}
    cc -c ${base_file_name}.c || die "cannot compile ${base_file_name}.c"
    exec 3> ${base_file_name}-main.c
    cat >&3 <<- _EOF_
	#include <stdio.h>
	#include "${base_file_name}.h"

	int main(int argc, char ** argv) {
	    while (--argc > 0) {
	        char const * name = *++argv;
	        ${base_name}_enum_t id = ${mapper_proc}(name);
	        if (id == ${PREFIX}_KWD_INVALID)
	             printf("INVALID:     %s\n", name);
	        else printf("%-12s == %d\n", name, id);
	    }
	    return 0;
	}
	_EOF_
    cc -o ${base_file_name} ${base_file_name}-main.c ${base_file_name}.o || \
        die "could not compile and link ${base_file_name}-main.c"
    ./${base_file_name} bad ${wordlist} $(echo ${wordlist} | sed 's/. / /g')
    rm -f ${base_file_name} ${base_file_name}.o ${base_file_name}-main*
}


init ${1+"$@"}
mk_header
run_gperf
${make_test_main} && mk_main || :
