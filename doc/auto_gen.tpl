[= AutoGen5 template

##  Documentation template
##
##  AutoGen Copyright (C) 1992-2001 Bruce Korb
##
## Author:            Bruce Korb <bkorb@gnu.org>
## Maintainer:        Bruce Korb <bkorb@gnu.org>
## Created:           Tue Sep 15 13:13:48 1998
## Last Modified:     Mar 4, 2001
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: auto_gen.tpl,v 3.15 2003/02/05 02:40:26 bkorb Exp $
## ---------------------------------------------------------------------

texi=autogen.texi

(setenv "SHELL" "/bin/sh")

(define texi-file-source (shell "
    if [ -f autogen.texi ]
    then
      mv -f autogen.texi autogen.texi.ori
      echo autogen.texi.ori
    elif [ -f ${top_srcdir}/doc/autogen.texi ]
    then
      echo autogen.texi
    else
      echo \"Cannot locate original autogen.texi file\" >&2
      kill -TERM ${AG_pid}
      exit 1
    fi" ))

=]
\input texinfo
@ignore
\internalpagesizes{46\baselineskip}{6in}{-.25in}{-.25in}{\bindingoffset}{36pt}%
@end ignore
@c %**start of header
@setfilename [=(base-name)=].info
@settitle AutoGen
@setchapternewpage off
@c %**end of header

@ignore
[=(set-writable) (dne "")=]

Plus bits and pieces gathered from all over the source/build
directories:
[= ` for f in ${DOC_DEPENDS} ; do echo "    $f" ; done ` =]

@end ignore
[=
(define e-addr "bkorb@gnu.org")

(shell (string-append "cat <<_EOF_
@set EDITION   ${AG_REVISION}
@set VERSION   ${AG_REVISION}
@set UPDATED   `date '+%B %Y'`
@set COPYRIGHT "    (get "copyright.date")
"\n@set TITLE     " (get "prog_title")
"\n@set PACKAGE   " (get "package")
"\n_EOF_" ))

=]

@dircategory GNU programming tools
@direntry
* AutoGen: (autogen).         @value{TITLE}
@end direntry

@ifinfo
This file documents @value{PACKAGE} Version @value{VERSION}

AutoGen copyright @copyright{} @value{COPYRIGHT} Bruce Korb
AutoOpts copyright @copyright{} @value{COPYRIGHT} Bruce Korb
snprintfv copyright @copyright{} 1999-2000 Gary V. Vaughan

[=(gpl "AutoGen" "")=]

@ignore
Permission is granted to process this file through TeX and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph.
@end ignore
@end ifinfo

@finalout
@titlepage
@title AutoGen - @value{TITLE}
@subtitle For version @value{VERSION}, @value{UPDATED}
@author Bruce Korb
@author @email{[=(shellf "echo '%s' | sed 's,@,@@,g'" e-addr )=]}

@page
@vskip 0pt plus 1filll
AutoGen copyright @copyright{} @value{COPYRIGHT} Bruce Korb
@sp 2
This is the second edition of the GNU AutoGen documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=(gpl "AutoGen" "")=]
@end titlepage
[=

DEFINE id-file =]
@ignore
Generated [= (tpl-file-line) =].
Extracted from [=srcfile=] line [=linenum=].
@end ignore[=

ENDDEF  =][=


(define text-tag "")=][=

DEFINE get-text     =][=

(set! text-tag
   (string-append "@ignore\n%s == "
      (string-upcase! (get "tag")) " == %s or the surrounding 'ignore's\n"
      "Extraction from autogen.texi\n"
      "@end ignore" ))

(extract texi-file-source text-tag) =][=

ENDDEF get-text     =][=

get-text tag = main =]
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

[= get-text tag = COMMENTS =]

@node    Full Syntax
@section YACC Language Grammar

The preprocessing directives and comments are not
part of the grammar.  They are handled by the scanner/lexer.
The following was extracted directly from the defParse.y source file:

@ignore
Extracted from $top_srcdir/agen5/defParse.y
@end ignore
@example
[= # extract the syntax from defParse.y, then escape the characters
     that texi sees as operators and remove comments:  =][=
`if test -z "$top_srcdir" || test ! -d "$top_srcdir"
 then top_srcdir=.. ; fi
 [ ! -f ${top_srcdir}/agen5/defParse.y ] && kill -2 ${AG_pid}
 sed -n -e '/^definitions/,$p' ${top_srcdir}/agen5/defParse.y |
 sed -e 's/@/@@/g;s/{/@{/g;s/}/@}/g' -e '/^\\/\\*/,/^ \\*\\//d' ` =]
@end example

[= get-text tag = TEMPLATE =]

@ignore

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
[=
(define func-name "")
(define func-str "") =][=

DEFINE set-func-name =][=
  (set! func-name (shell (sprintf "echo '%s' |
    sed -e 's/-p$/?/' -e 's/-x$/!/' -e 's/-to-/->/'"
    (string-tr! (get "name") "A-Z_^" "a-z--") )) )

  (set! func-str
      (if (exist? "string") (get "string") func-name)) =][=

ENDDEF =]
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
(define func-name "")
(define func-str "") =][=
FOR gfunc =][=
  (if (not (exist? "name")) (error "NO NAME")) =][=
  IF (not (exist? "general_use")) =][=
    set-func-name =]
