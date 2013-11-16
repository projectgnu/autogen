#! /bin/false

guile_scm_h=

while test $# -gt 0
do
    case "$1" in
    -I )
        test -f $2/libguile/__scm.h && {
            guile_scm_h=$2/libguile/__scm.h
            break
        }
        ;;
    -I* )
        f=${1#-I}
        test -f $f/libguile/__scm.h && {
            guile_scm_h=$f/libguile/__scm.h
            break
        }
        ;;
    esac
    shift
done

test -z "$guile_scm_h" && {
    guile_scm_h=/usr/include/libguile/__scm.h
    test -f $guile_scm_h || {
        echo "The Guile header __scm.h cannot be found"
        exit 1
    } 1>&2
}

grep -E $'/^#define[ \t]SCM_NORETURN.*\(noreturn\)'$guile_scm_h >/dev/null || \
    exit 0

test -d libguile || mkdir libguile || {
    echo "cannot make libguile directory"
    exit 1
} 1>&2

set -e
sed $'/^#define[ \t]SCM_NORETURN.*(noreturn)/s@(noreturn)@(__noreturn__)@' \
    $guile_scm_h > libguile/__scm.h
cp ${guile_scm_h%/__scm.h}.h .
