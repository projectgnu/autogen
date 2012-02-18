#! /bin/bash

cd ${addon_dir}/${dir}

case "${1}" in
( description )
    title='Create customized character classifications'
    text=$(cat <<- _EOF_
        This program will take a list of character classes and convert them
	into bit map tests.
	_EOF_
        )
    printf '<h3><a href="#%s">%s</a></h3><p>%s</p>' ${dir} "${title}" "${text}"
    return 0
    ;;

( files )
    primary_source=$(echo *.c *.h mk*.sh)
    example_source=test.sh
    make >&4 2>&4
    example_output=$(echo test-cmap.map test-cmap.h)
    build_example=Makefile
    text=$(sed '1,/^PURPOSE:/d;/^[A-Z ]\+:/Q' README)
    ;;
esac
