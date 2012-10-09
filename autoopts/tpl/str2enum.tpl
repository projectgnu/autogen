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

;;   partial      Accept unique partial matches of at least two characters.
;;   base-name    The base name is normally derived from the definitions
;;                file name.  This will override that and cause the
;;                output files to have this "base name".
;;   prefix       By default, the enumeration value uses the first segment
;;                of the base name as the prefix for each name.  This
;;                overrides that.
;;   type         After the prefix, each enum value has a "type" segment.
;;                By default, this is "cmd", but may be specified with this.
;;   invalid-val  Invalid enumeration value.  By default, 0 (zero).
;;                Alternatively, you may select "~0" to be the invalid value
;;                or specify it to be empty.  Then there is no specific
;;                invalid value, but the count value may be used.
;;   invalid-name By default, "invalid", but you may also select "unused" or
;;                any other string that suits your fancy.
;;   alias        A special character can be used to represent a command. e.g.
;;                   alias = "< source"; alias = "# comment"; alias = "? help";
;;                will cause a '<' character to represent "source",
;;                a '#' to represent comment, and '?' for help.
;;   undef-str    by default, the display string for an undefined value is
;;                "* UNDEFINED *".  Use this to change that.
;;   equate       A series of punctuation characters considered equivalent.
;;                Typically, "-_" but sometimes (Tandem) "-_^".
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
;;                <PREFIX>_CMD_<COMMAND> of type <base_name>_enum_t
;;                plus two extra entries: <PREFIX>_INVALID_CMD, and
;;                <PREFIX>_COUNT_CMD.

;;                External declarations of the  generated
;;                procedure(s).

;;   <basename>.c The implementing code

;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

;; =][=

CASE (suffix)                   =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                            =][=
INVOKE init-header              =][= DEBUG =][=

  FOR add-on-text               =][=
    IF (= (get "ao-file") "enum-header") =]
[= ao-text                      =][=
    ENDIF correct type          =][=
  ENDFOR add-on-text            =]

typedef enum {[=
    (if (> enum-val-offset 0)
        (string-append "\n    " invalid-cmd " = 0,") ) =][=
    (shellf "mk_enum_list %d" enum-val-offset) =]
    [= (. cmd-count) =][=
    (if (= (get "invalid-val") "~0")
        (string-append ",\n    " invalid-cmd " = ~0") ) =]
} [= (. enum-name) =];

extern [= (. enum-name)         =]
find_[=(. base-type-name)=]_id(char const * str[=(. len-arg)=]);[=

  IF (exist? "dispatch")        =]

typedef [= dispatch.d-ret =]([=(. base-type-name)=]_handler_t)(
    [=
    (define disp-arg-list (string-append enum-name " id, char const * str"))
    (if (exist? "dispatch.d-arg")
        (set! disp-arg-list (string-append disp-arg-list ", "
                            (get "dispatch.d-arg") )) )
    disp-arg-list =]);

[=(. base-type-name)=]_handler_t
[=
(if (not (exist? "dispatch.d-nam"))
    (error "dispatch needs callout procedure name format ('d-nam')"))

(define proc-list (shell "mk_proc_list"))

proc-list =];

extern [= dispatch.d-ret =]
disp_[=(. base-type-name)=](char * str[=
     (if (exist? "dispatch.d-arg")
         (string-append len-arg ", " (get "dispatch.d-arg"))
         len-arg) =]);[=

  ENDIF  dispatch exists        =][=

  IF (not (exist? "no-name"))   =]

extern char const *
[=(. base-type-name)=]_name([= (. enum-name) =] id);[=

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

  FOR add-on-text               =][=
    IF (= (get "ao-file") "enum-code") =]
[= ao-text                      =][=
    ENDIF correct type          =][=
  ENDFOR add-on-text            =][=

ESAC  suffix c/h

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;; Create the function that converts the name into an enum value.
;;;
;;;=][=

DEFINE init-header      =][=

(define enum-type "cmd")=][=
INCLUDE "str2init.tlib" =][=

(define tmp-str     "")
(define len-arg     (if (exist? "no-length") ""
                        ", unsigned int len"))

(define equate-from "")
(define equate-to   "")
(out-push-new)

=]
mk_char_list() {
    echo "$1" | sed 's/\(.\)/\1\
/g' | sort -u | tr -d '\n\t '
}

