[= autogen template -*-texinfo-*-
#
#  Documentation template
#  $Id: auto_gen.tpl,v 1.9 1998/07/14 21:27:51 bkorb Exp $
#
texi=autogen.texi =]
\input texinfo
@c %**start of header
@setfilename autogen.info
@settitle Autogen
@setchapternewpage off
@c %**end of header

@ignore
[=_eval "" _DNE=]
@end ignore

@set EDITION [=version=]
@set VERSION [=version=]
@set UPDATED [=_EVAL 'date "+%B %Y"' _shell =]

@dircategory GNU programming tools
@direntry
* autogen: (autogen).         [=prog_title=]
@end direntry

@ifinfo
This file documents [=package=] Version @value{VERSION}

Autogen copyright @copyright{} [=copyright=] Bruce Korb

[=prog_name _cap "" _gpl=]

@ignore
Permission is granted to process this file through TeX and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph
@end ignore
@end ifinfo


@titlepage
@title [=prog_title=]
@subtitle For version @value{VERSION}, @value{UPDATED}
@author Bruce Korb

@page
@vskip 0pt plus 1filll
[=prog_name _cap copyright _get owner _get
       "#3$%s copyright %s %s" _printf=]
@sp 2
This is the first edition of the GNU Autogen documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=prog_name _cap "" _gpl=]
@end titlepage

@ifinfo
@node Top, Introduction, , (dir)
@top [=prog_title=]
@comment  node-name,  next,  previous,  up

This file documents the [=prog_title=] package for creating an arbitrary
text files containing repetitive text with varying substitutions.
This edition documents version @value{VERSION}, @value{UPDATED}.

@menu
* Introduction::         Autogen's purpose
* Generalities::         General ideas
* Example Usage::        A Simple Example
* Definitions File::     Macro Definitions File
* Template File::        Output Template
* Invocation::           Running the Program
* Installation::         What Gets Installed Where
* Autoopts::             Automated Option Processing
* Future::               Some ideas for the future.
* Concept Index::        General index
* Function Index::	 Function index
@end menu

@end ifinfo

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Introduction
@chapter Introduction
@cindex Introduction

Autogen is a tool for automatically generating arbitrary text
files that contain repetitive text with varying substitutions.
This is particularly useful if you have several types of repetitive
text that all need to be kept in sync with each other.

One application for this would be, for example, processing
program options.  Processing options requires a minimum of four
different constructs be kept in proper order in different places in
your program.  You need minimally:

@enumerate
@item
the flag character in the flag string
@item
code to process the flag when it is encountered
@item
a global variable (typically something like:  @samp{int sflg = 0;})
@item
and please do not forget a line in the usage text :-)
@end enumerate

@noindent
You will need more things besides this if you choose to implement
long option names, rc/ini file processing, and environment variables.

All of these things can be kept in sync mechanically,
with the proper templates and this program.
In fact, I have already done so and @code{autogen} already
uses the AutoOpt facility.

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Generalities
@chapter General ideas
@cindex m4

