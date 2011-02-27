#! /bin/sh

sedcmd='
s/^\.Sh/.SH/
s/^\.Pp/.PP/

/^.Nm /{
  y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/
  s/-/\\-/g
  s/^.nm/.B/
}

/^\.Bd/s/.Bd.*/.in +4\
.nf/

/^\.Ed/s/.Ed.*/.in -4\
.fi/

/^\.Bl .*\(enum\|tag\)/,/^\.El/{
  s/^\.It  */.TP\
.BR /
  /^\.[BE]l/d
}

/^\.Bl .*bullet/,/^\.El/{
  s/.It *\([^ ]*\)/.TP\
\1/
  /^\.[BE]l/d
}

s/^\.Op Ar  */.BR /
'

sed "$sedcmd"
