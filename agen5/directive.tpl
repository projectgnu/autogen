[= AutoGen5 template -*- Mode: C -*-
# $Id: directive.tpl,v 4.8 2006/03/25 19:23:27 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h =]
[=

  (define ix 0)
  (define tmp-txt "")
  (define dir-tbl "")
  (define dir-enm "_EOF_\n")
  (define dir-nms "_EOF_\n")

  (string-append
     (dne " *  " "/*  ")
     "\n *\n *  copyright 1992-2006 Bruce Korb\n *\n"
     (gpl "AutoGen" " *  ")
     "\n */\n"
     (make-header-guard "directive")
  )

=]
#ifdef DEFINING

typedef char* (tDirProc)( char* pzArg, char* pzScan );

typedef struct dir_table tDirTable;
struct dir_table {
    size_t      nameSize;
    tCC*        pzDirName;
    tDirProc*   pDirProc;
    int         unused;
};

/*
 *  Declare the procedures that will handle the directives.
 */
static tDirProc doDir_IGNORE;[=
FOR directive    =][=

  (set! tmp-txt (get "name"))
  (set! dir-tbl (string-append dir-tbl
        (sprintf "    { %2d, zDirectives +%3d, doDir_%-10s 0 }"
                 (string-length tmp-txt) ix
                 (string-downcase! (string-append (get "name") ","))  )
        (if (last-for?) " };" ",\n")
  )     )

  (set! dir-enm (string-append dir-enm
                "DIR_" (string-upcase! (get "name")) "\n" ))

  (set! dir-nms (string-append dir-nms
                 " \"" tmp-txt "\\0\"\n" ))

  (set! ix (+ ix (string-length tmp-txt) 1))

=][=

  IF (not (exist? "dummy")) =]
static tDirProc doDir_[=name=];[=
  ELSE           =]
#define         doDir_[=name=] doDir_IGNORE[=
  ENDIF          =][=
ENDFOR directive =]

/*
 *  Define the constant string names for each directive.
 *  We supply all the needed terminating NULs, so tell the compiler
 *  the size to allocate.
 */
static const char zDirectives[[=(. ix)=]] =
[= (shellf "columns --spread=1 -I3 <<%s_EOF_" dir-nms) =];

/*
 *  Enumerate the directives
 */
typedef enum {
[= (shellf "columns -I4 -S, --spread=1 <<%s_EOF_" dir-enm) =]
} teDirectives;

/*
 *  Set up the table for handling each directive.
 */