chmod a+w ${tmp_dir}/commands
readonly max_cmd_width=[=(. max-cmd-width)=]
readonly min_cmd_width=[=(. min-cmd-width)=]
[= IF (exist? "equate") =]
equate_from='[=(define equate-from (get "equate")) equate-from=]'
equate_to=`echo "$equate_from" | sed 's/\(.\).*/\1/'`
equate_to=`echo "$equate_from" | tr "$equate_from" "$equate_to"`
[= ENDIF =]
mk_enum_list() {
    chmod a+w ${tmp_dir}/commands
    sort -o ${tmp_dir}/commands ${tmp_dir}/commands
    exec 3< ${tmp_dir}/commands
    declare n
    declare -u c
    declare fmt='\n    [=(string-append PFX-STR "_" ENUM-TYPE)
                =]_'"%-${max_cmd_width}s = %s,"
    while read -u3 n c
    do
        printf "$fmt" $c `expr $n + $1`
    done
    exec 3<&-
}[=

IF (exist? "alias")

=]
mk_all_cmds() {
    chmod a+w ${tmp_dir}/commands
    echo "$1" | while IFS='' read a
    do
        echo ${a#?}
    done | sed 's/[^a-zA-Z0-9]/_/g' >> ${tmp_dir}/commands
    sort -u -o ${tmp_dir}/commands ${tmp_dir}/commands
    cat ${tmp_dir}/commands
}[=

ENDIF alias exists =][=

IF (exist? "dispatch")

=]
mk_proc_list() {
    exec 3< ${tmp_dir}/commands
    declare n
    declare -l c
    {
        echo invalid
        while read -u3 n c
        do
            echo $c
        done
    } | columns -f '[= dispatch.d-nam =]' -I8 --spread=1 -S,
    exec 3<&-
}
mk_lengths() {
    exec 3< ${tmp_dir}/commands
    declare n=`expr ${max_cmd_width} + 1`
    declare -u c
    declare fmt="[[=(string-append PFX-STR "_" ENUM-TYPE)
                =]_%-${n}s ="
    while read -u3 n c
    do
        printf '\n        '"$fmt ${#c}," "$c]"
    done | sed '$s/,$//'

    exec 3<&-
}[=

ENDIF dispatch exists

=][=

(shell (out-pop #t))
(if (exist? "equate")
    (set! equate-to (shell "echo \"${equate_to}\"")) )
(define cmd-chars (join "" (stack "cmd")))

(if (exist? "no-case")
    (set! cmd-chars (string-append
          (string-downcase cmd-chars) (string-upcase cmd-chars) )) )

(if (exist? "equate")
    (set! cmd-chars (string-append cmd-chars (get "equate")))
)

(set! cmd-chars (shell "mk_char_list '" cmd-chars "'" ))
(emit "\n *\n * Command/Keyword Dispatcher\n */\n")
(make-header-guard "str2enum")

=][=

ENDDEF init-header

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; Create the function that converts the name into an enum value.
;;;
;;;=][=

DEFINE mk-finder                =][=

  IF (or (exist? "partial") (not (exist? "no-name"))) =]
[=
    (define str-table-name (string-append pfx-str "_names"))
    (string-table-new str-table-name)

    (out-push-new)
    (if insert-undef
        (emit (string-table-add-ref str-table-name undef-str) "\n") )
    (define start-ix (if (or insert-undef (= enum-val-offset 0)) 0 1))
    =][=

    FOR cmd (for-from start-ix) (for-by 1) (for-sep "\n")
       =][=
       (set! tmp-str (if (found-for?) (get "cmd") undef-str))
       (string-table-add-ref str-table-name tmp-str)
       =][=
    ENDFOR                      =][=

    (out-suspend "main")
    (emit-string-table str-table-name)
    (emit "\nstatic char const * " pfx-str "_name_table[" cmd-count "] = {\n")
    (out-resume "main")
    (shell "columns -I8 -S, --spread=1 --end=' };' <<_EOF_\n"
        (out-pop #t) "\n_EOF_" )=][=
  ENDIF  partial or ! no-name   =]

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
 * If not found, that value is [=(. invalid-cmd)=].
 */
[= (. enum-name) =]
find_[=(. base-type-name)=]_id(char const * str[=(. len-arg)=])
{[=
  IF (exist? "alias")           =]
  switch (*str) {[=
    FOR alias                   =]
  case '[= (define cmd (get "alias"))
           (substring cmd 0 1)=]': return [=
           (set! cmd (shellf "echo %s" (substring cmd 1)))
           (string-append PFX-STR "_" ENUM-TYPE "_"
                   (string->c-name! (string-upcase! cmd))) =];[=
    ENDFOR alias                =]
  default:
    if (! isalpha((unsigned)*str))
      return [=(. invalid-cmd)=];
    break;
  }

  {[=
  ENDIF   alias                 =][=

  IF (exist? "no-length")       =]
    static char const accept[] =
        [= (kr-string cmd-chars) =];
    unsigned int len = strspn(str, accept);[=
  ENDIF no-length

=]
    [= (. base-type-name) =]_map_t const * map;[=
  IF (or (exist? "no-case") (exist? "equate"))   =][=
    INVOKE cvt-chars            =][=

  ENDIF                         =]
    map = find_[=(. base-type-name)=]_name(str, len);[=
  IF (not (exist? "partial")) =]
    return (map == NULL) ? [=(. invalid-cmd)=] : map->[=(. pfx-str)=]_id;[=
  ELSE  =]
    if (map != NULL)
        return map->[=(. pfx-str)=]_id;
    if (len < 2)
        return [=(. invalid-cmd)=];
    {
        [= (. enum-name) =] res = [=(. invalid-cmd)=];
        int ix = 0;

        while (++ix < [=(. cmd-count)=]) {
            if (strncmp([=(. pfx-str)=]_name_table[ix], str, len) == 0) {
                if (res != [=(. invalid-cmd)=])
                    return [=(. invalid-cmd)=];
                res = ([= (. enum-name) =])ix;
            }
        }
        return res;
    }[=
  ENDIF =][=
  (if (exist? "alias") (emit "\n  }")) =]
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
    char name_buf[[=(+ 1 max-cmd-width)=]];
    unsigned int ix;

    /* too short, too long or followed immediately by a name char... */
    if (  (len <  [=(. min-cmd-width)=])
       || (len >= [=(. max-cmd-width)=])
       || isalnum((unsigned)str[len]) )
        return [=(. invalid-cmd)=];

    for (ix = 0; ix < len; ix++) {
        int ch = (unsigned char)str[ix];[=
  IF (exist? "equate")          =]
        switch (ch) {
        [= (shell "echo '" (get "equate") "' | "
               "sed \"s/\\(.\\)/case '\\1': /g\"") =]
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
 * Dispatch a [=(. base-type-name)=] function, based on the keyword.
 *
 * @param[in] str  a string that should start with a known key word.[=
   IF (not (exist? "no-length"))=]
 * @param[in] len  the length of the keyword at \a str.[=
   ENDIF =]
 * @returns [= dispatch.d-ret =], returned by the dispatched function.
 */
[= dispatch.d-ret =]
disp_[=(. base-type-name)=](char * str[=
    (if (not (exist? "dispatch.d-ret"))
        (error "dispatch needs callout procedure return type ('d-ret')"))

    (if (exist? "dispatch.d-arg")
        (string-append len-arg ", " (get "dispatch.d-arg"))
        len-arg) =])
{
    static [=(. base-type-name)=]_handler_t * const dispatch[] = {
[= (. proc-list) =] };
    [= (. enum-name) =] id = find_[=(. base-type-name)=]_id(str[=
        (if (exist? "no-length") "" ", len") =]);
    static unsigned int keywd_len[] = {[=
        (shell "mk_lengths") =] };
    unsigned int klen = [=
  IF (exist? "alias") =](! isalnum((unsigned)*str)) ? 1 : [=
  ENDIF  =]keywd_len[id];

    [=
    (if (= (get "dispatch.d-ret") "void") "" "return ")
    =]dispatch[id](id, str + klen[=

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
 * @returns the associated string, or "[=(. undef-str)=]" if \a id
 * is out of range.
 */
char const *
[=(. base-type-name)=]_name([= (. enum-name) =] id)
{
    if (id >= [=(. cmd-count)=])
        return [= (if insert-undef str-table-name
            (string-append "\"" undef-str "\"") ) =];
    return [=(. pfx-str)=]_name_table[id];
}
[=

ENDDEF mk-enum2name

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;=][=
DEFINE run-gperf     =]
[=
(define table-fmt (string-append
                "%-" (number->string (+ max-cmd-width 1))
                "s " PFX-STR "_" ENUM-TYPE "_%s\n" ))
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
%define hash-function-name      [=(. base-type-name)=]_hash
%define lookup-function-name    find_[=(. base-type-name)=]_name
%define word-array-name         [=(. base-type-name)=]_table
%define initializer-suffix      ,[=(. cmd-count)=]
[=

(define gperf-opts (out-pop #t))
(out-push-new (string-append tmp-dir "/" base-file-name ".gp"))

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
} [=(. base-type-name)=]_map_t;
%}

[= (. gperf-opts) =]

[=(. base-type-name)=]_map_t;
%%
[=
FOR cmd =][=
 (define cmd (get "cmd"))
 (if (exist? "no-case")
     (set! cmd (string-downcase cmd)) )
 (if (exist? "equate")
     (set! cmd (string-tr! cmd equate-from equate-to)) )

 (define tmp-val (string-append
                 (if (exist? "no-case") (string-downcase cmd) cmd) "," ))

 (sprintf table-fmt tmp-val (string-upcase! (string->c-name! cmd))) =][=
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

gperf [= (. base-file-name) =].gp | \
    sed -e '/^_*inline$/d' \
        -e '/^#line /d' \
        -e '/GNUC_.*_INLINE/d' \
        -e '/__attribute__.*inline/d' \
        -e '/^#if.*== *32/,/^#endif/d' \
        -e '/^#\(ifdef\|else\|endif$\)/d' \
        -e 's/^\(const [=(. base-type-name)=]_map_t\)/static inline \1/' \
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
