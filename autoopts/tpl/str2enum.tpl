[= AutoGen5 Template h c -*- Mode: Scheme -*-

# This file contains the templates used to generate
# keyword parsing code

=][=
 (emit (dne " *  " "/*  ")) (emit "\n * \n")
 (if (exist? "copyright")
     (license-description
         (get "copyright.type" "unknown")
         (get "package" (shell "basename `pwd`"))
         " * "
         (get "copyright.owner" (get "copyright.author" "unknown")) )
     (license-description "mbsd" "str2enum" " * " "Bruce Korb")
)
 =][= # /*

;; DESCRIPTION:
;; Determine the enumeration value associated with a name.  This can then be
;; used to automatically call a dispatch procedure.  A procedure to convert
;; that integer back into a string can also be generated.

;; PURPOSE:
;;    This template will produce a header and a source file.
;;    You supply a list of commands as values for "cmd" and it will produce
;;    an enumeration of these commands and code that will match strings against
;;    these command names.

;; YOU SUPPLY:
;;   an AutoGen definitions file.  It must contain a list of values for
;;   "cmd".  e.g.:  cmd = first, second, third;

;;   You may optionally supply any of the following:

;;   copyright    See the AutoOpts documentation for its usage.
;;   partial      Accept partial matches, if unique.
;;   base-name    The base name is normally derived from the definitions
;;                file name.  This will override that and cause the
;;                output files to have this "base name".
;;   prefix       By default, the enumeration value uses the first segment
;;                of the base name as the prefix for each name.  This
;;                overrides that.
;;   alias        A special character can be used to represent a command. e.g.
;;                   alias = "< source"; alias = "# comment"; alias = "? help";
;;                will cause a '<' character to represent "source",
;;                a '#' to represent comment, and '?' for help.
;;                If a command is aliased to but not in the "cmd" list,
;;                then you must still provide a handler procedure, but it
;;                will only be accessible via its alias.
;;
;;   dispatch     describes the format of handler functions:
;;      d-nam     printf format string for constructing the command name.
;;                The formatting element must be "%s" and the rest of
;;                the format string must be valid for a C procedure name.
;;      d-arg     The type and name of argument(s) to handler functions.
;;                The argument name must be separated from type information
;;                by at least one space character (separate any asterisks),
;;                and place a comma between each argument next to the
;;                argument name.  This text gets inserted unmodified into
;;                the procedure definition.
;;      d-ret     the function return type.

;;      The handler functions will have the command enumeration as its first
;;      first argument, a pointer to a constant string that will be the
;;      character *after* the parsed command (keyword) name, plus any "d-arg"
;;      arguments that follow that.  In addition to providing a handler for
;;      all the named "cmd"s, you must provide one for "invalid".

;; THE TEMPLATE PRODUCES:

;;   <basename>.h The enumeration of the commands in the form:
;;                <PREFIX>_KWD_<COMMAND> of type <base_name>_enum_t
;;                plus two extra entries: <PREFIX>_INVALID_KWD, and
;;                <PREFIX>_COUNT_KWD.

;;                External declarations of the  generated
;;                procedure(s).

;;   <basename>.c The implementing code

;;   If your commands include one named, "null", then an empty or all spaces
;;   input string will match the null command.

;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

;; =][=

CASE (suffix)            =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                     =][=
   (define base-file-name (base-name))
   (define move-output-file #f)
   (if (exist? "base-name") (begin
       (set! base-file-name (get "base-name"))
       (out-move (string-append base-file-name ".h"))
       (set! move-output-file #t)
   ))
   (define base-enum-name (string->c-name! (string-downcase base-file-name)))
   (make-tmp-dir)        =]
 *
 * Command/Keyword Dispatcher
 *
[=(out-push-new)\=]
mk_char_list() {
    echo "$1" | sed 's/\(.\)/\1\
/g'
}
[=

(shell (out-pop #t))

(define pfx-str    "")
(define idx         0)
(define tmp-str    "")

(if (exist? "prefix")
    (set! pfx-str (get "prefix"))
    (begin
       (set! idx (string-index base-enum-name (string->char-set "_-^")))
       (if (number? idx)
           (set! pfx-str (substring/copy base-enum-name 0 idx))
           (set! pfx-str base-enum-name)
)   )  )
(define PFX-STR (string-upcase pfx-str))
(define enum-name (string-append base-enum-name "_enum_t"))
(define len-arg   (if (exist? "no-length") ""
                  ", unsigned int len"))

(define cmd-chars (shell (string-append
  "mk_char_list '" (join "" (stack "cmd")) "'"
)))

(if (exist? "no-case")
    (set! cmd-chars (string-append
          (string-downcase cmd-chars) "\n"
          (string-upcase cmd-chars) )) )
(set! cmd-chars (shell (string-append
    "echo '" cmd-chars "' | sort -u | tr -d '\n\t '" )))

(if (exist? "equate")
    (set! cmd-chars (string-append cmd-chars (get "equate")))
)

(define kwd-list (string->c-name! (join "\n" (stack "cmd"))))

(if (exist? "alias")
    (set! kwd-list (string->c-name! (shell (string-append
        "echo '" kwd-list "' > $tmp_dir/commands\n"
        "echo '" (join "\n" (stack "alias")) "' | "
        "while IFS='' read line ; do "
            "line=`echo \"$line\" | sed 's/. *//'`\n"
            "grep -E \"^${line}\\$\" $tmp_dir/commands >/dev/null || {\n"
                "echo \"$line\" >> $tmp_dir/commands\n}\n"
        "done\nsort -u $tmp_dir/commands"
)))))

(emit "\n */\n")
(make-header-guard "str2enum")=]

typedef enum {
    [= (define invalid-kwd (string-append PFX-STR "_INVALID_KWD"))
       invalid-kwd =][=
   (string-upcase! (shell (string-append
     "while read nm ; do printf ',\n    " PFX-STR
            "_KWD_%s' $nm ; done <<_EOF_\n"
     kwd-list
     "\n_EOF_" )))  =],
    [= (define cmd-count (string-append PFX-STR "_COUNT_KWD"))
       cmd-count =]
} [= (. enum-name) =];

extern [= (. enum-name)         =]
find_[=(. base-enum-name)=]_id(char const * str[=(. len-arg)=]);
[=

  IF (exist? "dispatch")        =]
typedef [= dispatch.d-ret =]([=(. base-enum-name)=]_handler_t)(
    [=
    (define disp-arg-list (string-append enum-name " id, char const * str"))
    (if (exist? "dispatch.d-arg")
        (set! disp-arg-list (string-append disp-arg-list ", "
                            (get "dispatch.d-arg") )) )
    disp-arg-list =]);

[=(. base-enum-name)=]_handler_t
[=
(if (not (exist? "dispatch.d-nam"))
    (error "dispatch needs callout procedure name format ('d-nam')"))

(define proc-list (shell (string-append
   "columns -f '" (get "dispatch.d-nam") "' -I8 --spread=1 -S,<<_EOF_\n"
   "invalid\n"
   kwd-list
   "\n_EOF_"
)))

proc-list =];

extern [= dispatch.d-ret =]
disp_[=(. base-enum-name)=](char * str[=
     (if (exist? "dispatch.d-arg")
         (string-append len-arg ", " (get "dispatch.d-arg"))
         len-arg) =]);[=
  ENDIF                         =][=

  IF (not (exist? "no-name"))   =]
extern char const *
[=(. base-enum-name)=]_name([= (. enum-name) =] id);[=

  ENDIF dispatch

=]
#endif /* [=(. header-guard)    =] */
[= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  C CODE
;;;
;;;=][=
== c

=]
 */
#include "[=
    (if move-output-file
        (out-move (string-append base-file-name ".c")))
    header-file                 =]"[=
  IF (exist? "no-length")       =]
#include <ctype.h>
#include <string.h>[=
  ELIF (or (exist? "no-case") (exist? "alias")) =]
#include <ctype.h>[=
  ENDIF                         =][=

  INVOKE run-gperf              =][=
  INVOKE mk-finder              =][=

  IF (exist? "dispatch")        =][=
    INVOKE mk-dispatch          =][=
  ENDIF dispatch                =][=

  IF (not (exist? "no-name"))   =][=
    INVOKE mk-enum2name         =][=
  ENDIF dispatch                =][=

ESAC  suffix c/h

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;=][=

DEFINE mk-finder   =]

