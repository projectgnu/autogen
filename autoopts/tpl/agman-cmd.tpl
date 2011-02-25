[+: -*- Mode: nroff -*-

  AutoGen5 template man

## agman-cmd.tpl -- Template for command line man pages
##
## Time-stamp:      "2011-02-24 15:16:00 bkorb"
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following md5sums:
##
##  43b91e8ca915626ed3818ffb1b71248b COPYING.gplv3
##  06a1a2e4760c90ea5e1dad8dfaac4d39 COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f COPYING.mbsd

# Produce a man page for section 1, 5 or 8 commands.
# Which is selected via:  -DMAN_SECTION=n
# passed to the autogen invocation.  "n" may have a suffix, if desired.
#
:+][+:

INCLUDE "cmd-doc.tlib"

:+][+:

(sprintf "\n.TH %s %s \"%s\" \"%s\" \"%s\"\n"
        (get "prog-name") man-sect
        (shell "date '+%d %b %Y'") package-text section-name)
:+]
.\"
.SH NAME
[+: prog-name :+] \- [+: prog-title :+][+:

(out-push-new)            :+][+:

INVOKE build-doc          :+][+:

  (shell (string-append
    "fn='" (find-file "mdoc2man.sh") "'\n"
    "test -f ${fn} || die mdoc2man not found from $PWD\n"
    "${fn} <<\\_EndOfMdoc_ || die ${fn} failed in $PWD\n"
    (out-pop #t)
    "\n_EndOfMdoc_" ))    :+][+: #

agman-cmd.tpl ends here   :+]
