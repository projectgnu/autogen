[= autogen template -*-texinfo-*-

##  Documentation template
## 
##  AutoGen Copyright (C) 1992-1998 Bruce Korb
## 
## Author:            Bruce Korb <korbb@datadesign.com>
## Maintainer:        Bruce Korb <korbb@datadesign.com>
## Created:           Tue Sep 15 13:13:48 1998
## Last Modified:     Fri Feb 26 10:17:15 1999
##            by:     Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: auto_gen.tpl,v 2.11 1999/02/26 18:26:56 bkorb Exp $
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
This is the second edition of the GNU AutoGen documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=_eval AutoGen "" _gpl=]
@end titlepage

@ifinfo
@node Top, Introduction, , (dir)
@top [=prog_title=]
@comment  node-name,  next,  previous,  up

This file documents AutoGen, a tool designed for generating program
files that contain repetitive text with varied substitutions.

This edition documents version @value{VERSION}, @value{UPDATED}.

@menu
* Introduction::         AutoGen's Purpose
* Definitions File::     AutoGen Definitions File
* Template File::        AutoGen Template
* Invocation::           Running AutoGen
* Installation::         What Gets Installed Where
* AutoOpts::             Automated Option Processing
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

AutoGen is a tool designed for generating program files that contain
repetitive text with varied substitutions.  Its goal is to simplify the
maintenance of programs that contain large amounts of repetitious text.
This is especially valuable if there are several blocks of such text
that must be kept synchronized.

One common example is the problem of maintaining the code required for
processing program options.  Processing options requires a minimum of
four different constructs be kept in proper order in different places in
your program.  You need at least:

@enumerate
@item
The flag character in the flag string,
@item
code to process the flag when it is encountered,
@item
a global state variable or two, and
@item
a line in the usage text.
@end enumerate

@noindent
You will need more things besides this if you choose to implement
long option names, rc/ini file processing, environment variables
and so on.

All of this can be done mechanically; with the proper templates and this
program.  In fact, it has already been done and AutoGen itself uses it:
@xref{AutoOpts}.

@menu
* Generalities::         General Ideas
* Example Usage::        A Simple Example
* Testimonial::          A User's Perspective
@end menu

@node Generalities
@section General Ideas
@cindex m4

The initial idea of this program was to have a text file, a template if
you will, that contained the text of the desired output file, with a few
additional features.
@cindex design goals

@enumerate
@item
The text would need special markers to indicate where substitutions were
required.  (A la the @code{$@{VAR@}} construct in a shell @code{here} doc.
Without this, there is no point.)

@item
It would also need a way to indicate sections of text that were to be
skipped and for sections of text that were to be repeated.  This is a
major improvement over using C preprocessing macros.  With the C
preprocessor, you have no way of selecting output text.  It is an
@i{un}varying, mechanical substitution process.

@item
There also needed to be methods for carefully controlling the output.
Sometimes, it is just simply easier and clearer to compute some text or
a value in one context when its application needs to be later.  So,
methods were devised for saving text or values for later use.
@end enumerate

This template needs to be driven by a separate file of named values; a
definitions file.  My goal here is to require a template user to specify
@strong{only} those data that are necessary to describe his application of
the template.  By completely isolating the definitions from the template
it becomes possible to greatly increase the flexibility of the
implementation (template).  So, the important attributes of the
definitions are:

@enumerate
@item
The definitions should be completely separate from the template.

@item
Each datum should be named.  That way, they can be rearranged,
augmented and become obsolete without it being necessary to go
back and clean up older definition files.  Reduce incompatibilities!

@item
There should be named collections, a hierarchy, of definitions.
Associated values need to be collected and associated with a group name.
That way, the associated data can be used collectively for sets of
substitutions.
@end enumerate

@node Example Usage
@section A Simple Example
@cindex example, simple
@cindex simple example

Assume you have an enumeration of names and you wish to associate some
string with each name.  Assume also, for the sake of this example,
that it is either too complex or too large to maintain easily by hand.