/**
 * Convert a command (keyword) to a [= (. enum-name) =] enumeration value.[=
   IF (exist? "no-length")      =]
 * The length of the command is computed by calling \a strspn()
 * on the input argument.[=
   ENDIF                        =][=
   IF (exist? "equate")         =]
 * Within the keyword, the following characters are considered equivalent:
 *   \a [=(. cmd-chars)     =][=
   ENDIF                        =]
 *
 * @param[in] str  a string that should start with a known key word.[=
   IF (not (exist? "no-length"))=]
 * @param[in] len  the length of the keyword at \a str.[=
   ENDIF                        =]
 * @returns the enumeration value.
 * If not found, that value is [=(. invalid-kwd)=].
 */
[= (. enum-name) =]
find_[=(. base-enum-name)=]_id(char const * str[=(. len-arg)=])
{[=

  IF (exist? "no-length")       =]
    static char const accept[] =
        [= (kr-string cmd-chars) =];
    unsigned int len = strspn(str, accept);[=
  ENDIF no-length

=]
    [= (. base-enum-name) =]_map_t const * map;[=
  IF (or (exist? "no-case") (exist? "equate"))   =][=
    INVOKE cvt-chars            =][=

  ENDIF                         =][=
  IF (exist? "alias")           =]
    switch (*str) {[=
    FOR alias                   =]
    case '[= (define kwd (get "alias"))
             (substring kwd 0 1)=]': return [=
             (set! kwd (shellf "echo '%s' | sed 's/. *//'" kwd))
             (string-append PFX-STR "_KWD_"
                   (string->c-name! (string-upcase! kwd))) =];[=
    ENDFOR alias                =]
    default: break;
    }[=
  ENDIF   alias                 =]
    map = find_[=(. base-enum-name)=]_name(str, len);
    return (map == NULL) ? [=(. invalid-kwd)=] : map->[=(. pfx-str)=]_id;
}
[=

ENDDEF mk-finder

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; Convert one character to canonical form.  If there are equated characters
;;; or case mappings to do, do it here.
;;;
;;;=][=

DEFINE cvt-chars

=]
    char name_buf[[=(shell "echo $max_cmd_len")=]];
    unsigned int ix;

    if (  (len < [=(shell "echo $min_cmd_len")=])
       || (len > sizeof(name_buf))
       || isalnum((unsigned)str[len]))
        return [=(. invalid-kwd)=];

    for (ix = 0; ix < len; ix++) {
        int ch = (unsigned char)str[ix];[=
  IF (exist? "equate")          =]
        switch (ch) {
        [= (shell (string-append
             "echo '" (get "equate") "' | "
               "sed \"s/\\(.\\)/case '\\1': /g\""
        )) =]
            name_buf[ix] = '[= (substring (get "equate") 0 1) =]';
            break;
        default:[=

    IF (exist? "no-case")       =]
            name_buf[ix] = tolower(ch);[=
    ELSE                        =]
            name_buf[ix] = ch;[=
    ENDIF                       =]
            break;
        }[=

  ELSE                          =]
        name_buf[ix] = tolower(ch);[=
  ENDIF no-case/equate          =]
    }
    str = name_buf;[=

ENDDEF cvt-chars

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;=][=

DEFINE mk-dispatch

=]
/**
 * Dispatch a [=(. base-enum-name)=] function, based on the keyword.
 *
 * @param[in] str  a string that should start with a known key word.[=
   IF (not (exist? "no-length"))=]
 * @param[in] len  the length of the keyword at \a str.[=
   ENDIF =]
 * @returns [= dispatch.d-ret =], returned by the dispatched function.
 */
