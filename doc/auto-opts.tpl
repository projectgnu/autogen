[= -*- Mode: texinfo -*-

 AutoGen5 Template

#  This file is part of AutoGen.
#  AutoGen Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
#
#  AutoGen is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  AutoGen is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program.  If not, see <http://www.gnu.org/licenses/>.

=]
@page
@node AutoOpts
@chapter Automated Option Processing
@cindex autoopts

AutoOpts [=
(make-tmp-dir)
(out-push-new)
=]
test ${#AGexe} -eq 0 -o ${#top_srcdir} -eq 0 -o ${top_builddir} -eq 0 && \
     die "AGexe, top_srcdir and top_builddir must be set"
ag_cmd="${AGexe} -L${top_srcdir}/autoopts/tpl"
test "X${top_srcdir}" = "X${top_builddir}" || \
     ag_cmd="${ag_cmd} -L${top_builddir}/autoopts/tpl"
readonly ag_cmd

run_ag() {
    echo ${ag_cmd} "$@" >&2
    ${ag_cmd} "$@"
}

eval "`egrep '^AO_[A-Z]*=' ${top_srcdir}/VERSION`" 2> /dev/null
echo ${AO_CURRENT}.${AO_REVISION}
[=

(shell (out-pop #t))

=] is bundled with AutoGen.  It is a tool that virtually eliminates the
hassle of processing options and keeping man pages, info docs and usage text
up to date.  This package allows you to specify several program attributes, up
to a hundred option types and many option attributes.  From this, it then
produces all the code necessary to parse and handle the command line and
configuration file options, and the documentation that should go with your
program as well.
[=

INVOKE  get-text tag = autoopts

=]
@noindent
First, specify your program attributes and its options to AutoOpts,
as with the following example.

@example
[=

(out-push-new (string-append tmp-dir "/checkopt.def" ))

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
};
[= (texi-escape-encode (out-pop #t)) \=]
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
[= (out-push-new) \=]
cd ${tmp_dir}

test -f checkopt.def || die cannot locate checkopt.def
test -f check && rm -f check
cat >> checkopt.def <<- _EOF_
	include = '#include "compat/compat.h"';
	_EOF_
{
  run_ag checkopt.def
  opts="-o check -DTEST_CHECK_OPTS ${CFLAGS} ${INCLUDES}"
  ${CC:-cc} -include ${top_builddir}/config.h ${opts} checkopt.c ${LIBS}
} > checkopt.err 2>&1

test -x ./check || {
  cat checkopt.err >&2
  die cannot create checkopt program
}

./check --help | sed 's/\t/        /g'
[=

(texi-escape-encode (shell (out-pop #t)))

=]
@end example
[=

INVOKE  get-text tag = autoopts-main

=]
Here is an example program that uses the following set of definitions:

@example
[=

 (out-push-new (string-append tmp-dir "/default-test.def" ))

=]AutoGen Definitions options;

prog-name  = default-test;
prog-title = 'Default Option Example';
homerc     = '$$/../share/default-test', '$HOME', '.';
environrc;
long-opts;
gnu-usage;
usage-opt;
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
[= (out-push-new) \=]
set -x
log_file=${tmp_dir}/ao-doc-log
exec 7>&2 ; exec 2>> ${log_file}
TOPDIR=`cd ${top_builddir} >/dev/null ; pwd`
OPTDIR=${TOPDIR}/autoopts

{
  cd ${tmp_dir}
  chmod 666 *.def
  echo 'config-header = "config.h";' >> default-test.def
  HOME=${tmp_dir} run_ag default-test.def
  test -f default-test.c || die 'NO default-test.c PROGRAM'

  opts="-o default-test -DTEST_DEFAULT_TEST_OPTS ${INCLUDES}"
  ${CC:-cc} ${CFLAGS} ${opts} default-test.c ${LIBS}

  test -x ./default-test || die 'NO default-test EXECUTABLE'
} >&2

test $? -eq 0 || {
  printf '\n\ncannot build default test\n'
  cat $log_file
  die "cannot build AutoOpts doc"
} 2>&7 1>&7

HOME=${tmp_dir} ${tmp_dir}/default-test --help | \
   sed 's,	,        ,g;s,\([@{}]\),@\1,g'

exec 2>&7 7>&-
[=
 (shell (out-pop #t))
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
@noindent
Doing so with getdefs' option definitions yields this sample-getdefsrc file.
I tend to be wordy in my @code{doc} attributes:

@example
[= (texi-escape-encode (shell "
  cd ${tmp_dir}
  run_ag -Trc-sample.tpl ${top_srcdir}/getdefs/opts.def >/dev/null
  test -f sample-getdefsrc || die did not create sample-getdefsrc
  cat sample-getdefsrc
" )) =]
@end example
[=

INVOKE get-text tag = "ao-data1"

=]
@example[=
(out-push-new (string-append tmp-dir "/hello.c"))

\=]

#include <config.h>
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#ifdef   HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <autoopts/options.h>
int main(int argc, char ** argv) {
  char const * greeting = "Hello";
  char const * greeted  = "World";
  tOptionValue const * pOV = configFileLoad("hello.conf");

  if (pOV != NULL) {
    const tOptionValue* pGetV = optionGetValue(pOV, "greeting");

    if (  (pGetV != NULL)
       && (pGetV->valType == OPARG_TYPE_STRING))
      greeting = strdup(pGetV->v.strVal);

    pGetV = optionGetValue(pOV, "personalize");
    if (pGetV != NULL) {
      struct passwd * pwe = getpwuid(getuid());
      if (pwe != NULL)
        greeted = strdup(pwe->pw_gecos);
    }

    optionUnloadNested(pOV); /* deallocate config data */
  }
  printf("%s, %s!\n", greeting, greeted);
  return 0;
}
[= (texi-escape-encode (out-pop #t)) \=]
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
will produce the following output:

@example
[= (texi-escape-encode (shell "
cd ${tmp_dir}
${CC:-cc} -o hello hello.c ${CFLAGS} ${LIBS} ${LDFLAGS} || \
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

log=${tmp_dir}/genshellopt.log

(
  set -x
  opts="-o genshellopt -DTEST_GETDEFS_OPTS ${INCLUDES}"
  exec 3> ${tmp_dir}/genshellopt.def
  cat ${top_srcdir}/getdefs/opts.def >&3
  echo "test_main = 'optionParseShell';" >&3
  echo 'config-header = "config.h";' >&3
  exec 3>&-

  cd ${tmp_dir}
  HOME='' run_ag -t40 genshellopt.def
  test $? -eq 0 || die "autogen failed to create genshellopt.c - See ${log}"

  ${CC} ${CFLAGS} ${opts} genshellopt.c ${LIBS}
  test $? -eq 0 || die "could not compile genshellopt.c - See ${log}"
) > ${log} 2>&1

test -x ${tmp_dir}/genshellopt || \
  die "NO GENSHELLOPT PROGRAM - See ${log}"

${tmp_dir}/genshellopt --help > ${tmp_dir}/genshellopt.hlp

${tmp_dir}/genshellopt -o ${tmp_dir}/genshellopt.sh || \
  die cannot create ${tmp_dir}/genshellopt.sh

sedcmd='s,\t,        ,g;s,\\([@{}]\\),@\\1,g'
sed "${sedcmd}" ${tmp_dir}/genshellopt.hlp
cat <<- \_EOF_
	@end example

	@noindent
	Resulting in the following script:
	@example
	_EOF_

sed "${sedcmd}" ${tmp_dir}/genshellopt.sh

` =]
@end example
[=

INVOKE  get-text tag = autoinfo

=]
