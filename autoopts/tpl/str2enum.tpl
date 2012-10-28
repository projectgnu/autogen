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

=][=

CASE (suffix)                   =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                            =][=
INVOKE init-header              =]
#include <sys/types.h>
#include <inttypes.h>

typedef enum {[=
    (if (> enum-val-offset 0)
        (string-append "\n    " invalid-cmd " = 0,") ) =][=
    (shellf "mk_enum_list %d" enum-val-offset) =]
    [= (. cmd-count) =][=
    (if (= (get "invalid-val") "~0")
        (string-append ",\n    " invalid-cmd " = ~0") ) =]
} [= (. enum-name) =];[=

  FOR add-on-text               =][=
    IF (= (get "ao-file") "enum-header") =]
[= ao-text                      =][=
    ENDIF correct type          =][=
  ENDFOR add-on-text            =][=

  IF (not (exist? "no-code"))   =]

extern [= (. enum-name)         =]
[=(. find-func-name)=](char const * str[=(. len-arg)=]);[=

    IF (exist? "dispatch")      =]

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

(shell "mk_proc_list") =];

extern [= dispatch.d-ret =]
disp_[=(. base-type-name)=](char * str[=
     (if (exist? "dispatch.d-arg")
         (string-append len-arg ", " (get "dispatch.d-arg"))
         len-arg) =]);[=

    ENDIF  dispatch exists      =][=

    IF (not (exist? "no-name")) =]