[= dispatch.d-ret =]
disp_[=(. base-enum-name)=](char * str[=
    (if (not (exist? "dispatch.d-ret"))
        (error "dispatch needs callout procedure return type ('d-ret')"))

    (if (exist? "dispatch.d-arg")
        (string-append len-arg ", " (get "dispatch.d-arg"))
        len-arg) =])
{
    static [=(. base-enum-name)=]_handler_t * const dispatch[] = {
[= (. proc-list) =] };
    [= (. enum-name) =] id = find_[=(. base-enum-name)=]_id(str[=
        (if (exist? "no-length") "" ", len") =]);
    static unsigned int keywd_len[] = {
[= (shell (string-append
   "{ echo 0 ; g='" kwd-list "' ; for f in $g ; do echo ${#f} ; done } | "
      "columns -I8 -S, --end=' };'"
   )) =]
    unsigned int len = [=
  IF (exist? "alias") =](! isalnum((unsigned)*str)) ? 1 : [=
  ENDIF  =]keywd_len[id];

    [=
    (if (= (get "dispatch.d-ret") "void") "" "return ")
    =]dispatch[id](id, str + len[=

    IF (exist? "dispatch.d-arg") =], [=

       (out-push-new) \=]
      ( set -f
        set -- [= dispatch.d-arg =]
        for f in $*
        do case "$f" in
           ( *, ) printf '%s ' "$f" ;;
           esac
        done
        eval echo "\${$#}"
      )[= (shell (out-pop #t)) =][=

    ENDIF  dispatch.d-arg  =]);
}
[=

ENDDEF mk-dispatch

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  Make an enum to name converter.  If the input id is valid, we *must*
;;;  find an associated name.  The table is compact and starts with 0.
;;;
;;;=][=

DEFINE mk-enum2name

=]
/**
 * Convert an [= (. enum-name) =] value into a string.
 *
 * @param[in] id  the enumeration value
 * @returns the associated string, or "* UNDEFINED *" if \a id
 * is out of range.
 */
char const *
[=(. base-enum-name)=]_name([= (. enum-name) =] id)
{
    [=(. base-enum-name)=]_map_t const * tbl = [=(. base-enum-name)=]_table;

    if (id >= [=(. cmd-count)=])
        return "* UNDEFINED *";

    for (;;) {
        if (tbl->[= (. pfx-str) =]_id == id)
            return tbl->[= (. pfx-str) =]_name;
        tbl++;
    }
}
[=

ENDDEF mk-enum2name

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;=][=
DEFINE run-gperf     =]
[=
(define base-out-name (string-substitute base-enum-name "_" "-"))

(define table-fmt (shell (string-append
    "list='" kwd-list "' ; min_cmd_len=99999 max_cmd_len=0
    for f in $list
    do test ${#f} -gt $max_cmd_len && max_cmd_len=${#f}
       test ${#f} -lt $min_cmd_len && min_cmd_len=${#f}
    done
    expr $max_cmd_len + 1" )))

(set!   table-fmt (string-append "%-" table-fmt "s " PFX-STR "_KWD_%s\n"))
(out-push-new)

=][= #

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; GPERF OPTIONS
;;;
;;;\=]
%struct-type
%language=ANSI-C
%includes
%global-table
%omit-struct-type
%readonly-tables
%compare-strncmp

%define slot-name               [= (. pfx-str) =]_name
%define hash-function-name      [=(. base-enum-name)=]_hash
%define lookup-function-name    find_[=(. base-enum-name)=]_name
%define word-array-name         [=(. base-enum-name)=]_table
%define initializer-suffix      ,[=(. cmd-count)=]
[=

(define gperf-opts (out-pop #t))
(out-push-new (string-append tmp-dir "/" base-out-name ".gp"))

\=][= #

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; GPERF DEFINITIONS
;;;
;;;\=]
%{
# if 0 /* gperf build options: */
[= (prefix "// " gperf-opts) =]
# endif

#include "[=(. header-file)=]"
typedef struct {
    char const *    [= (. pfx-str) =]_name;
    [= (. enum-name) =] [= (. pfx-str) =]_id;
} [=(. base-enum-name)=]_map_t;
%}

[= (. gperf-opts) =]

[=(. base-enum-name)=]_map_t;
%%
[=
FOR cmd =][=
 (define kwd (get "cmd"))
 (define tmp-val (string-append
                 (if (exist? "no-case") (string-downcase kwd) kwd) "," ))
 (sprintf table-fmt tmp-val (string-upcase! (string->c-name! kwd))) =][=
ENDFOR \=]
%%
[=

(out-pop)
(out-push-new)

=][= #

;;; = = = = = = = = = = = = = = = = = = =  RUN GPERF
;;;
;;; After running gperf, delete all the "inline" lines, the soure line
;;; numbers, GNUC INLINE hints, attribute inlines, the ASCII text tests and
;;; all the conditional code.  These all check for stuff that is not relevant
;;; here.  Replace all the #define-s with their defined-to values and strip
;;; out the #define-s themselves.  Since we are appending to the output code,
;;; these #define-s interfere with what we need to do.  Finally, we also force
;;; the generated find procedure to be static.  We don't export it.
;;;
\=]
outdir=$PWD
cd ${tmp_dir}

gperf [= (. base-out-name) =].gp | \
    sed -e '/^_*inline$/d' \
        -e '/^#line /d' \
        -e '/GNUC_.*_INLINE/d' \
        -e '/__attribute__.*inline/d' \
        -e '/^#if.*== *32/,/^#endif/d' \
        -e '/^#\(ifdef\|else\|endif$\)/d' \
        -e 's/^\(const [=(. base-enum-name)=]_map_t\)/static inline \1/' \
    > baseline

sedcmd=`
    egrep '^#define ' baseline | \
        while read _ nm val
        do
            echo "s@\\<${nm}\\>@${val}@g"
        done`

sed "/^#define/d;${sedcmd}" baseline[=
(shell (out-pop #t)) =][=

ENDDEF run-gperf    \=]
/* end of [= (out-name)  =] */