* SCM [= (sprintf "%-22s" (string-append func-str "::"))
  =][= (string-append "@file{" func-name "} - " (get "what")) =][=
  ENDIF =][=
ENDFOR gfunc =]
* SCM make-header-guard::   @file{make-header-guard} - protect a header file
* SCM autogen-version::     @file{autogen-version} - ``[= version =]''
@end menu

[=
FOR gfunc =][=
  IF (not (exist? "general_use")) =][=
    set-func-name =]
@node SCM [= (. func-str) =]
@subsection [= (string-append "@file{" func-name "} - " (get "what")) =]
@findex [=(. func-name)=][=
% string "\n@findex %s" =]
[= id-file     =]
Usage:  ([=(. func-str)=][=
    FOR exparg =] [=
      arg_optional "[ " =][=arg_name=][= arg_list " ..." =][=
      arg_optional " ]" =][=
    ENDFOR exparg =])
@*
[= string (string-append func-name "@:  ") =][=doc=]
[=
    IF (exist? "exparg") =]
Arguments:[=
      FOR exparg =]
@*
[=arg_name=] - [=
    arg_optional "Optional - " =][=
        IF (exist? "arg_desc") =][=arg_desc=][=
        ELSE=]Undocumented[=
        ENDIF=][=
      ENDFOR exparg =][=
    ELSE
    =]
This Scheme function takes no arguments.[=
    ENDIF =][=
  ENDIF general_use =][=
ENDFOR gfunc
=]
@ignore
Generated [= (tpl-file-line) =].
@end ignore

@node SCM make-header-guard
@subsection @file{make-header-guard} - make self-inclusion guard
@findex make-header-guard
@findex header-file
@findex header-guard

