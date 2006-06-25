[= AutoGen5 template

##  Documentation template
##
##  AutoGen Copyright (C) 1992-2006 Bruce Korb
##
## Author:            Bruce Korb <bkorb@gnu.org>
## Maintainer:        Bruce Korb <bkorb@gnu.org>
## Created:           Tue Sep 15 13:13:48 1998
## Last Modified:     Mar 4, 2001
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: auto_gen.tpl,v 4.26 2006/06/25 00:51:11 bkorb Exp $
## ---------------------------------------------------------------------

texi=autogen.texi

# Set up some global Scheme variables
#
(setenv "SHELL" "/bin/sh")

(define temp-dir (shell "
    tempdir=`(mktemp -d ./.ag-XXXXXX) 2>/dev/null`
    test -z \"${tempdir}\" && tempdir=.ag-$$.dir
    test -d ${tempdir}  || mkdir ${tempdir} || die cannot mkdir ${tempdir}
    tmpdir=`cd ${tmpdir} ; pwd` || die cannot cd ${tempdir}
    echo ${tempdir}" ))

(define texi-file-source (shell "
    if [ -f autogen-texi.txt ]
    then
      echo autogen-texi.txt
    elif [ -f ${top_srcdir}/doc/autogen-texi.txt ]
    then
      echo ${top_srcdir}/doc/autogen-texi.txt
    else
      die Cannot locate original autogen-texi.txt file
    fi" ))

(define texi-escape-encode (lambda (in-str)
   (string-substitute in-str
      '("@"   "{"   "}")
      '("@@"  "@{"  "@}")
)  ))

(define temp-txt  "")
(define text-tag  "")
(define func-name "")
(define func-str  "")
(define func-name "")
(define func-str  "")

=][= # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

\=]
\input texinfo
@ignore
\internalpagesizes{46\baselineskip}{6in}{-.25in}{-.25in}{\bindingoffset}{36pt}%
@end ignore
@c %**start of header
@setfilename [=(base-name)=].info
@settitle AutoGen - [= prog-title =]
@setchapternewpage off
@c %**end of header
@copying
This manual is for GNU AutoGen version [= `
  UPDATED=\`date '+%B %Y'\`
  echo ${AG_REVISION}, updated ${UPDATED}` =].

Copyright @copyright{} [= copyright.date =] by Bruce Korb.

@quotation
Permission is granted to copy, distribute and/or modify this document under
the terms of the GNU Free Documentation License, Version 1.1 or any later
version published by the Free Software Foundation; with no Invariant
Sections, with the Front-Cover Texts being ``A GNU Manual,'' and with the
Back-Cover Texts as in (a) below.  A copy of the license is included in the
section entitled ``GNU Free Documentation License.''

(a) The FSF's Back-Cover Text is: ``You have freedom to copy and modify this
GNU Manual, like GNU software.  Copies published by the Free Software
Foundation raise funds for GNU development.''
@end quotation
@end copying

@ignore
[=(set-writable) (dne "")=]

Plus bits and pieces gathered from all over the source/build
directories:
[= ` for f in ${DOC_DEPENDS} ; do echo "    $f" ; done ` =]

@end ignore

@dircategory GNU programming tools
@direntry
* AutoGen: (autogen).         [= prog-title =]
@end direntry

@ifinfo
@ifnothtml
This file documents [= package =] Version [=`echo ${AG_REVISION}`=].

AutoGen copyright @copyright{} [= copyright.date =] Bruce Korb
AutoOpts copyright @copyright{} [= copyright.date =] Bruce Korb
snprintfv copyright @copyright{} 1999-2000 Gary V. Vaughan

[=(gpl "AutoGen" "")=]

@ignore
Permission is granted to process this file through TeX and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph.
@end ignore
@end ifnothtml
@end ifinfo

@finalout
@titlepage
@title AutoGen - [= prog-title =]
@subtitle For version [=`
  echo ${AG_REVISION}, ${UPDATED} `=]
@author Bruce Korb
@author @email{[= (texi-escape-encode "bkorb@gnu.org") =]}

@page
@vskip 0pt plus 1filll
AutoGen copyright @copyright{} [= copyright.date =] Bruce Korb
@sp 2
This is the second edition of the GNU AutoGen documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=(gpl "AutoGen" "")=]
@end titlepage

@ifinfo

@node Top, Introduction, , (dir)
@top The Automated Program Generator
@comment  node-name,  next,  previous,  up

This file documents AutoGen version [=`
  echo ${AG_REVISION}`=].  It is a tool designed
for generating program files that contain repetitive text with varied
substitutions.  This document is very long because it is intended as a
reference document.  For a quick start example, @xref{Example Usage}.

The AutoGen distribution includes the basic generator engine and
several add-on libraries and programs.  Of the most general interest
would be Automated Option processing, @xref{AutoOpts}, which also
includes stand-alone support for configuration file parsing, @xref{Features}.
Please see the ``Add-on packages for AutoGen'' section for additional
programs and libraries associated with AutoGen.

This edition documents version [=`echo ${AG_REVISION}, ${UPDATED}.`=]
[=

INVOKE  get-text tag = main

=]
[=
FOR directive =][=
  (if (exist? "dummy")
      (string-downcase! (sprintf "@samp{#%s}, " (get "name")))) =][=
ENDFOR directive=] and @samp{#if}.
Note that when ignoring the @code{#if} directive, all intervening
text through its matching @code{#endif} is also ignored,
including the @code{#else} clause.

The AutoGen directives that affect the processing of
definitions are:

@table @code[=
FOR directive "\n" =][=
  IF (exist? "text") =]
@item #[=% name (string-downcase! "%s") =][= % arg " %s" =]
@cindex #[=% name (string-downcase! "%s") =]
@cindex [=% name (string-downcase! "%s") =] directive
[= id-file =]
[=text     =][=
  ENDIF    =][=
ENDFOR directive=]
@end table
[=

INVOKE   get-text tag = COMMENTS

=]
@node    Full Syntax
@section Finite State Machine Grammar

The preprocessing directives and comments are not part of the grammar.  They
are handled by the scanner/lexer.  The following was extracted directly from
the generated defParse-fsm.c source file.  The "EVT:" is the token seen,
the "STATE:" is the current state and the entries in this table describe
the next state and the action to take.  Invalid transitions were removed
from the table.

@ignore
Extracted from $top_srcdir/agen5/defParse.y
@end ignore
@example
[= `

if test -z "$top_srcdir" || test ! -d "$top_srcdir"
then top_srcdir=.. ; fi
f=${top_srcdir}/agen5/defParse-fsm.c
test -f ${f} || die missing generated file: $f
sed -n -e '/^dp_trans_table/,/^};$/p' ${f} | \
  sed '/ \&dp_do_invalid /d;/^ *}/d;s/@/@@/g;s/{/@{/g;s/}/@}/g'

` =]
@end example
[=

INVOKE  get-text tag = TEMPLATE

=]
@ignore

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

@end ignore
@page
@node AutoGen Functions
@section AutoGen Scheme Functions

AutoGen uses Guile to interpret Scheme expressions within AutoGen
macros.  All of the normal Guile functions are available, plus several
extensions (@pxref{Common Functions}) have been added to
augment the repertoire of string manipulation functions and
manage the state of AutoGen processing.

This section describes those functions that are specific to AutoGen.
Please take note that these AutoGen specific functions are not loaded
and thus not made available until after the command line options have
been processed and the AutoGen definitions have been loaded.  They may,
of course, be used in Scheme functions that get defined at those times,
but they cannot be invoked.

@menu[=

FOR gfunc    =][=
  (if (not (exist? "name"))
      (error "NO NAME"))          =][=

  IF (not (exist? "general_use")) =][=
    INVOKE emit-menu-entry        =][=
  ENDIF      =][=

ENDFOR gfunc =]
* SCM autogen-version::     @file{autogen-version} - ``[= version =]''
* SCM c-file-line-fmt::     format file info as, ``@code{#line nn "file"}''
@end menu

[=
FOR gfunc =][=
  IF (not (exist? "general_use")) =][=
    INVOKE emit-scm-func          =][=
  ENDIF general_use               =][=
ENDFOR gfunc
=]
@ignore
Generated [= (tpl-file-line) =].
@end ignore

@node SCM autogen-version
@subsection @file{autogen-version} - autogen version number
@findex autogen-version

This is a symbol defining the current AutoGen version number string.
It was first defined in AutoGen-5.2.14.
It is currently ``[= version =]''.

@node SCM c-file-line-fmt
@subsection format file info as, ``@code{#line nn "file"}''
@findex c-file-line-fmt

This is a symbol that can easily be used with the functions
@xref{SCM tpl-file-line}, and @xref{SCM def-file-line}.
These will emit C program @code{#line} directives pointing to template
and definitions text, respectively.
@ignore

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

@end ignore
@page
@node Common Functions
@section Common Scheme Functions

This section describes a number of general purpose functions that make
the kind of string processing that AutoGen does a little easier.
Unlike the AutoGen specific functions (@pxref{AutoGen Functions}),
these functions are available for direct use during definition load time.
The equality test (@pxref{SCM =}) is ``overloaded'' to do string equivalence
comparisons.  If you are looking for inequality, the Scheme/Lisp way
of spelling that is, ``(not (= ...))''.

@menu[=

FOR gfunc                   =][=

  IF (exist? "general_use") =][=
    INVOKE emit-menu-entry  =][=
  ENDIF                     =][=

ENDFOR gfunc

=]
@end menu

[=

FOR gfunc =][=
  IF (exist? "general_use") =][=
    INVOKE emit-scm-func    =][=
  ENDIF general_use         =][=
ENDFOR gfunc

=][=

INVOKE  get-text tag = MACROS

=]
@menu
* AGMacro syntax::   AutoGen Macro Syntax[=
FOR macfunc =][=
  IF (exist? "desc") =][=
    (sprintf "\n* %-18s %s - %s"
       (string-append  (get "name") "::" )
       (string-upcase! (get "name"))
       (get "what") ) =][=
  ENDIF =][=
ENDFOR macfunc=]
@end menu
@node AGMacro syntax
@subsection AutoGen Macro Syntax
@cindex macro syntax

The general syntax is:

@example
[ @{ <native-macro-name> | <user-defined-name> @} ] [ <arg> ... ]
@end example

@noindent
The syntax for @code{<arg>} depends on the particular macro,
but is generally a full expression (@pxref{expression syntax}).
Here are the exceptions to that general rule:

@enumerate
@item
@code{INVOKE} macros, implicit or explicit, must be followed by
a list of name/string value pairs.  The string values are
@i{simple expressions}, as described above.

That is, the @code{INVOKE} syntax is one of these two:
@example
<user-macro-name> [ <name> [ = <expression> ] ... ]

INVOKE <name-expression> [ <name> [ = <expression> ] ... ]
@end example

@item
AutoGen FOR macros must be in one of three forms:

@example
FOR <name> [ <separator-string> ]

FOR <name> (...Scheme expression list)

FOR <name> IN <string-entry> [ ... ]
@end example
@noindent
where:
@table @samp
@item <name>
must be a simple name.
@item <separator-string>
is inserted between copies of the enclosed block.  Do not try to use ``IN''
as your separator string.  It won't work.
@item <string-entry>
is an entry in a list of strings.  ``@code{<name>}'' is assigned
each value from the ``@code{IN}'' list before expanding the @code{FOR} block.
@item (...Scheme expression list)
is expected to contain one or more of the @code{for-from},
@code{for-to}, @code{for-by}, and @code{for-sep} functions.
(@xref{FOR}, and @ref{AutoGen Functions})
@end table

The first two forms iterate over the @code{FOR} block if @code{<name>}
is found in the AutoGen values.  The last form will create the AutoGen
value named @code{<name>}.

@item
AutoGen @code{DEFINE} macros must be followed by a simple name.
Anything after that is ignored.  Consequently, that ``comment space''
may be used to document any named values the macro expects to have
set up as arguments.  @xref{DEFINE}.

@item
The AutoGen @code{COMMENT}, @code{ELSE}, @code{ESAC} and the @code{END*}
macros take no arguments and ignore everything after the macro name
(e.g. see @ref{COMMENT})
@end enumerate[=


#  FOR each defined function,
      this code will insert the extracted documentation =][=

FOR macfunc =][=
  IF (exist? "desc") =]

@node [=name=]
@subsection [=% name (string-upcase! "%s") =] - [=what=]
[= id-file =]
@findex [=% name (string-upcase! "%s") =][=
  (if (exist? "cindex")
      (string-append "\n@cindex "
         (join "\n@cindex " (stack "cindex")) ))  =]

[=desc=][=
  ENDIF desc exists =][=
ENDFOR macfunc=]
[=

INVOKE  get-text tag = augmenting

=]
@ignore

Invocation section from [= `

f=../agen5/autogen.texi
[ -f $f ] || f=${top_srcdir}/agen5/autogen.texi

cat <<_EOF_
${f}

@end ignore
@page

_EOF_

cat ${f}` =]
[=

INVOKE  get-text tag = installation

=]
@page
@node AutoOpts
@chapter Automated Option Processing
@cindex autoopts

AutoOpts [=`
eval "\`egrep '^AO_[A-Z]*=' ${top_srcdir}/VERSION\`" 2> /dev/null
echo ${AO_CURRENT}.${AO_REVISION}
`=] is bundled with AutoGen.  It is a tool that virtually eliminates the
hassle of processing options and keeping man pages, info docs and usage text
up to date.  This package allows you to specify several program attributes, up
to a hundred option types and many option attributes.  From this, it then
produces all the code necessary to parse and handle the command line and
configuration file options, and the documentation that should go with your
program as well.[=

INVOKE  get-text tag = autoopts

=]
@noindent
First, specify your program attributes and its options to AutoOpts,
as with the following example.

@example
[=

(out-push-new (string-append temp-dir "/checkopt.def" ))

=]AutoGen Definitions options;
prog-name     = check;
prog-title    = "Checkout Automated Options";
long-opts;

main = { main-type = shell-process; };

flag = {
    name      = check-dirs;
    value     = L;        /* flag style option character */
    arg-type  = string;   /* option argument indication  */
    max       = NOLIMIT;  /* occurrence limit (none)     */
    stack-arg;            /* save opt args in a stack    */
    descrip   = "Checkout directory list";
};

flag = {
    name      = show_defs;
    descrip   = "Show the definition tree";
    disable   = dont;     /* mark as enable/disable type */
                          /* option.  Disable as `dont-' */
};[=

(texi-escape-encode (out-pop #t)) =]
@end example

@noindent
Then perform the following steps:[= #

Developer note:  the following only works when AutoGen has been installed.
Since this may be being built on a system where it has not been installed,
the code below ensures we are running out tools out of the build directory =]

@enumerate
@item
@code{cflags="-DTEST_CHECK_OPTS `autoopts-config cflags`"}
@item
@code{ldflags="`autoopts-config ldflags`"}
@item
@code{autogen checkopt.def}
@item
@code{cc -o check -g $@{cflags@} checkopt.c $@{ldflags@}}
@item
@code{./check --help}
@end enumerate

@noindent
Running those commands yields:

@example
[= (texi-escape-encode (shell
"cd ${tempdir}
test -f checkopt.def || die cannot locate checkopt.def
test -f check && rm -f check
cat >> checkopt.def <<- _EOF_
	include = '#include \"compat/compat.h\"';
	_EOF_
(
  ${AGexe} -L${top_srcdir}/autoopts checkopt.def
  opts=\"-o check -DTEST_CHECK_OPTS ${CFLAGS} ${INCLUDES}\"
  ${CC} ${opts} checkopt.c ${LIBS}
) > checkopt.err 2>&1

test -x ./check || {
  cat checkopt.err >&2
  die cannot create checkopt program
}
./check --help | sed 's/\t/        /g'"
) ) =]
@end example
[=

INVOKE  get-text tag = autoopts-main

=]
Here is an example program that uses the following set of definitions:

@example
[=

 (out-push-new (string-append temp-dir "/default-test.def" ))

=]AutoGen Definitions options;

prog-name  = default-test;
prog-title = 'Default Option Example';
homerc     = '$$/../share/default-test', '$HOME', '.';
environrc;
long-opts;
gnu-usage;
version    = '1.0';
main = {
  main-type = shell-process;
};
#define DEBUG_FLAG
#define WARN_FLAG
#define WARN_LEVEL
#define VERBOSE_FLAG
#define VERBOSE_ENUM
#define DRY_RUN_FLAG
#define OUTPUT_FLAG
#define INPUT_FLAG
#define DIRECTORY_FLAG
#define INTERACTIVE_FLAG
#include stdoptions.def
[=

 (texi-escape-encode (out-pop #t))

=]@end example

@noindent
Running a few simple commands on that definition file:

@example
autogen default-test.def
copts="-DTEST_DEFAULT_TEST_OPTS `autoopts-config cflags`"
lopts="`autoopts-config ldflags`"
cc -o default-test $@{copts@} default-test.c $@{lopts@}
@end example

@noindent
Yields a program which, when run with @file{--help}, prints out:

@example
[= (shell (string-append "
OPTDIR=`cd ${top_builddir}/autoopts >/dev/null && pwd`
TOPDIR=`cd ${top_builddir} >/dev/null ; pwd`
libs=`cd ${OPTDIR} >/dev/null ; [ -d .libs ] && cd .libs >/dev/null ; pwd`

if [ -f ${libs}/libopts.a ]
then libs=\"${libs}/libopts.a\"
else libs=\"-L ${libs} -lopts\"
fi
libs=\"${libs} ${LIBS}\"

exec 3>&1
(
  cd ${tempdir}
  echo 'config-header = \"config.h\";' >> default-test.def
  HOME='' ${AGexe} -L${OPTDIR} -L${top_srcdir}/autoopts default-test.def
  test -f default-test.c || die 'NO default-test.c PROGRAM'

  opts=\"-o default-test -DTEST_DEFAULT_TEST_OPTS ${INCLUDES}\"
  ${CC} ${CFLAGS} ${opts} default-test.c ${libs}

  test -x ./default-test || die 'NO default-test EXECUTABLE'
) > ${tempdir}/default-test.log 2>&1

test $? -eq 0 || {
  die Check ${tempdir}/default-test.log file
}
HOME='$HOME/.default_testrc' ${tempdir}/default-test --help | \
   sed 's,\t,        ,g;s,\\([@{}]\\),@\\1,g'

exec 3>&-" ))
=]
@end example
[=

INVOKE  get-text tag = autoopts-api

=]
[=`

f=../autoopts/libopts.texi
[ ! -f $f ] && f=${top_srcdir}/autoopts/libopts.texi
test -f $f die "Cannot locate libopts.texi"
cat $f

`=]
[=

INVOKE  get-text tag = "autoopts-data"

=]
@example[=
(out-push-new (string-append temp-dir "/hello.c"))

=]
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <autoopts/options.h>
int main( int argc, char** argv ) {
  const char* greeting = "Hello";
  const char* greeted  = "World";
  const tOptionValue* pOV = configFileLoad( "hello.conf" );

  if (pOV != NULL) {
    const tOptionValue* pGetV = optionGetValue( pOV, "greeting" );

    if (  (pGetV != NULL)
       && (pGetV->valType == OPARG_TYPE_STRING))
      greeting = strdup( pGetV->v.strVal );

    pGetV = optionGetValue( pOV, "personalize" );
    if (pGetV != NULL) {
      struct passwd* pwe = getpwuid( getuid() );
      if (pwe != NULL)
        greeted = strdup( pwe->pw_gecos );
    }

    optionUnloadNested( pOV ); /* deallocate config data */
  }
  printf( "%s, %s!\n", greeting, greeted );
  return 0;
}
[= (texi-escape-encode (out-pop #t)) =]
@end example

@noindent
With that text in a file named ``hello.c'', this short script:

@example
cc -o hello hello.c `autoopts-config cflags ldflags`
./hello
echo 'greeting Buzz off' > hello.conf
./hello
echo personalize > hello.conf
./hello
@end example

@noindent
will produce the following output (for me):

@example
[= (texi-escape-encode (shell "
cd ${tempdir}
cc -o hello hello.c ${CFLAGS} ${LIBS} ${LDFLAGS} || \
  die cannot compile hello
./hello
echo 'greeting Buzz off' > hello.conf
./hello
echo personalize > hello.conf
./hello"
)) =]
@end example
[=

INVOKE  get-text tag = "ao-data2"

=]
[=

 (out-push-new)

=]fn() {
    cd ${top_builddir}/autoopts/test || return
    VERBOSE=true
    export VERBOSE
    ${MAKE} check TESTS=errors.test > /dev/null 2>&1
    if test ! -x testdir/errors
    then
      return
    fi
    cat <<-EOF

	Here is the usage output example from AutoOpts error handling
	tests.  The option definition has argument reordering enabled:

	@example
EOF

    ./testdir/errors -? | sed 's,	,        ,g;s,\([@{}]\),@\1,g'
    cmd='errors operand1 -s first operand2 -X -- -s operand3'
    cat <<-EOF
	@end example

	Using the invocation,
	@example
	  test-${cmd}
	@end example
	you get the following output for your shell script to evaluate:

	@example
	`testdir/${cmd}`
	@end example
EOF
}
fn
[=

(shell (out-pop #t))

=]
@node script-parser
@subsection Parsing with a Portable Script

If you had used @code{test-main = optionParseShell} instead, then you can,
at this point, merely run the program and it will write the parsing
script to standard out.  You may also provide this program with command
line options to specify the shell script file to create or edit, and you
may specify the shell program to use on the first shell script line.
That program's usage text would look something like the following
and the script parser itself would be very verbose:

@example
[= `

log=${tempdir}/genshellopt.log

(
  set -x
  opts="-o genshellopt -DTEST_GETDEFS_OPTS ${INCLUDES}"
  exec 3> ${tempdir}/genshellopt.def
  cat ${top_srcdir}/getdefs/opts.def >&3
  echo "test_main = 'optionParseShell';" >&3
  echo 'config-header = "config.h";' >&3
  exec 3>&-

  cd ${tempdir}
  cmd="valgrind --leak-check=full ${AGexe}"
  HOME='' ${cmd} -t40 -L${OPTDIR} -L${top_srcdir}/autoopts genshellopt.def
  test $? -eq 0 || die "autogen failed to create genshellopt.c - See ${log}"

  ${CC} ${CFLAGS} ${opts} genshellopt.c ${libs}
  test $? -eq 0 || die "could not compile genshellopt.c - See ${log}"
) > ${log} 2>&1

` =][= `

test -x ${tempdir}/genshellopt || \
  die "NO GENSHELLOPT PROGRAM - See ${log}"

${tempdir}/genshellopt --help > ${tempdir}/genshellopt.hlp

` =][= `

${tempdir}/genshellopt -o ${tempdir}/genshellopt.sh || \
  die cannot create ${tempdir}/genshellopt.sh

` =][= `

sedcmd='s,\t,        ,g;s,\\([@{}]\\),@\\1,g'
sed "${sedcmd}" ${tempdir}/genshellopt.hlp
cat <<- \_EOF_
	@end example

	@noindent
	Resulting in the following script:
	@example
	_EOF_

sed "${sedcmd}" ${tempdir}/genshellopt.sh

` =]
@end example
[=

INVOKE  get-text tag = autoinfo

=]
@menu
* AutoFSM::                        Automated Finite State Machine
* AutoXDR::                        Combined RPC Marshalling
* AutoEvents::                     Automated Event Management
[=`cat  ${ADDON_MENU}`=]
@end menu
[=

INVOKE  get-text tag = autofsm

=][= `

echo

for f in ${ADDON_TEXI}
do
   echo '@page'
   echo '@ignore'
   echo '* * * * * * * * * * * * * * * * *'
   echo "Copy of text from $f"
   echo '@end ignore'
   cat $f
done

` =]
[=

INVOKE  get-text tag = Future

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

=][=

DEFINE id-file =]
@ignore
Generated [= (tpl-file-line) =].
Extracted from [=srcfile=] line [=linenum=].
@end ignore[=

ENDDEF  id-file

# # # # # # # # # # # # # # # # # # # # =][=

DEFINE get-text     =][=

(set! text-tag
   (string-append "@ignore\n%s == "
      (string-upcase! (get "tag")) " == %s or the surrounding 'ignore's\n"
      "Extraction from autogen.texi\n"
      "@end ignore" ))

(extract texi-file-source text-tag) =][=

ENDDEF get-text

# # # # # # # # # # # # # # # # # # # # =][=

DEFINE set-func-name =][=

  (set! func-name (shell (sprintf "echo '%s' |
    sed -e 's/-p$/?/' -e 's/-x$/!/' -e 's/-to-/->/'"
    (string-tr! (get "name") "A-Z_^" "a-z--") )) )

  (set! func-str
      (if (exist? "string") (get "string") func-name)) =][=

ENDDEF

# # # # # # # # # # # # # # # # # # # # =][=

DEFINE emit-scm-func     =][=

  INVOKE  set-func-name  =]
@node SCM [= (. func-str)=]
@subsection [= (string-append "@file{" func-name "} - " (get "what")) =]
@findex [= (. func-name) =][=
% string "\n@findex %s"  =]
[=     id-file           =]
Usage:  ([= (. func-str) =][=

    FOR exparg           =] [=

      arg_optional  "[ " =][=arg_name=][= arg_list " ..." =][=
      arg_optional  " ]" =][=

    ENDFOR   exparg      =])
@*
[= string (string-append func-name ":  ") =][=
   (shell (string-append
          "(set -x;sed \"s/^\\`'//\" <<\\_EODoc_\n"
          (if (exist? "doc") (get "doc") "This function is not documented.")
          "\n_EODoc_\n)" ))=]
[=
    IF (exist? "exparg") =]
Arguments:[=
      FOR exparg         =]
@*
[=arg_name=] - [=

    arg_optional "Optional - " =][=
        IF (exist? "arg_desc") =][=arg_desc=][=
        ELSE             =]Undocumented[=
        ENDIF            =][=

      ENDFOR exparg      =][=

    ELSE
    =]
This Scheme function takes no arguments.[=
    ENDIF                =]
[=

ENDDEF

# # # # # # # # # # # # # # # # # # # # =][=

DEFINE emit-menu-entry   =][=

   INVOKE set-func-name  =][=
   (sprintf "\n* SCM %-20s @file{%s} - %s"  (string-append func-str "::")
            func-name (get "what"))
   =][=
ENDDEF  emit-menu-entry

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

=][=

(shell "rm -rf ${tempdir}")

;; Local Variables:
;; indent-tabs-mode: nil
;; mode: texinfo
;; End:

=]
