[= autogen template -*-texinfo-*-

##  Documentation template
## 
##  AutoGen Copyright (C) 1992-1998 Bruce Korb
## 
## Author:            Bruce Korb <korbb@datadesign.com>
## Maintainer:        Bruce Korb <korbb@datadesign.com>
## Created:           Tue Sep 15 13:13:48 1998
## Last Modified:     Fri Jan 29 07:12:38 1999
##            by:     Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: auto_gen.tpl,v 2.7 1999/02/24 18:53:27 bkorb Exp $
## ---------------------------------------------------------------------
##
texi=autogen.texi =]
\input texinfo
@c %**start of header
@setfilename autogen.info
@settitle AutoGen
@setchapternewpage off
@c %**end of header

@ignore
[=_eval "" _DNE=]

Plus bits and pieces gathered from all over the source/build
directories:
[=_FOR infile=]
	[=infile=][=
/infile =]
[=
_EVAL '
  echo "	${top_srcdir}/autoopts/autoopts.texi"
  for f in ${top_builddir}/*/*.menu
  do echo "	$f"
     echo "	`echo $f|sed \'s/\.menu$/\.texi/\'`"
  done' _shell =]

@end ignore

@set EDITION [=_EVAL "echo ${AG_REVISION}" _shell=]
@set VERSION [=_EVAL "echo ${AG_REVISION}" _shell=]
@set UPDATED [=_EVAL 'date "+%B %Y"' _shell =]

@dircategory GNU programming tools
@direntry
* AutoGen: (autogen).         [=prog_title=]
@end direntry

@ifinfo
This file documents [=package=] Version @value{VERSION}

AutoGen copyright @copyright{} [=copyright=] Bruce Korb

[=_eval AutoGen "" _gpl=]

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
[=_eval AutoGen copyright _get owner _get
       "#3$%s copyright %s %s" _printf=]
@sp 2
This is the first edition of the GNU AutoGen documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=_eval AutoGen "" _gpl=]
@end titlepage

@ifinfo
@node Top, Introduction, , (dir)
@top [=prog_title=]
@comment  node-name,  next,  previous,  up

This file documents the [=prog_title=] package for creating an arbitrary
text files containing repetitive text with varying substitutions.

This edition documents version @value{VERSION}, @value{UPDATED}.

@menu
* Introduction::         AutoGen's Purpose
* Generalities::         General Ideas
* Example Usage::        A Simple Example
* Definitions File::     Macro Definitions File
* Template File::        Output Template
* Invocation::           Running AutoGen
* Installation::         What Gets Installed Where
* Autoopts::             Automated Option Processing
* Add-Ons::              Add-on packages for AutoGen
* Future::               Some ideas for the future.
* Concept Index::        General index
* Function Index::	 Function index
@end menu

@end ifinfo

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Introduction
@chapter Introduction
@cindex Introduction

AutoGen is a tool for automatically generating arbitrary text
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
In fact, I have already done so and AutoGen already
uses the AutoOpt facility.

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Generalities
@chapter General Ideas
@cindex m4

The goal is to try to simplify the process of maintaining repetitive
program text.  If there is a single block of text that needs repetitive
substitutions, @code{#define} macros frequently fill the bill.  They do
not always work because they are inflexible and even sometimes obscure.
In many of these cases, @code{M4} will work quite well.  When
appropriate, those approaches are much simpler than AutoGen.  Even then
@code{M4} is often inadequate because the entire substitution lists must
be repeated in the proper contexts throughout the base text (template).

@cindex design goals
AutoGen addresses these problems by being extremely flexible in the
kinds of substitutions that can be performed and by separating the
substitution text (macro definitions) from the template text.  The
template text makes references to the definitions in order to compute
text to insert into the template and to compute which part of the
template to process at each step.

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
@page
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
#ifndef HEADER
#define HEADER

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

Assuming we have chosen our markers to be '[#' and '#]' to start
and end our macros, we have a template something like this.
(For a full description, @xref{Template File}.)

@example
[#autogen template h c #]
[#_IF _SFX h = #]
[#_FOR list#]
#define IDX_[#list_element _up#]  [#_eval _index#][# /list #]

extern const char* az_name_list[ [#_eval list _count #] ];
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
@page
@node Definitions File
@chapter Macro Definitions File
@cindex definitions file
@cindex .def file

This file consists of an identity statement that identifies it as a
AutoGen file, followed by block and text macro definitions.
Intermingled may be C-style comments and C preprocessing directives.
All C preprocessing directives are identified, but many
(notably @code{if}) are ignored.

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
AutoGen file.  It consists of the two keywords,
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
a message is printed and AutoGen exits.
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
@item with a double quote @code{(")}
@cindex string, double quote
The string follows the
C-style escaping (@code{\}, @code{\n}, @code{\f}, @code{\v}, etc., plus
octal character numbers specified as @code{\ooo}.  The difference
from "C" is that the string may span multiple lines.

@item with a single quote @code{(')}
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

@item with a back quote @code{(`)}
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
str = '"foo\n"
#ifdef LATER
"     bar\n"
#endif
';
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

Definition processing directives can @strong{only} be reliably processed
if the '#' character is the first character on a line.  Also, if you
want a '#' as the first character of a line in one of your string
assignments, you should either escape it by preceeding it with a
backslash @samp{\}, or by embedding it in the string as in @code{"\n#"}.

All of the normal C preprocessing directives are recognized,
plus an additional @code{#shell} @code{#endshell} pair.
One minor difference though is that AutoGen
directives must have the hash character @code{#} in column 1.
Another difference is that several of them are ignored.  They are:
[=
_FOR directive =][=
  _IF dummy _exist
    =]@samp{#[=name _down=]}, [=
  _ENDIF =][=
/directive=] and @samp{#if}.
Note that when ignoring the @code{#if} directive, all intervening
text through its matching @code{#endif} is also ignored,
including the @code{#else} clause.

The AutoGen directives that affect the processing of
definitions are:

@table @code[=
_FOR directive "\n"=][=
  _IF text _exist
    =]
@item #[=name _down =][=
    _IF arg _exist =] [=arg=][=
    _ENDIF=]
@cindex #[=name _down=]
@cindex [=name _down=] directive
[=text=][=
  _ENDIF=][=
/directive=]
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

The processing directives and comments are not
part of the grammar.  They are handled by the scanner/lexer.
The following was extracted directly from the agParse.y source file:

@example
[=_eval
  "sed -n -e'/^definitions/,$p' $top_srcdir/src/agParse.y |
  sed -e's/{/@{/g' -e's/}/@}/g' "
  _shell=]
@end example

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
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
AutoGen template file, fixing the starting and ending marks for
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
_FOR macfunc =][=
  _IF desc _exist =]
* [=name #:: + "#%-16s" _printf=] [=name _up=] - [=what=][=
  _ENDIF =][=
/macfunc=]
@end menu

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
AutoGen.

The template proper starts after the pseudo-macro.  The starting
character is either the first non-whitespace character or the first
character after the new-line that follows the end macro marker.

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

_FOR macfunc =][=
  _IF desc _exist =]

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
         _FOR evalexpr =]
@findex [=name _up=]
@item [=name _up=]
[=descrip=]
[=
         /evalexpr
=]
@end table[=
      _ESAC =][=
    _ENDIF =][=

  _ENDIF "unnamed does not exist" =][=
/macfunc=]

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Invocation
@chapter Running AutoGen
@cindex invocation

[=_EVAL 'cat ${top_builddir}/src/autogen.texi' _shell
=]@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Installation
@chapter What Gets Installed Where
@cindex Installation

There are several files that get installed.  The number depend
whether or not both shared and archive libraries are to be
installed.  The following assumes that everything is installed
relative to @code{$prefix}.  You can, of course, use
@code{configure} to place these files where you wish.

@strong{NB}:  AutoGen does not contain any compiled-in path names.
All support directories are located via option processing
and the environment variable @code{HOME}.

The installed files are:

@enumerate
@item
The executables in @code{bin/*} (autogen, getdefs and columns).

@item
The link library(ies) in @code{lib/libopts.*}.

@item
An include file in @code{include/options.h}, needed for
Automated Option Processing (see next chapter).

@item
Four template files in @code{share/autogen/*.tpl}, needed for
Automated Option Processing (see next chapter).

@item
Info-style help files in @code{info/autogen.info*}.
These files document both AutoGen and the option processing
library AutoOpts.
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

If you do perform a @code{make check} and there are any failures, you
will find the results in @code{tests/FAILURES}.  Needless to say, I
would be interested in seeing the contents of those files and any
associated messages.  If you choose to go on and analyze one of these
failures, you will need to invoke the test scripts individually.  You
may do so by specifying the test (or list of test) in the TESTS make
variable, thus:

@example
make TESTS=test-name.test check
@end example

All of the AutoGen tests are written to honor the contents of the
@t{VERBOSE} environment variable.  Normally, any commentary generated
during a test run is discarded unless the @t{VERBOSE} environment
variable is set.  So, to see what is happening during the test, you
might invoke the following with @i{bash} or @i{ksh}:

@example
VERBOSE=1 make TESTS="for.test forcomma.test" check
@end example

Or equivalently with @i{csh}:

@example
env VERBOSE=1 make TESTS="for.test forcomma.test" check
@end example

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Autoopts
@chapter Automated Option Processing
@cindex autoopts
[=_INCLUDE autoopts/autoopts.texi=]
@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Add-Ons
@chapter Add-on packages for AutoGen

This chapter includes several programs that either work closely
with AutoGen (extracting definitions or providing special formatting
functions), or else it is @code{mkmerge}.  I want to promote the
latter as an alternative to the builddir/srcdir schitzophrenia.
I hate it. :(

AutoOpts ought to appear in this list also, but since it is
the primary reason why many people would even look into AutoGen
at all, I decided to leave it in the list of chapters.

@menu
[=_EVAL 'for f in ${top_builddir}/*/*.menu
do cat $f ; done' _shell =]
@end menu

[=_EVAL 'for f in ${top_builddir}/*/*.menu
do echo '@page'
   cat `echo $f|sed \'s/\.menu$/\.texi/\'`
done' _shell =]

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
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
@page
@node Concept Index
@unnumbered Concept Index

@printindex cp
@page
@node Function Index
@unnumbered Function Index

@printindex fn
@page
@contents
@bye