@noindent
In a header file, @file{list.h}, you define the enumeration
and the global array containing the associated strings:

@example
typedef enum @{
        IDX_ALPHA,
        IDX_BETA,
        IDX_OMEGA @}  list_enum;

extern const char* az_name_list[ 3 ];
@end example

@noindent
Then you also have @file{list.c} that defines the actual
strings:

@example
#include "list.h"
const char* az_name_list[] = @{
        "some alpha stuff",
        "more beta stuff",
        "dumb omega stuff" @};
@end example

@noindent
First, we will define the information that is unique for
each enumeration name/string pair.

@example
autogen definitions list;
list = @{ list_element = alpha;
         list_info    = "some alpha stuff"; @};
list = @{ list_info    = "more beta stuff";
         list_element = beta; @};
list = @{ list_element = omega;
         list_info    = "dumb omega stuff"; @};
@end example

The @code{autogen definitions list;} entry defines the file as an
AutoGen definition file.  That is followed by three @code{list}
entries that define the associations between the enumeration
names and the strings.  The order of the differently named
elements inside of list is unimportant.  They are reversed inside
of the @code{beta} entry and that will not affect the output.

Now, to actually create the output, we need a template or two that can
be expanded into the files you want.  In this program, we use a single
template that is capable of multiple output files.

It looks something like this.
(For a full description, @xref{Template File}.)