extern char const *
[=(. base-type-name)=]_name([= (. enum-name) =] id);[=

    ENDIF dispatch

=]
[=ENDIF (not (exist? "no-code"))=]
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
#include "[= ;;"
    (if (exist? "no-code") (out-delete))
    (if move-output-file
        (out-move (string-append base-file-name ".c")))
    header-file                 =]"[=#"=][=
  (. extra-c-incl)              =][=

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

DEFINE init-header              =][=

(define computed-length #f)
(define return-length   #f)
(define tmp-str         "")
(define extra-c-incl    "")
(define len-arg         ", size_t len")
(define enum-type       "cmd")  =][=

INCLUDE "str2init.tlib"         =][=
  CASE length                   =][=
  !E                            =][=
    (if (or (exist? "no-case") (exist? "alias"))
      (set! extra-c-incl "\n#include <ctype.h>")) =][=

  = provided                    =][=
    (if (or (exist? "no-case") (exist? "alias"))
      (set! extra-c-incl "\n#include <ctype.h>")) =][=

  = returned                    =][=
    (set! extra-c-incl "\n#include <ctype.h>\n#include <string.h>")
    (set! len-arg         ", size_t * len")
    (set! computed-length #t)
    (set! return-length   #t)   =][=

  *                             =][=
    (set! extra-c-incl "\n#include <ctype.h>\n#include <string.h>")
    (set! computed-length #t)
    (set! len-arg "")           =][=
  ESAC                          =][=

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
equate_to=`echo "$equate_from" | sed "s#.#[=(substring equate-from 0 1)=]#g"`
[= ENDIF =]
mk_enum_list() {
    chmod a+w ${tmp_dir}/commands
    sort -o ${tmp_dir}/commands ${tmp_dir}/commands
    exec 3< ${tmp_dir}/commands
    declare n
    declare -u c
    declare fmt='\n    [=(string-append PFX-STR ENUM-TYPE)
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
    declare -l c
    {
        echo [=(. invalid-name)=]
        while read -u3 _ c
        do
            echo $c
        done
    } | columns -f '[= dispatch.d-nam =]' -I8 --spread=1 -S,
    exec 3<&-
}
mk_proc_dispatch() {
    exec 3< ${tmp_dir}/commands
    declare -l c
    declare -u C
    declare fmt=$(printf '\n        [[=(string-append PFX-STR ENUM-TYPE)
                =]_%%-%us] = ' ${max_cmd_width})"[= dispatch.d-nam =],"

    while read -u3 _ c
    do
        C=$c
        printf "$fmt" $C $c
    done | sed '$s/,$/ };/'

    exec 3<&-
}
mk_lengths() {
    exec 3< ${tmp_dir}/commands
    declare n=`expr ${max_cmd_width} + 1`
    declare -u c
    declare fmt="[[=(string-append PFX-STR ENUM-TYPE)
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

DEFINE mk-finder                =]

/**
 * Convert a command (keyword) to a [= (. enum-name) =] enumeration value.[=
   IF (. computed-length)       =]
 * The length of the command is computed by calling \a strspn()
 * on the input argument.[=
   ENDIF                        =][=
   IF (exist? "equate")         =]
 * Within the keyword, the following characters are considered equivalent:
 *   \a [=(. cmd-chars)     =][=
   ENDIF                        =]
 *
 * @param[in] str   a string that should start with a known key word.[=

   CASE (out-push-new)
        (. len-arg)             =][=
   *==  "size_t len"            =]
 * @param[in] len   the length of the keyword at \a str.[=
   *==  "size_t * len"          =]
 * @param[out] len  the length of the keyword found at \a str.[=
   ESAC len-arg                 =][=
   (define len-descrip (out-pop #t))
   len-descrip

=]
 * @returns the enumeration value.
 * If not found, that value is [=(. invalid-cmd)=].
 */
[= (. enum-name) =]
[=(. find-func-name)=](char const * str[=(. len-arg)=])
{[=
  IF (exist? "alias")           =]
  switch (*str) {[=
    FOR alias                   =]
  case '[= (define cmd (get "alias"))
           (substring cmd 0 1)=]': return [=
           (set! cmd (shellf "echo %s" (substring cmd 1)))
           (string-append PFX-STR ENUM-TYPE "_"
                   (string->c-name! (string-upcase! cmd))) =];[=
    ENDFOR alias                =]
  default:
    if (! isalpha((unsigned)*str))
      return [=(. invalid-cmd)=];
    break;
  }

  {[=
  ENDIF   alias                 =]
    [= (. base-type-name) =]_map_t const * map;[=

  IF (define len-param-name (if computed-length "clen" "len"))
     (define check-length   (if return-length "\n    *len = clen;" "\n"))
     computed-length            =]
    static char const accept[] =
        [= (set! check-length (string-append
        check-length
        "\n    if (isalnum((unsigned)str[clen]))"
        "\n        return " invalid-cmd ";" ))

        (kr-string cmd-chars)
        =];
    unsigned int clen = strspn(str, accept);[=
  ENDIF computed-length

=][=
  IF (or (exist? "no-case") (exist? "equate"))   =][=
    INVOKE cvt-chars            =][=
  ELSE  =][= (. check-length)   =][=
  ENDIF                        \=]

    map = find_[=(. base-type-name)=]_name(str, [= (. len-param-name) =]);[=

  IF (not (exist? "partial"))   =]
    return (map == NULL) ? [=(. invalid-cmd)=] : map->[=(. enum-field)=];[=
  ELSE                          =][=
    INVOKE find-part-match      =][=
  ENDIF                         =][=
  (if (exist? "alias") "\n  }") =]
}
[=

ENDDEF mk-finder

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; Convert one character to canonical form.  If there are equated characters
;;; or case mappings to do, do it here.
;;;
;;;=][=

DEFINE cvt-chars                =][=

(define min-len (if (and (exist? "partial") (> min-cmd-width 2))
                2 min-cmd-width))

=]
    char name_buf[[=(+ 1 max-cmd-width)=]];
    unsigned int ix;

    /* too short, too long or followed immediately by a name char... */
    if (  ([=(sprintf "%s < %u" len-param-name min-len)=])
       || ([=(sprintf "%s > %u" len-param-name max-cmd-width)=])
       || isalnum((unsigned)str[[=(. len-param-name)=]]) )
        return [=(. invalid-cmd)=];

    for (ix = 0; ix < [=(. len-param-name)=]; ix++) {
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
  (if return-length (string-append "\n    *len = clen;"))
  =][=

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
   (. len-descrip) =]
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
    static [=(. base-type-name)=]_handler_t * const dispatch[] = {[=
        (shell "mk_proc_dispatch") =]

    static unsigned int keywd_len[] = {[=
        (shell "mk_lengths") =] };

    [= (. enum-name) =] id = [=(. find-func-name)=](str[=
        (if (> (string-length len-arg) 0) ", len" "") =]);
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
;;;  Search through the alphabetized indexes for a unique partial match.
;;;
;;;=][=

DEFINE find-part-match          =]
    if (map != NULL)
        return map->[=(. enum-field)=];
    {
        static unsigned int const ix_map[] = {
[=

(out-push-new)

=]
    sed 's/:.*//' ${tmp_dir}/table  | \
    while read ix
    do
        echo $(( ix - 1 ))
    done | columns --spread=1 -S, -I12 --end ' };'
[=

(shell (out-pop #t))

=][= (define cmp-call (string-append
        "strncmp(map->" name-field ", str, " len-param-name ")" )) =]
        [= (. enum-name) =] res = [=(. invalid-cmd)=];
        static int const HI = (sizeof(ix_map) / sizeof(ix_map[0])) - 1;
        int lo = 0;
        int hi = HI;
        int av;
        int cmp;

        for (;;) {
            av  = (hi + lo) / 2;
            map = [=(. base-type-name)=]_table + ix_map[av];
            cmp = [=(. cmp-call)=];
            if (cmp == 0) break;
            if (cmp > 0)
                 hi = av - 1;
            else lo = av + 1;
            if (lo > hi)
                return [=(. invalid-cmd)=];
        }
        res = map->[=(. enum-field)=];
        if (av < HI) {
            map = [=(. base-type-name)=]_table + ix_map[av + 1];
            if ([=(. cmp-call)=] == 0)
                return [=(. invalid-cmd)=];
        }
        if (av > 0) {
            map = [=(. base-type-name)=]_table + ix_map[av - 1];
            if ([=(. cmp-call)=] == 0)
                return [=(. invalid-cmd)=];
        }
        return res;
    }[=

ENDDEF find-part-match

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
    static char const undef[] = "[=
        (if insert-undef "* UNDEFINED *" undef-str) =]";
    static char const * const nm_table[] = {
[=

(out-push-new)

=]
exec 4< ${tmp_dir}/table
while IFS='' read -u4 line
do
    str=${line%\"*}
    line=${line%'}'*}
    line=${line##*,}
    echo "[${line}] = \"${str#*\"}\""
done | columns -I8 -S, --spread=1 --end ' };'
[=

(shell (out-pop #t))

=]
    char const * res = undef;
    if (id < [=(. cmd-count)=]) {
        res = nm_table[id];
        if (res == NULL)
            res = undef;
    }
    return res;
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
                "s " PFX-STR ENUM-TYPE "_%s\n" ))
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

%define slot-name               [=
(define name-field (string-append pfx-str "_name")) name-field =]
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
    char const *    [=(. name-field)=];
    [= (. enum-name) =] [= (define enum-field (string-append pfx-str "_id"))
                       enum-field =];
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

sed -n -e '1,/_table\[\] =/d' \
       -e '/^ *{$/d' \
       -e '/^ *};/q' \
       -e 's/^ *//' \
       -e 's/}, {/},\
{/' \
       -e p baseline | \
    grep -n -v '""'  | \
    sort -t: -k2     > table

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
