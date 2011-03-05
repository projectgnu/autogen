#! /bin/sh

PS4='>mtplc> '

set -e
pid=$$
prog=`basename $0`

die() {
    echo "$prog failure:  $*"
    kill -TERM $pid
    sleep 1
    exit 1
}

target=${1} ; shift

for d in top_srcdir srcdir top_builddir builddir
do
    eval v=\${$d}
    test -d "$v" || die "$d does not reference a directory"
    v=`cd $v >/dev/null && pwd`
    eval ${d}=${v}
done

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
grep 'extension-defines' tpl-config.tlib >/dev/null || {
    txt=`sed -n '/POSIX.*SOURCE/,/does not conform to ANSI C/{
  /^#/p
}
/does not conform to ANSI C/q' ${top_builddir}/config.h`

    cat >> tpl-config.tlib <<- _EOF_
	[= (define extension-defines
	   "${txt}") \\=]
	_EOF_
}

case `uname -s` in
SunOS )
  while : ; do
    CONFIG_SHELL=`which bash`
    test -x "${CONFIG_SHELL}" && break
    CONFIG_SHELL=/usr/xpg4/bin/sh
    test -x "${CONFIG_SHELL}" && break
    die "You are hosed.  You are on Solaris and have no usable shell."
  done
  ;;
esac

for f in ${srcdir}/tpl/*.sh ${srcdir}/tpl/*.pl
do
    d=`basename $f | sed 's/\.[sp][hl]$//'`
    st=`sed 1q $f`

    case "$st" in
    */perl ) echo '#!' `which perl`
             sed 1d $f
             ;;

    */sh )   echo '#!' ${CONFIG_SHELL}
             sed 1d $f
             ;;

    * )      die "Invalid script type: $st"
             ;;
    esac > $d
    chmod 755 $d
done

CONFIG_SHELL=`which cat`

for f in man mdoc texi
do
    for g in man mdoc texi
    do
        test -f ${f}2${g} || {
            echo '#!' ${CONFIG_SHELL} > ${f}2${g}
            chmod 755 ${f}2${g}
        }
    done
done
