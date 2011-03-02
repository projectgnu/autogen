[+: -*- Mode: nroff -*-

  AutoGen5 template man

## agman-cmd.tpl -- Template for command line man pages
##
## Time-stamp:      "2011-03-02 13:07:42 bkorb"
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

(define head-line (lambda()
        (sprintf ".TH %s %s \"%s\" \"%s\" \"%s\"\n.\\\"\n"
                (get "prog-name") man-sect
        (shell "date '+%d %b %Y'") package-text section-name) ))

:+][+:

INCLUDE "cmd-doc.tlib"

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

.\" = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
.\"  S Y N O P S I S
.\" = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = :+][+:

DEFINE synopsis

:+]
.Sh SYNOPSIS
.Nm [+: (. PROG_NAME) :+][+:

  IF (define use-flags  (exist? "flag.value"))
     (define named-mode (not (or use-flags (exist? "long-opts") )))
     use-flags
                                            :+][+:
    IF (exist? "long-opts")                 :+]
.\" Mixture of short (flag) options and long options
.RB [ \-\fIflag\fP " [\fIvalue\fP]]... [" \-\-\fIopt\-name\fP[+:
#:+] " [[=| ]\fIvalue\fP]]..."[+:

    ELSE no long options:                   :+]
.\" Short (flag) options only
.RB [ \-\fIflag\fP " [\fIvalue\fP]]..."[+:
    ENDIF                                               
                                            :+][+:
  ELIF (exist? "long-opts")                             
                                            :+]
.\" Long options only
.RB [ \-\-\fIopt\-name\fP [ = "| ] \fIvalue\fP]]..."[+:

  ELIF  (not (exist? "argument"))           :+]
.RI [ opt\-name "[\fB=\fP" value ]]...
.PP
All arguments are named options.[+:
  ENDIF                                     :+][+:

  IF (exist? "argument")
    :+] [+: argument                        :+][+:

    IF (exist? "reorder-args")              :+]
.PP
Operands and options may be intermixed.  They will be reordered.
[+: ENDIF                                   :+][+:

  ELIF (or (exist? "long-opts") use-flags) 

:+]
.PP
All arguments must be options.[+:

  ENDIF                                     :+][+:

(if (exist? "explain")
    (string-append "\n.PP\n"
      (join "\n.PP\n" (stack "explain"))) ) :+][+:

ENDDEF synopsis

agman-cmd.tpl ends here   :+]
