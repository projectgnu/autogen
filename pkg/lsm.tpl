[=autogen template lsm=LSM =]
Begin3
Title:          AutoGen - [=prog_title=]

Version:        [=version=]

Entered-date:   [=`date +%d%b%y`=]

Description:    AutoGen is a tool for automatically generating
                arbitrary text files that contain repetitive text
                with varying substitutions.  This is particularly
                useful if you have several types of repetitive
                text that all need to be kept in sync with each
                other.  The goal is to try to simplify the process
                of maintaining repetitive program text.

                Included with autogen is a tool that virtually
                eliminates the hassle of processing options,
                keeping usage text up to date and so on.  This
                package allows you to specify several program
                attributes, innumerable options and option
                attributes, then it produces all the code
                necessary to parse and handle the command line and
                initialization file options.
                For a more complete description, @xref{features}.

Keywords:       macro, m4, cpp, code generation, preprocessor,
                options, getopts

Author:         autogen@linuxbox.com (Bruce Korb)
                gary@oranda.demon.co.uk (Gary V. Vaughan)

Maintained-by:  autogen@linuxbox.com (Bruce Korb)

Primary-site:   sunsite.unc.edu /pub/Linux/devel/lang/macro/
                [= `
cd $top_builddir
set -- autogen*.gz
if [ $# -gt 1 ]
then shift \`expr $# - 1\` ; fi
if [ ! -f $1 ]
then ct=564
else ct="\`expr \\\\( \\\`wc -c < $1\\\` + 1023 \\\\) / 1024\`"
fi
echo $ct KB $1
` =]

Alternate-site:

Original-site:

Platforms:      gunzip and ANSI-C

Copying-policy: GPL and LGPL

End
