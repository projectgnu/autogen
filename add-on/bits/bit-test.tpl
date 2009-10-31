[= AutoGen5 Template -*- Mode: shell-script -*-

sh

=]
#! /bin/bash

script=$(basename $0)

die() {
    exec >&2
    echo "${script} failure:  $*"
    exit 1
}

prog=./[= (base-name) =]
test -x ${prog} || die "${prog} executable not ready"

for bit in \
[=
  (shell (string-append
      "columns -I8 --line=' \\' <<_EOF_\n"
      (join "\n" (stack "bit.b_name"))
      "\n_EOF_"
  ))
=]
do
   txt=$(${prog} ${bit} 2>&1 | fgrep 'yields:')
   case "${txt}" in
   ( *' '${bit} ) : ;;
   ( * ) die "${prog} could not match '${bit}' -- ${txt}" ;;
   esac

   b=$(echo ${bit} | sed 's/.$//')
   txt=$(${prog} ${b} 2>&1 | fgrep 'yields:')
   case "${txt}" in
   ( *' '${bit} ) : ;;
   ( * ) echo "${prog} did not match '${b}' -- ${txt}" ;;
   esac
done
[=

(define bit-list (shell (string-append

"set -- `sort <<_EOF_\n"
(string->c-name! (join "\n" (stack "bit.b_name")))
"\n_EOF_\n`\neval f=\\${$#}
echo ${f}, ${1} | sed 's/.,/,/;s/.$//'"

)))

=]
${prog} '[= (. bit-list) =]' || \
    die "${prog} '[= (. bit-list) =]'"