@example
[#autogen template h c #]
[#_IF _SFX h = #]
typedef enum @{[#
   _FOR list , #]
        IDX_[#list_element _up#][#
   /list #] @}  list_enum;

extern const char* az_name_list[ [#_eval list _count #] ];
[#

_ELIF _SFX c = #]
#include "list.h"
const char* az_name_list[] = @{[#
_FOR list ,#]
        "[#list_info#]"[#
/list #] @};[#

_ENDIF header/program #]
@end example

The @code{[#autogen template h c #]} text tells AutoGen that this is a
template file; that it is to be processed twice; that the start macro
marker is @code{[#}; and the end marker is @code{#]}.  The template will
be processed first with a suffix value of @code{h} and then with
@code{c}.

The @code{[#_IF _SFX h = #]} and @code{[#_ELIF _SFX c = #]} clauses
select different text for the two different passes.  In this example,
the output is nearly disjoint and could have been put in two separate
templates.  However, sometimes there are common sections and this is
just an example.

The @code{[#_FOR list ,#]} and @code{[# /list #]} clauses delimit
blocks of text that will be repeated for every definition of @code{list}.
Inside of that block, the definition name-value pairs that
are members of each @code{list} are available for substitutions.

The remainder of the macros are expressions.  Some of these contain
special expression functions to obtain certain values or modify the
values before they are inserted into the output.  For example,
@code{_count} yields the number of definitions of a particular name,
and @code{_up} will change a string to all upper case.

@node Testimonial
@section A User's Perspective

@format
        Subject: Re: Sysadmin Package
           Date: Thu, 24 Sep 1998
           From: "Gary V. Vaughan"
             To: Alexandre
             CC: autoconf <autoconf@@gnu.org>

> Bruce Korb <korb@@datadesign.com> writes:
> 
> > I would like to recommend my tool.  It exactly and precisely
> > addresses these very problems in a manner far simpler than M4.

Alexandre Oliva wrote:
> 
> I'd appreciate opinions from others about advantages/disadvantages of
> each of these macro packages.
@end format

I am using AutoGen in my pet project, and find one of its best points to
be that it separates the operational data from the implementation.

Indulge me for a few paragraphs, and all will be revealed:
In the manual, Bruce cites the example of maintaining command line flags
inside the source code; traditionally spreading usage information, flag
names, letters and processing across several functions (if not files). 
Investing the time in writing a sort of boiler plate (a template in
AutoGen terminology) pays by moving all of the option details (usage,
flags names etc.) into a well structured table (a definition file if you
will),  so that adding a new command line option becomes a simple matter
of adding a set of details to the table.

So far so good!  Of course, now that there is a template, writing all of
that tedious optargs processing and usage functions is no longer an
issue.  Creating a table of the options needed for the new project and
running AutoGen generates all of the option processing code in C
automatically from just the tabular data.  AutoGen in fact already ships
with such a template... AutoOpts.

One final consequence of the good separation in the design of AutoGen is
that it is retargettable to a greater extent.  The
egcs/gcc/fixinc/inclhack.def can equally be used (with different
templates) to create a shellscript (inclhack.sh) or a c program
(inclhack.c).

This is just the tip of the iceberg.  AutoGen is far more powerful than
these examples might indicate, and has many other varied uses.  I am
certain Bruce or I could supply you with many and varied examples, and I
would heartily recommend that you try it for your project and see for
yourself how it compares to m4.

As an aside, I would be interested to see whether someone might be
persuaded to rationalise autoconf with AutoGen in place of m4...  Ben,
are you listening?  autoconf-3.0! `kay?  =)O|

@format
Sincerely,
        Gary V. Vaughan
@end format

@ignore
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
@end ignore
@page
@node Definitions File
@chapter AutoGen Definitions File
@cindex definitions file
@cindex .def file

This chapter describes the syntax and semantics of the AutoGen
definition file.  Knowledge of this syntax is required of anyone that
uses AutoGen to instantiate a template.  Consequently, we keep it
simple.

The definitions file is used to associate values with names.  When
multiple values are associated with a single name, an implicit array of
values is formed.  Values may be either simple strings or compound
collections of value-name pairs.  An array may not contain both simple
and compound members.  For a simple example, @xref{Example Usage} or
@xref{Example}.

For purposes of commenting and controlling the processing of the
definitions, C-style comments and most C preprocessing directives are
honored.  The major exception is that the @code{#if} directive is
ignored, along with all following text through the matching
@code{#endif} directive.  The C preprocessor is not actually invoked, so
C macro substitution is @strong{not} performed.

@menu
* Identification::  The First Definition
* Definitions::     Simple and Compound Definitions
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
AutoGen Definitions foobar;
@end example

@noindent
Note that, other than the name @code{foobar}, the words @samp{AutoGen}
and @samp{definitions} are searched for without case sensitivity.
Most lookups in this program are case insensitive.

@node Definitions
@section Simple and Compound Definitions
@cindex definitions
@cindex simple definitions
@cindex compound definitions

Any name may have multiple values associated with it in the definition
file.  If there is more than one instance, the @strong{only} way to
expand all of the copies of it is by using the _FOR (@xref{FOR}) text
function on it, as described in the next chapter.

There are two kinds of definitions, @samp{simple} and @samp{compound}.
They are defined thus:

@example
compound_name '=' '@{' definition-list '@}' ';'

text_name '=' string ';'

no_text_name ';'
@end example

@noindent
The names may be a simple name taking the next available index,
or may specify an index by name or number.  For example:

@example
txt_name
txt_name[2]
txt_name[ DEF_NAME ]
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
containing compound definition.  @xref{FOR}.

@cindex simple definitions, format
The string values for definitions may be specified in one of four
formation rules:

@table @samp
@item with a double quote @code{"}
@cindex string, double quote
The string follows the C-style escaping (@code{\}, @code{\n}, @code{\f},
@code{\v}, etc.), plus octal character numbers specified as @code{\ooo}.
The difference from "C" is that the string may span multiple lines.

@item with a single quote @code{'}
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

could be misinterpreted by the definitions scanner, whereas
this would not:

@example
foo = '
\#endif
';
@end example

@item with a back quote @code{`}
@cindex string, shell output
This is treated identically with the double quote, except that the
resulting string is written to a shell server process and the definition
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
global = "value for a global text definition.";

/*
 *  Include a standard set of definitions
 */
#include standards.def

a_block = @{
    a_field;
    /*
     *  You might want to document sub-block definitions, too
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
@chapter AutoGen Template
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
@node AutoOpts
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
