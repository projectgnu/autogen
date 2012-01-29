[= AutoGen5 Template -*- Mode: C -*-

c

# This dispatch template is
# copyright (c) 2005-2012 Bruce Korb
# all rights reserved.

=]
[= INVOKE Initialize                \=][=# /*

DESCRIPTION:
Call a function based on an input name
The input name is converted to an enumeration value and used as an index
into a dispatch table.

PURPOSE:
   This template will produce a header file and one or two source files.
   You supply a list of commands as values for "cmd" and it will produce
   an enumeration of these commands and code that will match strings against
   these command names.  The input command name need only be as long as
   needed for a unique partial match.  For example, if there is only one
   command that starts with "x", then the single letter "x" is sufficient
   for a unique match.

YOU SUPPLY:
   an AutoGen definitions file.  It must contain a list of values for
   "cmd".  e.g.:  cmd = first, second, third;

   You may optionally supply any of the following:

   "copyright"  See the AutoOpts documentation for its usage.
   "ident"      is inserted at the top of all output files.
   "c-text"     is inserted at the top of the .c files
   "static"     produce one file with all scoping set to "static".
                If set to the value "handler", then the handler procedures
                are presumed to be static as well.  They default to
                "extern" scope.
   "no-gperf"   Do not use gperf for creating the lookup procedure.
   "no-partial" Do not match partial matches (By default, a unique partial
                match will be considered a match.)
   "abbrev"     is used to construct the name for an invalid value handler

   "dispatch"   describes the format of handler functions:
      "fmt"     printf format string for constructing the command name.
      "arg"     The type and name of argument(s) to handler functions.
                The argument name must be separated from type information
                by at least one space character.
      "ret"     the function return type.

   The handler functions are expected to have the following profile,
   assuming we are expanding the "cmd" named "mumble" and "dispatch" has:
      ret = int;
      fmt = 'cmd_%s_hdlr';
      arg = 'void * cookie';

   then:
      extern int cmd_mumble_hdlr(char const * cmd, void * cookie);
   though "cmd" will point past the command name and any following white
   space.

   The values "static", "no-gperf", "no-partial" and "dispatch" may each be
   either present or absent.  Consequently, there are 16 flavors of output.

THE TEMPLATE PRODUCES:

   <base-name> is based on the name of your definitions file:

   <base-name>.h    The enumeration of the commands in the form:
                    <BASE_NAME>_<COMMAND> of type <base_name>_enum_t

                    External declarations of the two or three generated
                    procedures.  (dispatch_<base_name> is produced IFF
                    there is a "dispatch" value.  This text is incorporated
                    into the <base-name>.c file if "static" is specified.

   <base-name>.c    The code for the two or three procedures.

   <base-name>-hdlr.c  This is produced if there is a "dispatch" value and if
                    the environment variable EMIT_DISPATCH is set.

   If your commands include one named, "help", then an input string starting
   with a "?" will match the help command.

   If your commands include one named, "null", then an empty or all spaces
   input string will match the null command.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

\=]
[= INVOKE preamble                  \=]
[= IF (not (exist? "static"))       \=]
[=    INVOKE start-header           \=]
[= ENDIF  not static text           \=]
[= INVOKE enum-and-prototypes       \=]
[= IF (not (exist? "static"))       \=]
[=    (ag-fprintf 0 "\n\n#endif /* %s */\n/* end of %s */\n"
                    header-guard (out-name))
      (out-pop)                      =]
#include "[= (. header-file) =]"
[= ENDIF                            \=]
[= c-text                           \=]
[= INVOKE string-to-enum            \=]
[= INVOKE enum-to-string            \=]
[= IF (exist? "dispatch")           \=]
[=    INVOKE dispatch-proc          \=]
[= ENDIF (exist? "dispatch")        \=]
/* end of [= (out-name) =] */
[= # /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE Initialize   =][=

   (define tp-nm    (string-downcase! (if (exist? "type")
           (string->c-name! (get "type"))
           (string->c-name! (base-name)) )))

   (define enum-nm  (string-append tp-nm "_enum_t"))
   (define TYPE     (string-upcase (if (exist? "prefix")
           (string->c-name! (get "prefix"))
           tp-nm )))

   (define INVALID  (string-append TYPE "_" (if (exist? "invalid")
     (string-upcase! (string->c-name! (get "invalid"))) "INVALID" )))

   (define proc-mode (if (exist? "static") "static" "extern"))
   (define name-len  0)
   (define max-len   0)
   (define min-len   256)
   (define cmd-ct    (count "cmd"))
   (define clist     (string->c-name! (join "\n" (stack "cmd"))))
   (define cmd-name  "")
   (out-push-new)   =][=

   FOR cmd          =][=

     (set! cmd-name (string->c-name! (get "cmd")))
     (set! name-len (string-length cmd-name))
     (if (> name-len max-len)
         (set! max-len name-len) )
     (if (< name-len min-len)
         (set! min-len name-len) ) =][=

     CASE cmd       =][=
     =  help        =][= INVOKE spec-char-cmd  spec-char = "'?'" =][=
     =  null        =][= INVOKE spec-char-cmd  spec-char = "NUL" =][=
     ESAC cmd       =][=

   ENDFOR           =][=

   (define special-char-handling (out-pop #t))
   (if (not (defined? 'special-char-handling ;; '
       )    )
       (set! special-char-handling "") ) =][=

ENDDEF Initialize

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE Preamble                 =][=

(dne " * " "/*")                =][=

  IF (exist? "copyright")       =][=
  CASE (get "copyright.type")   =][=

    =  gpl  =]
 *
[= (gpl  prog-name " * " )      =][=

    = lgpl  =]
 *
[=(lgpl prog-name (get "copyright.owner") " * " ) =][=

    =  bsd  =]
 *
[=(bsd  prog-name (get "copyright.owner") " * " ) =][=

    = note  =]
 *
 * Copyright (c) [= copyright.years =] by [= copyright.owner =]
 * All rights reserved.
 *
[=(prefix " * " (get "copyright.text"))=][=

    *       =] * <<unspecified license type>>[=

  ESAC                          =][=
  ENDIF  copyright exists       =]
 *
 *  Recognizer generation options:
 *
 *  [= (if (exist? "static") "static" "external") =] scope
 *  [= (if (exist? "no-gperf") "binary search" "gperf generated") =]
 *  partial name matches [= (if (exist? "no-partial")
    "are not matched" "are matched, if the match is unique") =][=
  IF (exist? "dispatch") =]
 *  A dispatch routine is generated that will call handler routines[=
  ENDIF =]
 */
[= (stack-join "\n" "ident")        =][=

ENDDEF Preamble

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE start-header                 =][=

   (define header-file (if (exist? "header-file") (get "header-file")
   (shellf "echo '%s' | sed 's/.c$/.h/'" (out-name)) ))

   (out-push-new header-file)       =][=
   INVOKE  preamble                 =][=
   (make-header-guard "recognizer") =][=

ENDDEF start-header

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE enum-and-prototypes      \=]

#include <ctype.h>
#include <string.h>

typedef enum { [= (. INVALID) =] = 0,
[=

 ;;  "clist" contains the names of the commands in C program name format.
 ;;  Upper case them, prepend "TYPE_" to the front and comma separate them.
 ;;  We now have our list of enumeration names.
 ;;
 (shell (string-append
    "( tr a-z A-Z | "
    "  columns -I4 --spread=1 --sep=, --format='" TYPE "_%s'\n"
    ") <<_EOCmds_\n" clist "\n_EOCmds_" ))

=]
} [= (. enum-nm) =];

#define [= (. TYPE) =]_COUNT [= (count "cmd") =]

[= (define string-to-enum-proto
    (sprintf "%s %s\n%s_enum(char const * name, char const ** next)"
             proc-mode enum-nm tp-nm) )
   string-to-enum-proto =];

[= (define enum-to-string-proto
    (sprintf "%s char const *\n%s_name(%s cmd)"
             proc-mode tp-nm enum-nm) )
   enum-to-string-proto =];[=


  IF (exist? "dispatch")    =][=
    FOR dispatch            =][=

      IF (define arg-list  "char const * cmd")
         (define call-list "cmd")
         (exist? "arg")     =][=

        FOR arg             =][=

          (set! arg-list (string-append arg-list ", " (get "arg")))
          (set! call-list (string-append call-list ", "
                (shellf "echo '%s' | sed $'s/.*[ \t*]//'" (get "arg")) ))
          =][=

        ENDFOR              =][=

      ENDIF

=]
[= (define dispatch-proc-proto
    (sprintf "%s %s\ndispatch_%s(%s)"
             proc-mode (get "ret") tp-nm arg-list) )
   dispatch-proc-proto      =];[=

    ENDFOR dispatch         =][=

  ENDIF  dispatch exists    =]
[=

ENDDEF enum-and-prototypes

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE string-to-enum           \=][=

  (define ix 0)
  (define enum-list "")
  (define sort-list "")

  =][=

  FOR  cmd      =][=

    (set! cmd-name (get "cmd"))
    (set! enum-list (string-append enum-list (sprintf "%d\n"             ix)))
    (set! sort-list (string-append sort-list (sprintf "%s %d\n" cmd-name ix)))
    =][=

  ENDFOR        =][=

  INVOKE emit-search-code      \=]

[= (. string-to-enum-proto) =]
{
    size_t nmlen = 0;
    char   name_buf[[= (+ max-len 1) =]];

    /*
     * Skip leading white space and figure out how many name characters
     * there are.  It cannot be longer than the longest name in our table.
     */
    while (isspace(*name))   name++;

    if (isalpha(*name)) {
        char const * ps = name;
        char * pd = name_buf;
        for (;;) {
            char ch = *(ps++);
            if (isupper(ch))
                *(pd++) = _tolower(ch);

            else if (isalnum(ch))
                *(pd++) = ch;

            else switch (ch) {
                case '-':
                case '_': *(pd++) = '_'; break;
                default:  goto name_scan_done;
            }

            if (++nmlen > [= (. max-len) =])
                return [= (. INVALID) =];
        } name_scan_done:;
        *pd = '\0';
    }

    {
        struct [=(. tp-nm)=]_index const * xlat = [=
                 (. tp-nm) =]_find(name_buf, nmlen);
        if (xlat != NULL) {
            if (next != NULL) {
                name += nmlen;
                while (isspace(*name))   name++;
                *next = name;
            }

            return ([= (. enum-nm) =])xlat->idx;
        }
    }
[= IF (or (exist? "no-gperf") (exist? "no-partial")) =]
    return [= (. INVALID) =];
[= ELSE    \=]

    /* check for partial match: */
    {
        int av;
        int hi = [= (. cmd-ct) =] - 1;
        int lo = 0;
        for (;;) {
            int cmp;
            av = (hi + lo) / 2;
            cmp = strncmp([= (. table-name) =][av].name, name_buf, nmlen);
            if (cmp == 0)
                break;
            if (cmp < 0)
                lo = av + 1;
            else
                hi = av - 1;
            if (hi < lo)
                return [= (. INVALID) =];
        }

        if ((av > 0) &&
            (strncmp([= (. table-name) =][av-1].name, name_buf, nmlen) == 0))
            return [= (. INVALID) =];

        if ((av < [= (- cmd-ct 1) =]) &&
            (strncmp([= (. table-name) =][av+1].name, name_buf, nmlen) == 0))
            return [= (. INVALID) =];

        if (next != NULL) {
            name += nmlen;
            while (isspace(*name))   name++;
            *next = name;
        }

        return ([= (. enum-nm) =])[= (. table-name) =][av].idx;
    }
[= ENDIF  \=]
}
[=

ENDDEF string-to-enum

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE emit-search-code         =][=

  IF (exist? "no-gperf")

=]
typedef struct [=(. tp-nm)=]_index {
    char const * name; int const idx;
} [=(. tp-nm)=]_index_t;

static [=(. tp-nm)=]_index_t const *
[=(. tp-nm)=]_find(char const * tstr, unsigned int len)
{
[= (out-push-new) \=]
[=   INVOKE sorted-name-table \=]
[= (prefix "    " (out-pop #t)) =]
[= IF (not (exist? "no-partial")) =]
    int av;
[= ENDIF =]
    {
        int hi = [= (- cmd-ct 1) =];
        int lo = 0;
        do  {
            int cmp;
            [= (if (exist? "no-partial") "int ")
            =]av  = (hi + lo) / 2;
            cmp = strcmp([= (. table-name) =][av].name, tstr);
            if (cmp == 0)
                return [= (. table-name) =] + av;
            if (cmp < 0)
                lo = av + 1;
            else
                hi = av - 1;
        } while (hi >= lo);
    }
[= IF (exist? "no-partial") =]
    return NULL;
[= ELSE =]
    {
        int mct = 0;
        int ix;

        if ((av > 0) &&
            (strncmp([= (. table-name) =][av-1].name, tstr, len) == 0)) {
            mct++;
            ix = av - 1;
        }

        if ((av < [= (- cmd-ct 1) =]) &&
            (strncmp([= (. table-name) =][av+1].name, tstr, len) == 0)) {
            mct++;
            ix = av + 1;
        }

        if (strncmp([= (. table-name) =][av].name, tstr, len) == 0) {
            mct++;
            ix = av;
        }

        if (mct != 1)
            return NULL;

        return [= (. table-name) =] + ix;
    }
[= ENDIF \=]
}
[=

  ELSE  use gperf   =]
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  The hash code was generated by "gperf(1GNU)" from the following:
 *
[= ;; */
  (make-gperf tp-nm clist)
  (emit (shellf "sed '/^int main(/,$d;s/^/ *  /' ${gpdir}/%s.gperf"
                tp-nm))
  (emit "\n */\n")
  (gperf-code tp-nm)
=]
/*
 *  END of gperf code
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */[=
    IF (not (exist? "no-partial")) =][=
      INVOKE sorted-name-table     =][=
    ENDIF                          =][=

  ENDIF  use gperf/not          =][=

ENDDEF emit-search-code

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE sorted-name-table         =][=
  (define table-name (string-append tp-nm "_mapping_table")) =]
static struct [=(. tp-nm)=]_index const [= (. table-name) =][] = {
[=

    (define cmd-tbl-fmt (sprintf "    { %%-%ds %%5d },\n" (+ 3 max-len)))
    (out-push-new)  =][=

    FOR cmd         =][=
      (ag-fprintf 0 cmd-tbl-fmt (string-append
                  "\"" (get "cmd") "\",") (+ (for-index) 1)) =][=
    ENDFOR          =][=

    (shell (string-append
    "( sort | sed '$s/,$/ };/'\n) <<_EOF_\n"
    (out-pop #t)
    "_EOF_" ))      =]
[=

ENDDEF sorted-name-table

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE enum-to-string           \=]

[= (. enum-to-string-proto) =]
{
    /*
     *  Table of Command Names
     */
    static char const * const names[[=(+ cmd-ct 1)=]] = {
[= (shell (string-append
   "columns -I8 -S, --spread=1 -f'\"%s\"' <<\\_EOF_\n** INVALID **\n"
    clist "\n_EOF_"))
=] };

    if ((unsigned)cmd > [= (. cmd-ct) =])
        cmd = [= (. INVALID) =];

    return names[cmd];
}
[=

ENDDEF enum-to-string

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE dispatch-proc            =]

typedef [= (define ret-type (get "dispatch.ret"))
           ret-type =] (handle_[= (. tp-nm) =]_t)([= (. arg-list) =]);

[= CASE static =][=
   ==* handle =]static[=
     *        =]extern[=
   ESAC       =] handle_[= (. tp-nm) =]_t [=
    (define  cmd-fmt (if (exist? "fmt") (get "fmt") "do_%s"))
    (define inval-cmd (string-append "inval_"
            (if (exist? "abbrev") (get "abbrev") tp-nm)  ))
    (sprintf cmd-fmt inval-cmd) =],
[=
(define cmd-procs (shellf

"columns -I4 --spread=1 --sep=, --format='%s'<<_EOF_\n%s\n_EOF_"
  cmd-fmt (string->c-name! (join "\n" (stack "cmd")))

))

cmd-procs

=];

static handle_[= (. tp-nm) =]_t * const hdl_procs[[= (+ 1 cmd-ct) =]] = {
    [= (sprintf cmd-fmt inval-cmd) =],
[= (. cmd-procs) =] };

[= (. dispatch-proc-proto) =]
{
    [= (. enum-nm) =]     id  = [= (. tp-nm) =]_enum(cmd, &cmd);
    handle_[= (. tp-nm) =]_t * hdl = hdl_procs[id];
    [=
     (if (= (get "ret") "void")
         (sprintf "(*hdl)(%s)"        call-list)
         (sprintf "return (*hdl)(%s)" call-list)
         ) =];
}
[=

  IF (getenv "EMIT_DISPATCH")  =][=
    INVOKE Handler     =][=
  ENDIF                =][=

ENDDEF dispatch-proc

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE Handler          =][=

 (out-push-new (string-append (base-name) "-hdlr.c"))
 (define fmt-fmt (sprintf
    "handling %%-%ds (%%d) cmd - args:  '%%%%s'\\n" max-len))
 
 (set-writable)

\=]
[= INVOKE preamble      =]
#include <stdio.h>[=

    (set! cmd-fmt (string-append
          ret-type "\n" cmd-fmt "(" arg-list ")"))
    (if (= (get "static") "handler")
        (set! cmd-fmt (string-append "static " cmd-fmt)) )

    (if (and (defined? 'header-file) ;;'
              (> (string-length header-file) 1))
         (sprintf "\n#include \"%s\"" header-file))

=]
[= dispatch.c-text =][=

   FOR cmd              =][=
      IF (not (= (get "cmd") "help")) =]

[= (sprintf cmd-fmt (get "cmd")) =]
{
#ifdef DEBUG
    printf("[= (sprintf fmt-fmt (get "cmd") (+ 1 (for-index)))
            =]", cmd);
#else
    fputs("[= cmd =]\n", stdout);
#endif
    return ([= (. ret-type) =])0;
}
[=

      ENDIF not "help" \=]
[= ENDFOR cmd          \=]
[= IF (match-value? = "cmd" "help")     =][=
      IF (getenv "SHOW_NOHELP")         =][=
                (define show-nohelp #t) =][=
      ELSE =][= (define show-nohelp #f) =][=
      ENDIF=]

[= (define help-fmt (sprintf "%%-%ds%%s\n"
      (- (+ max-len 4) (remainder max-len 4)) ))
   (define cmd-name "")

   (sprintf cmd-fmt "help") =]
{
    static char const h_txt[ [=
        (out-push-new)      =][=
     FOR cmd                =][=
        (set! cmd-name (get "cmd"))
        (if (exist? (string-append cmd-name "-help"))
            (sprintf help-fmt cmd-name (get (string-append cmd-name "-help")) )
            (if (= cmd-name "help")
                (sprintf help-fmt cmd-name "Display this help text")
                (if show-nohelp
                    (sprintf help-fmt cmd-name "No provided help")
                    "")
        )   )               =][=
     ENDFOR each "cmd"      =][=

    (define help-text (out-pop #t))
    (string-length help-text)   =] ] =
       [= (c-string help-text)  =];
    if (*cmd != '\0')
        printf("help args:  %s\n", cmd);
    fwrite(h_txt, [= (string-length help-text) =], 1, stdout);
    return ([= (. ret-type) =])0;
}
[= ENDIF have-a-help cmd    =]

[= (sprintf cmd-fmt inval-cmd) =]
{
    char z[[= (+ 4 max-len) =]];
    {
        char * p = z;
        int    l = 0;
        for (;;) {
            int ch = *(cmd++);
            if ((! isalnum(ch)) &&
                (ch != '-') &&
                (ch != '_'))
                break;
            *(p++) = ch;
            if (++l >= [=(. max-len)=]) {
                *(p++) = '.';
                *(p++) = '.';
                *(p++) = '.';
                break;
            }
        }
        *p = '\0';
    }
    printf("Invalid command:  '%s'\n", z);
    return ([= (. ret-type) =])~0;
}
/*
 *  You are encouraged to remove this main procedure, fill out the
 *  dispatch procedures and not regenerate this again.
 */
int
main( int argc, char** argv )
{
    while (--argc > 0)
        dispatch_[= (. tp-nm) =]( *++argv[=FOR dispatch.arg=], 0[=ENDFOR=] );
    return 0;
}
[= (out-pop)
   (set-writable #f)    =][=

ENDDEF Handler

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ =][=

DEFINE spec-char-cmd            =]
        case [= spec-char =]:  /* [= (. cmd-name) =] command alias */
            res = [= (. TYPE) =]_[= (string-upcase! cmd-name) =];[=
  IF (not (= cmd-name "null"))  =]
            name++;[=
  ENDIF is cmd-name not null?   =]
            break;
[=

ENDDEF spec-char-cmd

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ \=]
