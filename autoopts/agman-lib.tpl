[+:   -*- nroff -*-

## agman-lib.tpl -- Template for command line man pages
##
##  AutoOpts copyright 1992-2005 Bruce Korb
##
## Time-stamp:      "2005-04-10 09:21:44 bkorb"
## Author:          Jim Van Zandt <jrv@vanzandt.mv.com>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 28 15:35:12 1999
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: agman-lib.tpl,v 4.1 2005/04/10 20:32:23 bkorb Exp $
## ---------------------------------------------------------------------
##
## This "library" converts texi-isms into man-isms.  It gets included
## by the man page template at the point where texi-isms might start appearing
## and then "emit-man-text" is invoked when all the text has been assembled.
##

AutoGen5 template null

:+][+:

    ;; * * * * * * * * * * * * * * * * * * * * * * * * *
    ;;
    ;;  Display the command line prototype,
    ;;  based only on the argument processing type.
    ;;
    ;;  And run the entire output through "sed" to convert texi-isms
    ;;
    (out-push-new)

:+]sed \
 -e   's;@code{\([^}]*\)};\\fB\1\\fP;g' \
 -e    's;@var{\([^}]*\)};\\fB\1\\fP;g' \
 -e   's;@samp{\([^}]*\)};\\fB\1\\fP;g' \
 -e      's;@i{\([^}]*\)};\\fI\1\\fP;g' \
 -e   's;@file{\([^}]*\)};\\fI\1\\fP;g' \
 -e   's;@emph{\([^}]*\)};\\fI\1\\fP;g' \
 -e 's;@strong{\([^}]*\)};\\fB\1\\fP;g' \
 -e 's/@\([{}]\)/\1/g' \
 -e 's,^\$\*$,.br,' \
 -e '/@ *example/,/@ *end *example/s/^/    /' \
 -e 's/^ *@ *example/.nf/' \
 -e 's/^ *@ *end *example/.fi/' \
 -e '/^ *@ *noindent/d' \
 -e '/^ *@ *enumerate/d' \
 -e 's/^ *@ *end *enumerate/.br/' \
 -e '/^ *@ *table/d' \
 -e 's/^ *@ *end *table/.br/' \
 -e 's/^@item \(.*\)/.sp\
.IR "\1"/' \
 -e 's/^@item/.sp 1/' \
 -e 's/\*\([a-zA-Z0-9:~=_ -]*\)\*/\\fB\1\\fP/g' \
 -e 's/``\([a-zA-Z0-9:~+=_ -]*\)'"''"'/\\fI\1\\fP/g' \
 -e 's/^@\*/.br/' <<'_End_Of_Man_'
[+:

DEFINE emit-man-text    :+]
_End_Of_Man_[+:

(shell (out-pop #t) )   :+][+:

ENDDEF emit-man-text    :+][+: #

agman-lib.tpl ends here :+]
