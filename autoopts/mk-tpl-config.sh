#! /bin/sh

##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2014 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following sha256 sums:
##
##  8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
##  4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
##  13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd

prog=`basename $0 .sh`

die() {
    echo "$prog failure:  $*"
    kill -TERM $progpid
    sleep 1
    exit 1
}

init() {
    PS4='>tpc-${FUNCNAME}> '
    set -e
    progpid=$$
    prog=`basename $0`
    progdir=`\cd \`dirname $0\` >/dev/null ; pwd`
    readonly progpid progdir prog

    for d in top_srcdir srcdir top_builddir builddir
    do
        eval v=\${$d}
        test -d "$v" || die "$d does not reference a directory"
        v=`cd $v >/dev/null && pwd`
        eval ${d}=${v}
    done
    . ${top_builddir}/config/shdefs
}

collect_src() {
    exec 8>&1 1>&2
    cd ${builddir}
    sentinel_file=${1} ; shift
    cat 1>&8 <<- _EOF_
	#define  AUTOOPTS_INTERNAL 1
	#include "autoopts/project.h"
	#define  LOCAL static
	#include "ao-strs.h"
	_EOF_

    for f in "$@"
    do  test "X$f" = "Xproject.h" || \
            printf '#include "%s"\n' $f
    done 1>&8
}

extension_defines() {
    cd ${builddir}/tpl

    test -f tpl-config.tlib || die "tpl-config.tlib not configured"
    test -f ${top_builddir}/config.h || die "config.h missing"
    ${GREP} 'extension-defines' tpl-config.tlib >/dev/null && return

    txt=`sed -n '/POSIX.*SOURCE/,/does not conform to ANSI C/{
	    /^#/p
	}
	/does not conform to ANSI C/q' ${top_builddir}/config.h`

    {
        sed '/define  *top-build-dir/d;/^;;;/d' tpl-config.tlib
        cat <<- _EOF_
	(define top-build-dir   "`cd ${top_builddir} >/dev/null
		pwd`")
	(define top-src-dir     "`cd ${top_srcdir} >/dev/null
		pwd`")
	(define extension-defines
	   "${txt}") \\=]
	_EOF_
    } > tpl-config.$$
    mv -f  tpl-config.$$  tpl-config.tlib
}

set_shell_prog() {
    case `uname -s` in
    SunOS )
      while : ; do
        POSIX_SHELL=`which bash`
        test -x "${POSIX_SHELL}" && break
        POSIX_SHELL=/usr/xpg4/bin/sh
        test -x "${POSIX_SHELL}" && break
        die "You are hosed.  You are on Solaris and have no usable shell."
      done
      ;;
    esac

    for f in ${srcdir}/tpl/*.sh ${srcdir}/tpl/*.pl
    do
        d=`basename $f | sed 's/\.[sp][hl]$//'`
        st=`sed 1q $f`

        case "$st" in
        *perl ) echo '#!' `which perl`
                 sed 1d $f
                 ;;

        */sh )   echo '#!' ${POSIX_SHELL}
                 sed 1d $f
                 ;;

        * )      die "Invalid script type: $st"
                 ;;
        esac > $d
        chmod 755 $d
    done
}

set_cat_prog() {
    while :
    do
        \unalias -a
        unset -f command cat which
        POSIX_CAT=`which cat`
        test -x "$POSIX_CAT" && break
        POSIX_CAT=`
            PATH=\`command -p getconf CS_PATH\`
            command -v cat `
        test -x "${POSIX_CAT}" && break
        die "cannot locate 'cat' command"
    done

    formats='man mdoc texi'
    for f in $formats
    do
        for g in $formats
        do
            test -f ${f}2${g} || {
                printf "#! ${POSIX_SHELL}\nexec ${POSIX_CAT} "'${1+"$@"}\n' \
                    > ${f}2${g}
                chmod 755 ${f}2${g}
            }
        done
    done
}

init
collect_src "$@" > ${builddir}/libopts.c
extension_defines
set_shell_prog
set_cat_prog
touch ${sentinel_file}
