#! /bin/sh

set -e

target=${1} ; shift
builddir=`pwd`
top_builddir=`dirname ${builddir}`

{
    cat <<- _EOF_
	#define  AUTOOPTS_INTERNAL
	#include "compat/compat.h"
	#define  LOCAL static
	_EOF_

    while test $# -gt 0
    do  printf '#include "%s"\n' $1
        shift
    done
} > ${target}

cd tpl

test -f tpl-config.tlib || exit 1
test -f ${top_builddir}/config.h || exit 1
grep 'extension-defines' tpl-config.tlib >/dev/null && exit 0
txt=`sed -n '/POSIX.*SOURCE/,/does not conform to ANSI C/{
  /^#/p
}
/does not conform to ANSI C/q' ${top_builddir}/config.h`

cat >> tpl-config.tlib <<- _EOF_
	[= (define extension-defines
	   "${txt}") \\=]
	_EOF_