Emit a @code{#ifndef}/@code{#define} sequence based upon the output
file name and the provided prefix.  It will also define a scheme
variables named, @code{header-file} and @code{header-guard}.
The @code{#define} name is composed as follows:

@enumerate
@item
The first element is the string argument and a separating underscore.
@item
That is followed by the name of the header file with illegal characters
mapped to underscores.
@item
The end of the name is always, "@code{_GUARD}".
@item
Finally, the entire string is mapped to upper case.
@end enumerate

The final @code{#define} name is stored in an SCM symbol named
@code{header-guard}.  Consequently, the concluding @code{#endif} for the
file should read something like:

@example
#endif /* [+ (. header-guard) +] */
@end example

The name of the header file (the current output file) is also stored in an SCM
symbol, @code{header-file}.  Therefore, if you are also generating a
C file that uses the previously generated header file, you can put
this into that generated file:

@example
#include "[+ (. header-file) +]"
@end example

Obviously, if you are going to produce more than one header file from
a particular template, you will need to be careful how these SCM symbols
get handled.

Arguments:
@*
prefix - first segment of @code{#define} name

@node SCM autogen-version
@subsection @file{autogen-version} - autogen version number
@findex autogen-version

This is a symbol defining the current AutoGen version number string.
It was first defined in AutoGen-5.2.14.
It is currently ``[= version =]''.

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

@menu[=
(define func-name "")
(define func-str "") =][=
FOR gfunc =][=
  IF (exist? "general_use") =][=
    set-func-name =]
* SCM [= (sprintf "%-20s" (string-append func-str "::"))
  =][= (string-append "@file{" func-name "} - " (get "what")) =][=
  ENDIF =][=
ENDFOR gfunc =]
* SCM html-escape-encode::  @file{html-escape-encode} - escape special chars
@end menu

[=
FOR gfunc =][=
  IF (exist? "general_use") =][=
    set-func-name =]
@node SCM [= (. func-str) =]
@subsection [= (string-append "@file{" func-name "} - " (get "what")) =]
@findex [=(. func-name)=][=
% string "\n@findex %s" =]
[= id-file     =]
Usage:  ([=(. func-str)=][=
    FOR exparg =] [=
      arg_optional "[ " =][=arg_name=][= arg_list " ..." =][=
      arg_optional " ]" =][=
    ENDFOR exparg =])
@*
[= string (string-append func-name ":  ") =][=doc=]
[=
    IF (exist? "exparg") =]
Arguments:[=
      FOR exparg =]
@*
[=arg_name=] - [=
    arg_optional "Optional - " =][=
        IF (exist? "arg_desc") =][=arg_desc=][=
        ELSE=]Undocumented[=
        ENDIF=][=
      ENDFOR exparg =][=
    ELSE
    =]
This Scheme function takes no arguments.[=
    ENDIF =][=
  ENDIF general_use =][=
ENDFOR gfunc
=]

@node SCM html-escape-encode
@subsection @file{html-escape-encode} - escape special chars
@findex html-escape-encode

Usage:  (html-escape-encode str)
@*
Substitute escape sequences for characters that are special to HTML/XML.
It will replace "@code{&}", "@code{<}" and "@code{>}" with the strings,
"@code{&amp;}", "@code{&lt;}", and "@code{&gt;}", respectively.

Arguments:
@*
str - string to transform

[= get-text tag = MACROS =]
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

That is, the @code{INVOKE} syntax is either:
@example
<user-macro-name> [ <name> [ = <expression> ] ... ]
@end example
@noindent
or
@example
INVOKE <name-expression> [ <name> [ = <expression> ] ... ]
@end example

@item
AutoGen FOR macros must be in one of two forms:

@example
FOR <name> [ <separator-string> ]
@end example
@noindent
or
@example
FOR <name> (...Scheme expression list)
@end example
@noindent
where @code{<name>} must be a simple name and the Scheme expression list
is expected to contain one or more of the @code{for-from},
@code{for-to}, @code{for-by}, and @code{for-sep} functions.
(@xref{FOR}, and @ref{AutoGen Functions})

@item
AutoGen @code{DEFINE} macros must be followed by a simple name.
Anything after that is ignored.  @xref{DEFINE}.

@item
The AutoGen @code{COMMENT}, @code{ELSE}, @code{ESAC} and the @code{END*}
macros take no arguments and ignore everything after the macro name
(e.g. see @ref{COMMENT})
@end enumerate
[=

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

[= get-text tag = augmenting =]

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

[= get-text tag = installation =]

@page
@node AutoOpts
@chapter Automated Option Processing
@cindex autoopts

AutoOpts [=`
eval "\`egrep '^AO_[A-Z]*=' ${top_srcdir}/VERSION\`" 2> /dev/null
echo ${AO_CURRENT}.${AO_REVISION}
`=] is bundled with AutoGen.  It is a tool that virtually eliminates
the hassle of processing options and keeping man pages, info docs and
usage text up to date.  This package allows you to specify several program
attributes, up to a hundred option types and many option attributes.
From this, it then produces all the code necessary to parse and handle
the command line and initialization file options, and the documentation
that should go with your program as well.

[= get-text tag = autoopts =]
[= `
f=../autoopts/libopts.texi
[ ! -f $f ] && f=${top_srcdir}/autoopts/libopts.texi
[ -f $f ] || {
  echo "Cannot locate libopts.texi" >&2
  kill -n $AG_pid
  exit 1
}
cat $f` =]
[= get-text tag = "autoopts-data" =]

@example
[= `

[ ! -d .tmp ] && mkdir .tmp
OPTDIR=\`cd ${top_srcdir}/autoopts ; pwd\`

libs="\`cd ${OPTDIR} ; [ -d .libs ] && cd .libs ; pwd\`"
if [ -f ${libs}/libopts.a ]
then libs="${libs}/libopts.a"
else libs="-L ${libs} -lopts"
fi

opts="-o genshellopt -DTEST_GETDEFS_OPTS -g -I${OPTDIR}"

( cat ${top_srcdir}/getdefs/opts.def
  echo "test_main = 'putShellParse';"
) | (
  cd .tmp
  HOME='' ${AGEXE} -t40 -L${OPTDIR} -bgenshellopt -- -

  ${CC} ${opts} genshellopt.c ${libs} -lm
) > /dev/null 2>&1

( .tmp/genshellopt --help 2>&1 ) |
  sed -e 's,\t,        ,g;s,@,@@,g;s,{,@{,g;s,},@},g'
  rm -rf .tmp

` =]
@end example
[= get-text tag = autoinfo =]

@menu
* AutoFSM::                        Automated Finite State Machine
* AutoXDR::                        Combined RPC Marshalling
* AutoEvents::                     Automated Event Management
[=`cat  ${ADDON_MENU}`=]
@end menu

[= get-text tag = autofsm =][=
`
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
[= get-text tag = Future =][=

(shell "[ -f autogen.texi.ori ] && rm -f autogen.texi.ori")
(set-writable #t)

;; Local Variables:
;; indent-tabs-mode: nil
;; mode: texinfo
;; End:

=]