The goal is to try to simplify the process of maintaining
repetitive program text.  If there is a single block of text
that needs repetitive substitutions, @code{#define}
macros frequently fill the bill.  Sometimes, though, they are
not quite powerful enough and you will need to use something
like @code{m4}.  When appropriate, those approaches are much
simpler than autogen.

@cindex design goals
Autogen has been designed for
addressing the problem when there are the same or similar
substitutions required in multiple blocks of repeating text.
@code{#define}s do not always work because they are inflexible
and even sometimes obscure.  @code{m4} is often inadequate
because the entire substitution lists must be repeated in
the proper contexts throughout the base text (template).

Autogen addresses these problems by separating
the substitution text (macro definitions) from the template
text.  The template text makes references to the definitions
in order to compute text to insert into the template and
to compute which part of the template to process at each step.

@table @samp
@item template text
@cindex .tpl file
@cindex template file
This is the template used to create the output files.
It contains text that is copied to the output verbatim and
text between escape markers (denoted at the beginning of the file).
The text between markers is used to generate replacement text,
control the output or to control the processing of the template.

Conventionally, it uses the file name suffix @code{tpl}.

@item macro definitions
@cindex .def file
@cindex definition file
This file is named on the @code{autogen} invocation line.
It identifies the template that the definitions are to be used with
and it contains text macro and group macro definitions.

Conventionally, it uses the file name suffix @code{def}.
@end table

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Example Usage
@chapter A Simple Example
@cindex example, simple
@cindex simple example

Assume you have a list of names and you wish to keep them in an array
and you want an enumerated index for each entry and that it is too
much trouble to do by hand:

@noindent
You have list.h:

@example
#define IDX_ALPHA    0
#define IDX_BETA     1
#define IDX_OMEGA    2

extern const char* az_name_list[ 3 ];
#endif
@end example

@noindent
and you have list.c:

@example
#include "list.h"
const char* az_name_list[ 3 ] = @{
        "some alpha stuff",
        "more beta stuff",
        "dumb omega stuff" @};
@end example

To do this, you need a template or two that can be expanded
into the files you want.  In this program, we use a single
template that is capable of multiple output files.

Assuming we have chosen our markers to be '[#' and '#]' to
start and end our macros, we have a template something like this
(a full description of this is in the next chapter, "Complete Syntax"):

@example
[#autogen template h c #]
[#_IF _SFX h = #]
[#_FOR list#]
#define IDX_[#list_element _up#]  [#_eval _index#][# /list #]

extern const char* az_name_list[ [#_eval list _hival 1 + #] ];
#endif[#

_ELIF _SFX c = #]
#include "list.h"
const char* az_name_list[ [#_eval list _hival 1 + #] ] = @{[#
_FOR list ,#]
        "[#list_info#]"[#
/list #] @};[#

_ENDIF header/program #]
@end example

@noindent
and along with this must be included the macro definitions:

@example
autogen definitions list;
list = @{ list_element = alpha;
         list_info    = "some alpha stuff"; @};
list = @{ list_element = beta;
         list_info    = "more beta stuff"; @};
list = @{ list_element = omega;
         list_info    = "dumb omega stuff"; @};
@end example

@noindent
Furthermore, if we ever need a name/enumeration mapping again,
we can always write a new set of definitions for the old template.

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Definitions File
@chapter Macro Definitions File
@cindex definitions file
@cindex .def file

This file consists of an identity statement that identifies it as a
autogen file, followed by block and text macro definitions.
Intermingled may be C-style comments and C preprocessing directives.
All C preprocessing directives are identified, but many
@strong{especially @code{if}} are ignored.

@menu
* Identification::  The First Definition
* Definitions::     Text Macros and Block Macros
* Directives::      Controlling What Gets Processed
* Comments::        Commenting Your Definitions
* Example::         What it all looks like.
* Full Syntax::     YACC Language Grammar
@end menu

@node Identification
@section The First Definition
@cindex identification

The first definition in this file is used to identify it as a
autogen file.  It consists of the two keywords,
@samp{autogen} and @samp{definitions} followed by the default
template name and a terminating semi-colon @code{;}.

@cindex template, file name
@cindex .tpl, file name
@cindex tpl, file name

@noindent
The template file is determined from the template name in the following way:

@enumerate
@item
The template name is appended, if needed, with the string, ".tpl".
@item
@cindex Templ-Dirs
The file is looked for in the current directory.
@item
The file is looked for in the directories named
in the @code{Templ-Dirs} options on the command line,
in the order specified.
@item
If the file is not found in any of these locations,
a message is printed and @code{autogen} exits.
@end enumerate

For a @code{foobar} template, your identification definition will look
like this:

@example
Autogen Definitions foobar;
@end example

@noindent
Note that, other than the name @code{foobar}, the words @samp{autogen}
and @samp{definitions} are searched for without case sensitivity.
Most lookups in this program are case insensitive.

@node Definitions
@section Text Macros and Block Macros
@cindex macros
@cindex text macros
@cindex block macros

Any macro may appear multiple times in the definition file.
If there is more than one instance, the @strong{only} way
to expand all of the copies of it is by using the @code{_FOR}
text function on the macro, as described in the next chapter.

There are two kinds of macro definitions, @samp{text} and @samp{block}.
Macros are defined thus:

@example
block_name '=' '@{' definition-list '@}' ';'

text_name '=' string ';'

no_text_name ';'
@end example

@noindent
The macro names may be a simple name taking the next available index,
or may specify an index by name or number.  For example:

@example
mac_name
mac_name[2]
mac_name[ DEF_NAME ]
@end example
@noindent
@code{DEF_NAME} must be defined to have a numeric value.
If you do specify an index, you must take care not to cause conflicts.

@noindent
@code{No_text_name} is a text definition with a shorthand empty string
value.

@noindent
@code{definition-list} is a list of definitions that may or may not
contain nested block definitions.  Any such definitions may @strong{only}
be expanded within the a @code{FOR} block iterating over the
containing block macro.

@cindex text macros, format
The string values for the text macros may be specified in one of four
quoting rules:

@table @samp
@item with a double quote @code{"}
@cindex string, double quote
The string follows the
C-style escaping (@code{\}, @code{\n}, @code{\f}, @code{\v}, etc., plus
octal character numbers specified as @code{\ooo}.  The difference
from "C" is that the string may span multiple lines.

@item with a single quote "'"
@cindex string, single quote
This is similar to the shell single-quote string.  However, escapes
@code{\} are honored before another escape, single quotes @code{'}
and hash characters @code{#}.  This latter is done specifically
to disambiguate lines starting with a hash character inside
of a quoted string.  In other words,

@example
foo = '
#endif
';
@end example

could be misinterpreted by the definitions parser, whereas
this would not:

@example
foo = '
\#endif
';
@end example

@item with a back quote @code{`}
@cindex string, shell output
This is treated identically with the double quote, except that the
resulting string is written to a shell server process and the macro
takes on the value of the output string.

NB:  The text in interpreted by a server shell.  There may be
left over state from previous @code{`} processing and it may
leave state for subsequent processing.  However, a @code{cd}
to the original directory is always issued before the new
command is issued.

A definition utilizing a backquote may not be joined with any other text.

@item without surrounding quotes
The string must not contain any of the characters special to the
definition text.  E.g. @code{;}, @code{"}, @code{'}, @code{`}, @code{=},
@code{@{}, @code{@}}, @code{[}, @code{]}, @code{#} or any
white space character.  Basically, if the string looks like it is a
normal file name or variable name, and it is not one of two keywords
(@samp{autogen} or @samp{definitions}) then it is OK to not quote it.
@end table

If the single or double quote characters is used, then you
also have the option, a la ANSI-C syntax, of implicitly
concatenating a series of them together, with intervening
white space ignored.

NOTE: You @strong{cannot} use directives to alter the string
content.  That is,

@example
str = "foo"
#ifdef LATER
      "bar"
#endif
      ;
@end example

@noindent
will result in a syntax error.  However,

@example
str = "foo
#ifdef LATER
      bar
#endif\n";
@end example

@noindent
@strong{Will} work.  It will enclose the @samp{#ifdef LATER}
and @samp{#endif} in the string.  But it may also wreak
havoc with the definition processing directives.  The hash
characters in the first column should be disambiguated with
an escape @code{\} or join them with previous lines:
@code{"foo\n#ifdef LATER...}.

@node Directives
@section Controlling What Gets Processed
@cindex directives

Directives can @strong{only} be reliably processed if the '#' character
is the first character on a line.  Also, if you want a '#' as the first
character of a line in one of your string assignments, you should either
escape it by preceeding it with a backslash @samp{\}, or by embedding
it in the string as in @samp{"\n#"}.

All of the normal C preprocessing directives are recognized,
plus an additional @code{#shell} @code{#endshell} pair.
One minor difference though is that autogen
directives must have the hash character @code{#} in column 1.
Another difference is that several of them are ignored.  They are:
[=
_FOR agdirect_func =][=
  _IF dummy _exist
    =]@samp{#[=name _down=]}, [=
  _ENDIF =][=
/agdirect_func=] and @samp{#if}.
Note that when ignoring the @code{#if} directive, all intervening
text through its matching @code{#endif} is also ignored,
including the @code{#else} clause.

The autogen directives that affect the processing of
definitions are:

@table @code[=
_FOR agdirect_func "\n"=][=
  _IF text _exist
    =]
@item #[=name _down =][=
    _IF arg _exist =] [=arg=][=
    _ENDIF=]
@cindex #[=name _down=]
@cindex [=name _down=] directive
[=text=][=
  _ENDIF=][=
/agdirect_func=]
@end table

@node Comments
@section Commenting Your Definitions
@cindex comments

The definitions file may contain C-style comments.

@example
/*
 *  This is a comment.
 *  It continues for several lines and suppresses all processing
 *  until the closing characters '*' and '/' appear together.
#include is ignored.
 */
@end example

@node Example
@section What it all looks like.

@noindent
This is an extended example:

@example
autogen definitions @samp{template-name};
/*
 *  This is a comment that describes what these
 *  definitions are all about.
 */
globel = "value for a global text macro.";

/*
 *  Include a standard set of definitions
 */
#include standards.def

a_block = @{
    a_field;
    /*
     *  You might want to document sub-block macros, too
     */
    a_subblock = @{
        sub_name  = first;
        sub_field = "sub value.";
    @};

#ifdef FEATURE
    /*
     *  This definition is applicable only if FEATURE
     *  has been defined during the processing of the definitions.
     */
    a_subblock = @{
        sub_name  = second;
    @};
#endif

@};

a_block = @{
    a_field = here;
@};
@end example

@node    Full Syntax
@section YACC Language Grammar

Extracted fromt the agParse.y source file:

@example
[=_eval
  "sed -n -e'/^definitions/,$p' $top_srcdir/src/agParse.y |
  sed -e's/{/@{/g' -e's/}/@}/g' "
  _shell=]
@end example

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Template File
@chapter Output Template
@cindex template file
@cindex .tpl file

The template is really composed of two parts.
The first part consists of a pseudo macro invocation and commentary.
It is followed by the template proper.

@cindex pseudo macro
@cindex macro, pseudo
This pseudo macro is special.  It is used to identify the file as a
autogen template file, fixing the starting and ending marks for
the macro invocations in the rest of the file, and specifying the list
of suffixes to be generated by the template.

@menu
* template id::      Format of the Pseudo Macro
@end menu

Auto-genning a file consists of copying text from the template proper
to the output file until a start macro marker is found.  The text from
the start marker to the end marker constitutes the macro text.  If it
starts with a hash mark @code{#}, then the macro is a comment.  If it
starts with an underscore @code{_}, then it is an explicit function
invocation.  These functions are listed below.  If it starts with an
alphabetic character, then an implicit function will be invoked.  See
the discussion below.  If it begins with any other character (white
space being ignored), then it is in error and processing will stop.

The following macro functions are currently supported.  The @code{eval}
functionality is also used by many of the functions before examining
their argument lists.  The entries have been alphabatized:

@menu[=
_FOR agfunc_func =][=
  _IF unnamed _exist ! deprecated _exist ! & =]
* [=name #:: + "#%-16s" _printf=] [=name _up=] - [=what=][=
  _ENDIF =][=
/agfunc_func=]
@end menu

If the macro text begins with an alphabetic character, the
macro function invoked is selected by default.  The default
depends on the type of macro named by the first token in the
macro.  @xref{Definitions File} chapter, for detailed
information on how macro types are determined.

@table @samp
@item block macros
The @code{FOR} function is invoked over the text that starts
after the closing @code{end} mark and ends just before the
opening @code{mark} marker containing a slash @code{/} and
the block macro name.

@item text macros
The @code{EVAL} function is invoked, including all the
processing arguments.  If there are multiple copies of the
text macro, the one with the highest index is selected.  If
you wish to retrieve all the defined values for a text macro
named @code{txt}, you would need to code:

@example
[#_FOR txt#][#txt#][#/txt#]
@end example
@noindent
Inside the domain of the @code{_FOR} function, only the
@code{txt} value for the current index is visible.

@item undefined macros
It is treated as a comment; no function is invoked.
@end table

@noindent
A @code{template block} is template text that does not have any
incomplete @code{case}, @code{for} or @code{if} macro functions.

@node template id
@section Format of the Pseudo Macro
@cindex template id

The starting macro marker must be the first non-white space characters
encountered in the file.  The marker consists of all the contiguous
ASCII punctuation characters found there.  With optional intervening
white space, this marker must be immediately followed by the keywords,
"autogen" and "template".  Capitalization of these words is not
important.  This is followed by zero, one or more suffix specifications.

Suffix specifications consist of a sequence of POSIX compliant file name
characters and, optionally, an equal sign and a file name "printf"-style
formatting string.  Two string arguments are allowed for that string:
the base name of the definition file and the current suffix (that being
the text to the left of the equal sign).  (Note: "POSIX compliant file
name characters" consist of alphanumerics plus the period (@code{.}),
hyphen (@code{-}) and underscore (@code{_}) characters.)  If there are
no suffix specifications, then the generated file will be written to the
stdout file descriptor.

The pseudo macro ends with an end macro marker.  Like the starting macro
marker, it consists of a contiguous sequence of arbitrary punctuation
characters.  However, additionally, it may not begin with any of the
POSIX file name characters and it may not contain the start macro
marker.

This pseudo macro may appear on one or several lines of text.
Intermixed may be comment lines (completely blank or starting with the
hash character @code{#} in column 1), and file content markers (text
between @code{-*-} pairs on a single line).  This may be used to
establish editing "modes" for the file.  These are ignored by
autogen.

The template proper starts after the pseudo-macro.
The starting character is the first character that meets one
of the following conditions:

@enumerate
@item
It is the first character of a start-macro marker, or
@item
the first non-whitespace character, or
@item
the first character after a new-line.
@end enumerate

So, assuming we want to use @code{[#} and @code{#]} as the start and
end macro markers, and we wish to produce a @file{.c} and a @file{.h}
file, then the first macro invocation will look something like this:

@example
[#autogen template -*- Mode: emacs-mode-of-choice -*-

h=chk-%s.h

# it is important that the end macro mark appear after
# useful text.  Otherwise, the '#]' by itself would be
# seen as a comment and ignored.

c #]
@end example

Note:  It is generally a good idea to use some sort of opening
bracket in the starting macro and closing bracket in the ending
macro  (e.g. @code{@{}, @code{(}, @code{[}, or even @code{<}
in the starting macro).  It helps both visually and with editors
capable of finding a balancing parenthesis.

[=

#  FOR each defined function,
      this code will insert the extracted documentation =][=

_FOR agfunc_func =][=
  _IF unnamed _exist ! deprecated _exist ! & =]

@node [=name=]
@section [=name _up=] - [=what=]
@findex [=name _up=][=
    _FOR cindex =]
@cindex [=cindex=][=
    /cindex=]

[=desc=][=
    _IF table _exist =][=
      _CASE table _get =][=
      _ agexpr_func =]
@table @samp[=
         _FOR agexpr_func =]
@findex [=name _up=]
@item [=name _up=]
[=descrip=]
[=
         /agexpr_func
=]
@end table[=
      _ESAC =][=
    _ENDIF =][=

  _ENDIF "unnamed does not exist" =][=
/agfunc_func=]

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Invocation
@chapter Running the program
@cindex invocation

@code{Autogen} accepts the following options,
as shown in this AutoOpt generated usage text:

@example
[=_eval "#../src/autogen --help 2>&1 |
        sed -e 's/{/@{/' -e 's/}/@}/' -e 's/\t/        /g'"  _shell=]
@end example

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Installation
@chapter What Gets Installed Where
@cindex Installation

There are several files that get installed.  The number depend
whether or not both shared and archive libraries are to be
installed.  The following assumes that everything is installed
relative to @code{$prefix}.  You can, of course, use
@code{configure} to place these files where you wish.

@strong{NB}:  Autogen does not contain any compiled-in path names.
All support directories are located via option processing
and the environment variable @code{HOME}.

The installed files are:

@enumerate
@item
The executable in @code{bin/autogen}.

@item
The link library(ies) in @code{lib/libopts.*}.

@item
An include file in @code{include/options.h}, needed for
Automated Option Processing (see next chapter).

@item
Three template files in @code{share/autogen/opt*.tpl}, needed for
Automated Option Processing (see next chapter).

@item
Info-style help files in @code{info/autogen.info*}.
These files document both autogen and the option processing
library autoopts.
@end enumerate

This program, library and supporting files can be installed
with two commands:

@itemize @bullet
@item
<src-dir>/configure [ <configure-options> ]
@item
make install
@end itemize

However, you may wish to insert @code{make}
and @code{make check} before the second command.

If you do do a @code{make check} and there are any failures,
you will find the results in @code{tests/FAILURES}.  Needless to say,
I would be interested in seeing the contents of those files and
any associated messages.  If you choose to go on and analyze
one of these failures, you will have to invoke the test script
by hand.  Automake does not provide an easy way to do this
and there are some things you have to do to make it work.

Here is the code from the make file that runs each test:
@example
TESTS_ENVIRONMENT = testsubdir=$(testsubdir) \
        top_srcdir=$(top_srcdir) CC=$(CC)

srcdir=$(srcdir); export srcdir; \
for tst in $(TESTS); do \
  if test -f $$tst; then dir=.; \
  else dir="$(srcdir)"; fi; \
  if $(TESTS_ENVIRONMENT) $$dir/$$tst; then \
    all=`expr $$all + 1`; \
    echo "PASS: $$tst"; \
  elif test $$? -ne 77; then \
    all=`expr $$all + 1`; \
    failed=`expr $$failed + 1`; \
    echo "FAIL: $$tst"; \
  fi; \
done
@end example

Notes:
@itemize @bullet
@item
@samp{testsubdir} is a configured value that defaults to @samp{testdir}.
@item
@samp{srcdir} is a configured value that defaults to @samp{.}.
@item
@samp{top_srcdir} needs to be set correctly.
@item
@samp{CC} needs to refer to an ANSI-compliant compiler.
@end itemize

So, you must invoke the @samp{test-name.test} file thus,
replacing @code{.} and @code{..} as needed:

@example
testsubdir=testdir top_srcdir=.. srcdir=. ./test-name.test
@end example

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Autoopts
@chapter Automated Option Processing
@cindex autoopts

Included with autogen is a tool that virtually eliminates the hassle
of processing options, keeping usage text up to date and so on.
This package allows you to specify several program attributes,
innumerable options and option attributes, then it produces
all the code necessary to parse and handle the command line
and initialization file options.  See @samp{features}
for a more complete description.

@menu
* Features::       Autoopts Features
* opts.def::       Option Definitions
* opts.h::         User Interface
* Macro Usage::    User Interface Usage
* opts.c::         Data Structure and Callout Procs
* dependencies::   Autogen Inter-dependency Graph
@end menu

@node Features
@section Autoopts Features
@cindex features

Autoopts supports option processing and option state saving
with the following features:

@enumerate
@item
Process initialization/rc files
(if specified in the definition and the file(s) exist).
@item
Process environment variable initializations
(if specified in the definition and the variable(s) exist).
@item
Processes the command line options.
@item
Store the option state into an initialization/rc file
(if init files are specified and the save options option
is specified on the command line).
@item
Verify that required options are present the
required minimum number of times on the command line.
@item
Verify that options do not appear more than the maximum number
of times.
@item
Verify that conflicting options do not appear together.
@item
Verify that options that require the presence of other options
are, in fact, used in the presence of other options.
@item
Provides a callable routine to parse a line of text as if
it were a line from one of the RC/INI files.  (Consequently,
you may stash options in a text file you process and pass
the relevent text for option processing.)
@item
Produce usage information for the library routine "optionUsage()".
Three levels of usage are supported:
@table @samp
@item Brief
as when an option error is encountered.
@item Normal
as when "--help" is specified on the command line.
@item Extended
when "--more-help" is specified on the command line.
This output is passed through a pager (the @code{more} program,
if the PAGER environment variable is not set).
This output differs from normal help only if a usage
detail is specified.
@end table
@item
Produce a #define for the program version that can be
used by the library routine "doVersion()".
@end enumerate

Furthermore, there are provisions for enablement and disablement
prefixes to the "long-name" versions of the options.  The option
processing supports POSIX compliant options.  With the "long_opts"
attribute set, it also supports GNU standard long options.
There is automatic support for "help", "more-help", a GNU-compliant
"version" and "save-opts" options.

@node opts.def
@section Option Definitions
@cindex opts.def

This file contains the autogen definitions that descrbes the
options for a particular program.  There are two groups of
definitions.  The first describes the program and option
processing attributes, the second group describes the name,
usage and other attributes related to each option.

@menu
* program attributes::    Program Description and Attributes
* option attributes::     Option Attributes
* standard options::      Automatically Supported Options
* example options::       Autogen's option definitions
@end menu

[=_EVAL "sed -n '/^@node program attributes/,/^@ignore/p' \
        $top_srcdir/autoopts/options.tpl |
sed -e's/`/@code{/g' \
    -es/\\\'/}/g     \
    -e's/^\\([a-zA-Z]\\)/@item \\1/'  \
    -e 's/^[ \t]*//' " _shell =]
@end ignore

@node standard options
@subsection Automatically Supported Options
@cindex standard options

Autoopts provides automated support for four options.
@code{help} and @code{more-help} are always provided.
@code{version} is provided if @code{version} is defined
in the option definitions.  @code{save-opts} is provided
if @code{homerc} is defined.

Below are the option names and flag values.
The flag characters are activated iff at least one user-defined
option uses a flag value.

@table @samp
@item help -?
This option will immediately invoke the @code{USAGE()} procedure
and display the usage line, a description of each option with
its description and option usage information.  This is followed
by the contents of the definition of the @code{detail} text macro.

@item more-help -!
This option is identical to the @code{help} option, except that
it also includes the contents of the @code{detail-file} file
(if provided and found), plus the output is passed through
a pager program.  (@code{more} by default, or the program identified
by the @code{PAGER} environment variable.)

@item version -v
This will print the program name, title and version.
If it is followed by the letter @code{c} and
a value for @code{copyright} and @code{owner} have been provided,
then the copyright will be printed, too.
If it is followed by the letter @code{n}, then the full
copyright notice (if available) will be printed.

@item save-opts ->
This option will cause the option state to be printed in
RC/INI file format when option processing is done but not
yet verified for consistency.  The program will terminate
successfully without running when this has completed.

The output file will be the RC/INI file name (default or provided
by @code{rcfile}) in the last directory named in a @code{homerc}
definition
@end table

@node example options
@subsection Autogen's option definitions
@cindex example options

Below is the option definition file used by autogen.
It will cause to be generated the interface and code files described
in the next two sections and the usage information
sampled above in the chapter on "Running the program".

@example
[=_EVAL "sed -n '/^copyright =/,$p' $top_srcdir/src/opts.def |
         sed '-es/@/@@/g' '-es/{/@{/g' '-es/}/@}/g'"
        _shell=]
@end example

@node opts.h
@section User Interface
@cindex opts.h
@cindex interface file

This file, along with the public #define-d values in <options.h>,
provide the user accessible interface to the option information.
If @code{prefix} has been defined for this program,
then the defined prefix will be inserted into all of the
generated macros and external names.  Below is an example from
@code{autogen} itself:

@example
[=_EVAL "echo '#ifndef AUTOGEN_OPTS_H' ; echo '#ifndef AUTOGEN_OPTS_H' ; echo
         sed -n '/^#include/,$p' $top_builddir/src/opts.h |
         sed -e's/{/@{/g' -e's/}/@}/g'" _shell=]
@end example

@node Macro Usage
@section User Interface Usage
@cindex Macro Usage

The macros and enumerations defined in the options
header (interface) file are used as follows:

@menu
[=_FOR component=]
* [=comp_name #:: + #%-28s _printf=] [=comp_name=] - [=title=][=
/component=]
@end menu
[=
_FOR component=]
@node [=comp_name=]
@subsection [=comp_name=] - [=title=]
@cindex [=comp_name=]

[=description=]
[=/component=]

@node opts.c
@section Data Structure and Callout Procs
@cindex opts.c

This contains internal stuff subject to change.  Basically, it contains
a single global data structure containing all the information provided
in the option definitions, plus a number of static strings and any
callout procedures that are specified or required.  You should never
have need for looking at this, execpt to examine the code generated
for implementing the @code{flag_code} construct.

@node dependencies
@section Autogen Inter-dependency Graph
@cindex autoopts dependencies

Since autoopts requires autogen and autogen uses autoopts,
the autogenned components of autogen were bootstrapped by
the author and provided in the release.

@ifinfo
@code{Autogen} @strong{requires} a @sc{POSIX} regular expression
library, such as @sc{GNU} regex.  Version 5 will probably require the
@sc{GNU} guile library too.  Other @code{autogen} based packages are
free to assume the presence of all of these; the interdependencies of
these packages will thus be:

@ignore
TODO: Figure out how to include an eps diagram in the printed manual.

@end ignore
@example
                                  .--------(autogetopts)
                                  V              |
                autoopts<---->autogen--------.   |
                   |              |          |   |
                   |              V          |   |
                   |           libguile--.   |   |
                   |                     V   V   V
                   `------------------->POSIX regex
@end example

@code{autogetopts} is an as-yet-to-be-implemented package
that will generate the option processing loop, but call
@code{getopts(3)} to parse the options.  It will @strong{only}
support option processing and the brief form of usage text.
@end ifinfo

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore

@node Future
@chapter Some ideas for the future.
@cindex futures

Here are some things that might happen in the distant future.

@itemize @bullet

@item
Rewrite in Guile.

@item
Write code for "autogetopt" (GNU getopt), or
possibly the new glibc argp parser.

@item
Fixup current tools that contain
miserably complex perl, shell, sed, awk and m4 scripts
to instead use this tool.
@end itemize

@node Concept Index
@unnumbered Concept Index

@printindex cp

@node Function Index
@unnumbered Function Index

@printindex fn

@contents
@bye
