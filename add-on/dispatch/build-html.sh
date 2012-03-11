#! /bin/bash

cd ${addon_dir}/${dir}

case "${1}" in
( description )
    text=$(sed $'1,/^DESCRIPTION:/d;/^PURPOSE:/Q;/^[ \t]*$/d' dispatch.tpl)
    title=$(echo "${text}" | sed -n 1p)
    text=$(echo "${text}" | sed 1d)
    printf '<h3><a href="#%s">%s</a></h3><p>%s</p>' ${dir} "${title}" "${text}"
    ;;

( files )
    make >&4 2>&4
    primary_source=dispatch.tpl
    example_source=greek.def
    example_output=greek.c\ greek.h
    build_example=Makefile\ test-all.sh
    text=$(sed $'1,/^PURPOSE:/d;/^[A-Z ]\+:/,$d;/^[ \t]*$/d' dispatch.tpl)
    ;;
esac
