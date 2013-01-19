#! /bin/sh

## mdoc2man.sh -- script to convert mdoc-isms to man-isms
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2012 Bruce Korb - all rights reserved
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

## This "library" converts mdoc-isms into man-isms.  It gets included
## by the man page template at the point where mdoc-isms might start appearing
## and then "emit-man-text" is invoked when all the text has been assembled.
##
## Display the command line prototype,
## based only on the argument processing type.
##
## And run the entire output through "sed" to convert mdoc-isms

# /bin/sh on Solaris is too horrible for words
#
case "$0" in
/bin/sh ) test -x /usr/xpg4/bin/sh && exec /usr/xpg4/bin/sh ${1+"$@"} ;;
esac

parent_pid=$$
prog=`basename $0 .sh`
NmName=

die() {
    echo "$prog error:  $*" >&2
    ps -p ${AG_pid:-999999999} >/dev/null 2>&1 && \
        kill -TERM ${AG_pid}
    kill -TERM $parent_pid
    sleep 1
    kill -9 $parent_pid
    sleep 1
    exit 1
}

had_no_arg() {
    die "'$1' command had no argument: <$line>"
}

# One function for each mdoc structure.
#
do_nest_enum() {
    do_enum
}

do_enum() {
    echo '.in +4'
    local ix=1

    while IFS='' read -r line
    do
        case "${line}" in
        .It* ) printf '.ti -4\n%d\n\t' $ix
               ix=`expr $ix + 1`
               ;;

        .Bl' '*enum* )   do_nest_enum      ;;
        .Bl' '*tag* )    do_nest_tag       ;;
        .Bl' '*bullet* ) do_nest_bullet    ;;
        .Bd' '* )        do_nest_block     ;;
        .Op' '* )        do_nest_optional  ;;
        .Fl' '* )        do_nest_flag      ;;
        .Ar' '* )        do_nest_arg       ;;

        .El* )           echo '.in -4'
                         return 0          ;;

        * )              echo "$line"      ;;
        esac
    done
    die "EOF reached processing '.Bl -enum'"
}

do_nest_tag() {
    echo '.in +4'
    while IFS='' read -r line
    do
        case "${line}" in
        .It* ) printf '.ti -4\n.IR '
               echo ${line#.It} ;;

        .Bl' '*enum* )   do_nest_enum      ;;
        .Bl' '*tag* )    do_nest_tag       ;;
        .Bl' '*bullet* ) do_nest_bullet    ;;
        .Bd' '* )        do_nest_block     ;;
        .Op' '* )        do_nest_optional  ;;
        .Fl' '* )        do_nest_flag      ;;
        .Ar' '* )        do_nest_arg       ;;

        .El* )           echo '.in -4'
                         return 0          ;;

        * )              echo "$line"      ;;
        esac
    done
    die "EOF reached processing '.Bl -tag'"
}

do_tag() {
    while IFS='' read -r line
    do
        case "${line}" in
        .It* ) printf '.TP\n.BR '
               echo ${line#.It}            ;;

        .Bl' '*enum* )   do_nest_enum      ;;
        .Bl' '*tag* )    do_nest_tag       ;;
        .Bl' '*bullet* ) do_nest_bullet    ;;
        .Bd' '* )        do_nest_block     ;;
        .Op' '* )        do_nest_optional  ;;
        .Fl' '* )        do_nest_flag      ;;
        .Ar' '* )        do_nest_arg       ;;
        .El* )           return 0          ;;
        * )              echo "$line"      ;;
        esac
    done
    die "EOF reached processing '.Bl -tag'"
}

do_nest_bullet() {
    do_bullet
}

do_bullet() {
    echo '.in +4'
    while IFS='' read -r line
    do
        case "${line}" in
        .It* ) printf '.ti -4\n\\fB*\\fP\n'
               echo ${line#.It}
               ;;

        .Bl' '*enum* )   do_nest_enum      ;;
        .Bl' '*tag* )    do_nest_tag       ;;
        .Bl' '*bullet* ) do_nest_bullet    ;;
        .Bd' '* )        do_nest_block     ;;
        .Op' '* )        do_nest_optional  ;;
        .Fl' '* )        do_nest_flag      ;;
        .Ar' '* )        do_nest_arg       ;;

        .El* )           echo '.in -4'
                         return 0          ;;

        * )              echo "$line"      ;;
        esac
    done
    die "EOF reached processing '.Bl -bullet'"
}

do_nest_block() {
    do_block
}

do_block() {
    printf '.br\n.in +4\n.nf\n'
    while IFS='' read -r line
    do
        case "${line}" in
        .B*  ) die ".Bx command nested within .Bd" ;;

        .Ed* ) echo .in -4
               echo .fi
               return 0          ;;

        * )    echo "$line"      ;;
        esac
    done
    die "EOF reached processing '.Bd'"
}

do_nest_optional() {
    do_optional
}

do_optional() {
    set -- $line
    shift
    local text='['
    while test $# -gt 0
    do
        # Ns may be fun...
        m1="$1"
        case "X$1" in
        X\\\& )
            # The general escape sequence is: \&
            text="${text} ${1}"
            ;;

        X[!0-9a-zA-Z]* )
            # This one should be a generic punctuation check
            # Punctuation is any of the 10 chars: .,:;()[]?!  (but
            # more are actually used and accepted) and is output in
            # the default font.
            #
            text="${text} ${1}"
            ;;

        XAr | XCm )
            text=${text}' "\fI'${2}'\fR"'
            shift
            ;;

        XFl )
            text=${text}' \fB-'${2}'\fR'
            shift
            ;;

        * ) # Grab subsequent non-keywords and punctuation, too
            text="${text} $1"
            ;;
        esac
        shift || die "shift failed after $m1"
    done
    echo "${text} ]"
}

do_nest_flag() {
    do_flag
}

do_flag() {
    echo ${line#.Fl}
}

do_nest_arg() {
    do_arg
}

do_arg() {
    line=`echo ${line#.Ar}`
    echo "\\fI${line}\\fR"
}

do_NmName() {
    # do we want to downcase the line first?  Yes...
    set -- `echo ${line#.Nm} | \
        sed -e 's/-/\\-/g' \
            -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`
    NmNameSfx=

    if test $# -gt 0
    then case "$1" in
        [A-Za-z]* )
            NmName=$1
            shift
            ;;
        esac

        test $# -gt 0 && NmNameSfx=" $*"
    fi
    echo ".B $NmName$NmNameSfx"
}

do_line() {
    case "${line}" in
    .Bl' '*enum* )   do_enum      ;;
    .Bl' '*tag* )    do_tag       ;;
    .Bl' '*bullet* ) do_bullet    ;;
    .Bd' '*        ) do_block     ;;
    .Op' '* )        do_optional  ;;
    .Fl' '* )        do_flag      ;;
    .Ar' '* )        do_arg       ;;
    .Nm' '* )        do_NmName    ;;
    .Nm     )        do_NmName    ;;
    * )              echo "$line" ;;
    esac
    return 0
}



easy_fixes='
s/^\.Sh/.SH/
s/^\.Ss/.SS/
s/^\.Em/.I/
s/^\.Pp/.PP/
s/^.in *\\-/.in -/
'

readonly easy_fixes
set -f

{
    while IFS='' read -r line
    do
        do_line
    done
} | sed "${easy_fixes}"

exit 0