#define DIRECTIVE_CT  [= (+ (high-lim "directive") 1) =]
static tDirTable dirTable[ DIRECTIVE_CT ] = {
[= (. dir-tbl) =]

/*
 *  This text has been extracted from [=`echo ${srcdir}/schemedef.scm`=]
 */
#define SCHEME_INIT_FILE [= (c-string (out-name)) =]
static const int  schemeLine = __LINE__+2;
static const char zSchemeInit[] =
[= (kr-string (shell

"sed -e \"s/AUTOGEN_VERSION/${AG_VERSION}/;s/^[ \t]*//\" \\
     -e '/^;/d;/^$/d' ${srcdir}/schemedef.scm" ))

 ;; "
=][= # " =]; /* for emacs: ' */ /* " */

/*
 *  The shell initialization string.  It is not in "const" memory because
 *  we have to write our PID into it.
 */
static char zShellInit[] =
    [= #  Things this scriptlett has to do:

1.  Open fd 8 as a duplicate of 2.  It will remain open.
    Divert 2 to /dev/null for the duration of the initialization.
2.  Do zsh and bash specific things to make those shells act normal.
3.  Trap a number of common signals so we can ignore them
4.  Make sure that the "cd" builtin does not emit text to stdout
5.  Set up a macro that prints a message, kills autogen and exits
6.  Restore stderr to whereever it used to be.

=][= (out-push-new)
=]exec 8>&2 2>/dev/null

if test -n "${ZSH_VERSION+set}" && (emulate sh) 1>&2
then
  emulate sh
  NULLCMD=:

else if test -n "${BASH_VERSION+set}" && (set -o posix) 1>&2
then
  set -o posix
fi ; fi

for f in 1 2 5 6 7 13 14
do trap "echo trapped on $f >&2" $f ; done

test -n "${CDPATH}" && {
  CDPATH=''
  unset CDPATH
}
( unalias cd ) 1>&2 && unalias cd
die() {
  echo $* >&8
  kill -TERM ${AG_pid}
  exit 1
}
exec 2>&8
AG_pid=[=
(kr-string (out-pop #t))=] "\000........."; /* K&R confuses emacs: " */


#if defined(SHELL_ENABLED)
/*
 *  "gperf" functionality only works if the subshell is enabled.
 */
[= (out-push-new) \=]
gpdir=.gperf.$$
test -d ${gpdir} || mkdir ${gpdir} || die "cannot mkdir ${gpdir}"
cd ${gpdir} || die cannot cd into ${gpdir}
gpdir=`pwd`
gperf_%2$s=${gpdir}/%2$s

( cat <<- '_EOF_'
	%%{
	#include <stdio.h>
	typedef struct index t_index;
	%%}
	struct index { char* name; int idx; };
	%%%%
	_EOF_

  idx=1
  while read f
  do echo "${f}, ${idx}"
     idx=`expr ${idx} + 1`
  done <<- _EOLIST_
%1$s
	_EOLIST_

  cat <<- '_EOF_'
	%%%%
	int main( int argc, char** argv ) {
	    char*    pz = argv[1];
	    t_index* pI = in_word_set( pz, strlen( pz ));
	    if (pI == NULL)
	        return 1;
	    printf( "0x%%02X\n", pI->idx );
	    return 0;
	}
	_EOF_
) > %2$s.gperf

gperf -t -D -k'*' %2$s.gperf > %2$s.c
test $? -eq 0 || die "gperf failed on ${gpdir}/%2$s.gperf"

res=`${MAKE-make} %2$s 2>&1`
test $? -eq 0 -a -x ${gperf_%2$s} || \
  die "could not build gperf program: ${res}"
[=
  (set! tmp-txt (out-pop #t))
  (emit (sprintf "static const char zMakeGperf[%d] =\n"
                 (+ 1 (string-length tmp-txt)) ))
  (kr-string tmp-txt)
=]; /* K&R confuses emacs: " */

[= (out-push-new) \=]
test -n "${gperf_%1$s}" || die 'no environment variable "gperf_%1$s"'
test -x "${gperf_%1$s}" || die "no gperf program named  ${gperf_%1$s}"
${gperf_%1$s} %2$s
[=
  (set! tmp-txt (out-pop #t))
  (emit (sprintf "static const char zRunGperf[%d] =\n"
                 (+ 1 (string-length tmp-txt)) ))
  (kr-string tmp-txt)
=]; /* K&R confuses emacs: " */
#endif

#ifdef DAEMON_ENABLED
typedef struct inet_family_map_s {
    const char*     pz_name;
    unsigned short  nm_len;
    unsigned short  family;
} inet_family_map_t;

[= `

list=\`find /usr/include -follow -name socket.h | \
 xargs egrep '^#define[ \t]+AF_[A-Z0-9]+[ \t]+[0-9]' | \
 sed 's,^.*#define[ \t]*AF_,,;s,[ \t].*,,;/^MAX$/d'\`

set -- $list
echo "#define INET_FAMILY_TYPE_CT $#"
echo "inet_family_map_t inet_family_map[ \`expr $# + 1\` ] = {"

for f
do
   g=\`echo $f | tr '[A-Z]' '[a-z]'\`':'
   ct=\`echo $g | wc -c\`
   printf "    { %-14s %3d, AF_${f} },\\n" "\\\"${g}\\\"," ${ct}
done | sort -u

`=]
    { NULL, 0, 0 } };

#endif /* DAEMON_ENABLED */
#endif /* DEFINING */
#endif /* [=(. header-guard)=] */
/*
 *  End of [= (out-name) =] */[= #

end of directive.tpl  =]
