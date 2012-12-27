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
    primary_source=$(
        test -f .sdir && cd $(<.sdir) >/dev/null
        git ls-files . | \
        grep -E -v '^(.gitignore|test.sh|README|build-html.sh)$')
    example_source=test.sh
    {
        make
        KEEP_TEST_RESULTS=true make check
    } >/dev/null 2>&1

    example_output=$(echo test-cmap.map test-cmap.h)
    text=$(sed '1,/^PURPOSE:/d;/^[A-Z ]\+:/Q' README)
    ;;
esac
