#! /bin/sh

sedcmd='
s/^\.Sh/.SH/
s/^\.Pp/.PP/

/^\.Bd/s/.Bd.*/.in +4\
.nf/

/^\.Ed/s/.Ed.*/.in -4\
.fi/

/^\.Bl .*\(enum\|tag\)/,/^\.El/{
  s/^\.It.*/.TP\
.B */
}

/^\.Bl .*bullet/,/^\.El/{
  s/.It *\([^ ]*\)/.TP\
\1/
}

'

sed "$sedcmd"
