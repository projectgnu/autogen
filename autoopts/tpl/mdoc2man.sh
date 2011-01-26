#! /bin/sh

sedcmd='
s/^\.Sh/.SH/
s/^\.Pp/.PP/'

sed "$sedcmd"
