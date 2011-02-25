[+: -*- Mode: nroff -*-

  AutoGen5 template mdoc

## agman-cmd.tpl -- Template for command line mdoc pages
##
## Time-stamp:      "2011-02-24 13:27:49 bkorb"
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

:+]
.\"
[+: `set -- \`date '+%B %d %Y' | sed 's/ 0/ /'\`
    echo ".Dd $1 $2, $3"` :+]
.Os [+: `uname -sr`       :+]
.Dt [+: (string-append PROG_NAME " " man-sect " " section-name) :+]
.Sh NAME
.Nm [+: prog-name         :+]
.Nd [+: prog-title        :+][+:

INVOKE build-doc          :+][+:

agmdoc-cmd.tpl ends here  :+]
