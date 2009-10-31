#! /bin/bash

prog=$(basename $0 .sh)

usage()
{
  test $# -gt 0 && {
    exec >&2
    echo "${prog} error:  $*"
  }
  echo USAGE:\  $prog [-p] [ c-file1 [ ... ]]

  cat <<- \_EOF_
	WHERE:  all local procedures tagged, 'static' and the beginning of the
	        source file has lines marked, "/* START-STATIC-FORWARD */" and
	        "/* END-STATIC-

	'-p' specifies producing a "proto.h" containing local procedures.

	There are three flavors of procedures:  those static to a module,
	those local to the library or program, and those exported for other
	programs and libraries to link against.  They are labeled,
	"static", "LOCAL" and "EXPORT" respectively.  The latter two are
	#defined into something the compiler can cope with.
	_EOF_
  exit $#
}

do_proto=false

${VERBOSE:-false} && set -x

case "$1" in
"-?" | -h* | --h* )
  usage ;;

-p | --pro* )
  shift
  rm -f proto.h
  exec 5> proto.h
  do_proto=true
  dirname=$(basename $(pwd))
  marker=$(echo ${dirname} | \
      sed 's/[^A-Za-z0-9]/_/g' | \
      tr '[a-z]' '[A-Z]' )_PROTO_H_GUARD
  cat >&5 <<- _EOF_
	/* -*- buffer-read-only: t -*- vi: set ro:
	 *
	 * Prototypes for ${dirname}
	 * Generated $(date)
	 */
	#ifndef ${marker}
	#define ${marker} 1

	#ifndef LOCAL
	#  define LOCAL extern
	#  define REDEF_LOCAL 1
	#else
	#  undef  REDEF_LOCAL
	#endif
	_EOF_
esac

if (( $# == 0 ))
then
  set -- *.c
  test -f ${1} || usage "No *.c files in" $(pwd)
fi

for f
do
  test ! -f ${f} && echo "not a file:  $f" >&2 && continue
  ct=$(echo $(egrep '(START|END)-STATIC-FORWARD' $f | wc -l) )

  case ${ct} in
  2 )
    rm -f $f.[XY]

    exec 4> $f.X
    sed '/START-STATIC-FORWARD/q' $f >&4
    echo "/* static forward declarations maintained by ${prog} */" >&4
    sed -n '/END-STATIC-FORWARD/,$p' $f > $f.Y
    sed -n '/^static /,/^{/p' $f.Y | \
      sed 's/^{.*//;s/)$/);/;$d' >&4
    cat $f.Y >&4
    exec 4>&-

    if cmp $f.X $f > /dev/null 2>&1
    then :
    else echo WARNING: ${f} has been updated >&2
      mv -f $f.X $f ; fi

    rm -f $f.[XY]
    ;;

  0 ) : ;;
  * )
    echo "bad marker line count:" $ct in $f >&2
    continue
  esac

  ${do_proto} && {
    ct=$(echo $(egrep '^LOCAL ' $f | wc -l) )
    test $ct -eq 0 && continue

    echo $'/*\n *  Extracted from '$f$'\n */' >&5
    sed -n '/^LOCAL /,/^{/p' $f | \
      sed 's/)$/);/;s/^{.*//' >&5
  }
done

${do_proto} && {
  cat >&5 <<- _EOF_
	#ifdef REDEF_LOCAL
	#  undef LOCAL
	#  define LOCAL
	#endif
	#endif /* ${marker} */
	_EOF_
  exec 5>&-
}
true
