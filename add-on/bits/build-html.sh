#! /bin/bash

cd ${addon_dir}/${dir}

case "${1}" in
( description )
    text=$(sed $'1,/^DESCRIPTION:/d;/^PURPOSE:/Q;/^[ \t]*$/d' bits.tpl)
    title=$(echo "${text}" | sed -n 1p)
    text=$(echo "${text}" | sed 1d)
    printf '<h3><a href="#%s">%s</a></h3><p>%s</p>' ${dir} "${title}" "${text}"
    return 0
    ;;

( files )
    primary_source=bits.tpl
    example_source=$(echo b-test.def bit-test.tpl)
    make >&4 2>&4
    example_output=$(echo b-test.[ch])
    build_example=Makefile
    text=$(sed $'1,/^PURPOSE:/d;/^[A-Z ]\+:/,$d;/^[ \t]*$/d' bits.tpl)
    ;;
esac
