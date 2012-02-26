#! /bin/sh

case "${VERBOSE}" in
[YytT1-9]* ) set -x ;;
esac
base_name=test-cmap
rm -f ${base_name}*

case "${1}" in
-c | --cl* )
    echo "rm -f ${base_name}*"
    exit 0
    ;;
esac

rm -f test[-_]*
set -e

cat > ${base_name}.map <<- _EOF_
	%file ${base_name}.h
	%test
	%comment
	   Copyright (c) 2000, 2001 Free Software Foundation, Inc.

	This file is part of some library.
	%

	# Basic types:
	#
	cntrl       "\x00-\x1F\x7F"
	digit       "0-9"
	lower       "a-z"
	horiz-space " \t"
	line-end    "\r\n"
        vert-space  "\f\v"
	upper       "A-Z"

	# Compound char types.  Only these may have subtracted names.
	#
	xdigit      "a-fA-F"    +digit
	token-end   "\x00"      +horiz-space +line-end
	name-start  "_"         +upper +lower
	punctuation "\x20-\x7E" -upper -lower -horiz-space

	# pure combined char types:
	#
	space       +horiz-space +line-end +vert-space

	%emit
	#define HOST_CHARSET_UNKNOWN 0
	#define HOST_CHARSET_ASCII   1
	#define HOST_CHARSET_EBCDIC  2

	#if  '\n' == 0x0A && ' ' == 0x20 && '0' == 0x30 \\
	   && 'A' == 0x41 && 'a' == 0x61 && '!' == 0x21
	#  define HOST_CHARSET HOST_CHARSET_ASCII
	#else
	# if '\n' == 0x15 && ' ' == 0x40 && '0' == 0xF0 \\
	   && 'A' == 0xC1 && 'a' == 0x81 && '!' == 0x5A
	#  define HOST_CHARSET HOST_CHARSET_EBCDIC
	# else
	#  define HOST_CHARSET HOST_CHARSET_UNKNOWN
	# endif
	#endif
	%
	_EOF_

./char-mapper ${base_name}.map
bash ./${base_name}.h
test "X${KEEP_TEST_RESULTS}" = Xtrue || \
    rm -f ${base_name}*
