#! /bin/sh

case "${VERBOSE}" in
[YytT1-9]* ) set -x ;;
esac
base_name=test-cmap

case "${1}" in
-c | --cl* )
    echo "rm -f ${base_name}*"
    rm -f ${base_name}*
    exit 0
    ;;
esac

set -e

def_guard=TEST_CMAP_DEFINE

rm -f ${base_name}* || :
exec 3> ${base_name}.map
tbl_name=`echo ${base_name} | sed 's/[^a-zA-Z0-9_]/_/g'`

cat >&3 <<- _EOF_
	%guard          ${def_guard}
	%file           ${base_name}.h
	%table          ${tbl_name}
	_EOF_

cat >&3 <<- \_EOF_

	%comment
        This file contains the character classifications
        used by char-mapper to test its proper functioning.
	%

	lower-case      "a-z"
	upper-case      "A-Z"
	alphabetic      +lower-case   +upper-case
	oct-digit       "0-7"
	dec-digit       "89"          +oct-digit
	hex-digit       "a-fA-F"      +dec-digit
	alphanumeric    +alphabetic   +dec-digit
	var-first       "_"           +alphabetic
	variable-name   +var-first    +dec-digit
	option-name     "^-"          +variable-name
	value-name      ":"           +option-name
	horiz-white     "\t "
	compound-name   "[.]"         +value-name   +horiz-white
	whitespace      "\v\f\r\n\b"  +horiz-white
	unquotable      "!-~"         -"\"#(),;<=>[\\]`{}?*'"
	end-xml-token   "/>"          +whitespace
	graphic         "!-~"
	plus-n-space    "+"           +whitespace
	punctuation     "!-~"         -alphanumeric -"_"
	suffix          "-._"         +alphanumeric
	suffix-fmt      "%/"          +suffix
	_EOF_

exec 3>&-
rm -f ag-char-map.h
./char-mapper ${base_name}.map
test -f ${base_name}.h

list=$(sed -n '/^lower-case/,$s/ .*//p' ${base_name}.map | tr -- -a-z _A-Z)

exec 3> ${base_name}.c
cat >&3 <<- _EOF_
	#include <stdio.h>
	#define ${def_guard} 1
	#include "${base_name}.h"

	int main(int argc, char ** argv) {
	  int ch = 0;
	  int const chct = sizeof(${tbl_name})/sizeof(${tbl_name}[0]);
	  for (; ch < chct; ch++) {
	    int has_bit = 0;
	    printf("\n0x%04X (%c):\n", ch, (isprint(ch) ? ch : '?'));
	_EOF_

nl='
'
fmt="    if (IS_%s_CHAR(ch)) {${nl}      fputs(\"  is %s\\\\n\", stdout);\n"
fmt="${fmt}${nl}      has_bit++;${nl}    }\n"

for e in ${list}
do
    printf "${fmt}" $e $e
done >&3
cat >&3 <<- _EOF_
	    if (has_bit == 0)
	      fputs("  is *NOT* a member of any class\n", stdout);
	  }
	  return 0;
	}
	_EOF_
exec 3>&-

${CC:-cc} -o ${base_name} ${base_name}.c
case "$-" in
*x* ) ./${base_name}
      ls -l ${base_name}* ;;
*   ) ./${base_name} >/dev/null
      rm -f ${base_name}* ;;
esac
