#! /bin/bash

die() {
  echo "fatal mk-all error:  $*" >&2
  exit 1
}

test_comp() {
  test_nm=test-greek
  exec 3> ${test_nm}.c
  cat >&3 <<- _EOF_
	#include <stdio.h>
	#include "greek.c"
	_EOF_
  g=greek-hdlr.c
  handler=$(test -f ${g} && echo true || echo false)
  if ${handler}
  then
    echo '#include "greek-hdlr.c"' >&3
  else
    cat >&3 <<- _EOF_
	int main( int argc, char** argv ) {
	    while (--argc > 0) {
	        char const * pz = *++argv;
	        int          vl = greek_enum(pz, NULL);
	        char const * pr = greek_name(vl);
	        printf("%-6s--> %2d (%s)\n", pz, vl, pr);
	    }
	    return 0; }
	_EOF_
  fi
  exec 3>&-

  cc -g -o ${test_nm} ${test_nm}.c || exit 1

  ./${test_nm} ${test_args} > ${test_nm}.out

  if ${handler}
  then
    cmp ${test_nm}.out handler.out || exit 1
  else
    cmp ${test_nm}.out recognize.out || exit 1
  fi

  args=
  ./${test_nm} ${test_abbrev_args} > ${test_nm}-abbrev.out

  if test ${#pt} -gt 0
  then
    if ${handler}
    then
      clist="${test_nm}-abbrev.out invalid-abbrev.out"

    else
      clist="${test_nm}-abbrev.out invalid-handler-abbrev.out"
    fi

  else
    if ${handler}
    then
      clist="${test_nm}-abbrev.out handler-abbrev.out"

    else
      clist="${test_nm}-abbrev.out recognize-abbrev.out"
    fi
  fi

  cmp $clist || {
    diff -u $clist
    exit 1
  }
}

cleanup() {
  rm -f greek*.[ch] *.out test-greek*
}

init() {
  cleanup
  test_args='junk alpha beta gamma delta omega help'
  test_abbrev_args=$(for f in ${test_args}
    do echo $f | sed 's/\(..\).*/\1/' ; done)

  cat > handler.out <<- \_EOF_
	Invalid command:  'junk'
	alpha
	beta
	gamma
	delta
	omega
	alpha   this is the first greek letter
	beta    beta customers are helpful
	omega   this is the end.
	help    The help text skips secret commands 'gamma' and 'delta'.
	_EOF_

  cat > recognize.out <<- \_EOF_
	junk  -->  0 (** INVALID **)
	alpha -->  1 (alpha)
	beta  -->  2 (beta)
	gamma -->  3 (gamma)
	delta -->  4 (delta)
	omega -->  5 (omega)
	help  -->  6 (help)
	_EOF_

  cat > handler-abbrev.out <<- \_EOF_
	Invalid command:  'ju'
	alpha
	beta
	gamma
	delta
	omega
	alpha   this is the first greek letter
	beta    beta customers are helpful
	omega   this is the end.
	help    The help text skips secret commands 'gamma' and 'delta'.
	_EOF_

  cat > recognize-abbrev.out <<- \_EOF_
	ju    -->  0 (** INVALID **)
	al    -->  1 (alpha)
	be    -->  2 (beta)
	ga    -->  3 (gamma)
	de    -->  4 (delta)
	om    -->  5 (omega)
	he    -->  6 (help)
	_EOF_

  cat > invalid-abbrev.out <<- \_EOF_
	Invalid command:  'ju'
	Invalid command:  'al'
	Invalid command:  'be'
	Invalid command:  'ga'
	Invalid command:  'de'
	Invalid command:  'om'
	Invalid command:  'he'
	_EOF_

  cat > invalid-handler-abbrev.out <<- \_EOF_
	ju    -->  0 (** INVALID **)
	al    -->  0 (** INVALID **)
	be    -->  0 (** INVALID **)
	ga    -->  0 (** INVALID **)
	de    -->  0 (** INVALID **)
	om    -->  0 (** INVALID **)
	he    -->  0 (** INVALID **)
	_EOF_

  export EMIT_DISPATCH=yes
}

test "X${1}" = X-cleanup && {
  cleanup
  exit 0
}
init

for st in "" 'static = handler;'
do

  for gp in "" no-gperf\;
  do

    for pt in "" no-partial\;
    do
      rm -f greek*.[ch]

      echo "${st:-extern} ${gp:-gperf} ${pt:-partial-matching}"
      (cat greek.def
       echo "${st} ${gp} ${pt}"
      ) | autogen -bgreek || exit 1

      test_comp
    done
  done
done

for st in "" static\;
do

  for gp in "" no-gperf\;
  do

    for pt in "" no-partial\;
    do
      rm -f greek*.[ch]

      echo "${st:-extern} ${gp:-gperf} ${pt:-partial-matching}"

      (sed '/^dispatch /,$d' greek.def
       echo "${st} ${gp} ${pt}"
      ) | autogen -bgreek || exit 1

      test_comp
    done
  done
done
cleanup
