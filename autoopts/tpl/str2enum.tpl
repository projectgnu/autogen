[= AutoGen5 Template h c -*- Mode: Scheme -*-

# This file contains the templates used to generate
# keyword parsing code

=][= (dne " *  " "/*  ") =][=

CASE (suffix)            =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                     =][=

   (make-tmp-dir)        =]
 *
 * Keyword Dispatcher
 *
[=(out-push-new)\=]
mk_char_list() {
    echo "$1" | sed 's/\(.\)/\1\
/g'
}
[=

(shell (out-pop #t))
(define base-name (string-downcase! (string->c-name! (get "base-name"))))

(if (= 0 (string-length base-name))
    (error "no 'base-name' specified"))

(define pfx-str    "")
(define idx         0)
(define kwd-max-len 0)
(define kwd-min-len 9999)
(define tmp-str    "")

(if (exist? "prefix")
    (set! pfx-str (get "prefix"))
    (begin
       (set! idx (string-index base-name (string->char-set "_-^")))
       (if (number? idx)
           (set! pfx-str (substring/copy base-name 0 idx))
           (set! pfx-str base-name)
)   )  )
(define PFX-STR (string-upcase pfx-str))
(define enum-name (string-append base-name "_enum_t"))
(define len-arg   (if (exist? "no-length") ""
                  ", unsigned int len"))

(define keyword-chars (shell (string-append
  "mk_char_list '" (join "" (stack "keyword")) "'"
)))

(if (exist? "no-case")
    (set! keyword-chars (string-append
          (string-downcase keyword-chars) "\n"
          (string-upcase keyword-chars) )) )
(set! keyword-chars (shell (string-append
    "echo '" keyword-chars "' | sort -u | tr -d '\n\t '" )))

(if (exist? "equate")
    (set! keyword-chars (string-append keyword-chars (get "equate")))
)

(license-description "mbsd" "str2enum" " * " "Bruce Korb")
=]
 */
[=(make-header-guard "str2enum")=]

typedef enum {
    [= (define invalid-kwd (string-append PFX-STR "_INVALID_KWD"))
       invalid-kwd =][=
    FOR keyword  =],
    [=(. PFX-STR) =]_KWD_[=
       (set! tmp-str (get "keyword"))
       (set! idx (string-length tmp-str))
       (if (> idx kwd-max-len) (set! kwd-max-len idx))
       (if (< idx kwd-min-len) (set! kwd-min-len idx))

       (string-upcase! (string->c-name! (get "keyword"))) =][=
    ENDFOR       =],
    [= (define keyword-count (string-append PFX-STR "_COUNT_KWD"))
       keyword-count =]
} [= (. enum-name) =];

extern [= (. enum-name)         =]
find_[=(. base-name)=]_id(char const * str[=(. len-arg)=]);
[=

  IF (exist? "dispatch")        =]
typedef [= dispatch.d-ret =]([=(. base-name)=]_handler_t)(
    [=
    (define disp-arg-list (string-append enum-name " id, char const * str"))
    (if (exist? "dispatch.d-arg")
        (set! disp-arg-list (string-append disp-arg-list ", "
                            (get "dispatch.d-arg") )) )
    disp-arg-list =]);

[=(. base-name)=]_handler_t
[=
(if (not (exist? "dispatch.d-nam"))
    (error "dispatch needs callout procedure name format ('d-nam')"))

(define proc-list (shell (string-append
   "columns -f '" (get "dispatch.d-nam") "' -I8 --spread=1 -S,<<_EOF_\n"
   (string->c-name! (join "\n" "invalid" (stack "keyword")))
   "\n_EOF_"
)))

proc-list =];

extern [= dispatch.d-ret =]
disp_[=(. base-name)=](char * str[=
     (if (exist? "dispatch.d-arg")
         (string-append len-arg ", " (get "dispatch.d-arg"))
         len-arg) =]);[=
  ENDIF                         =][=

  IF (not (exist? "no-name"))   =]
extern char const *
[=(. base-name)=]_name([= (. enum-name) =] id);[=

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
#include "[=(. header-file)=]"[=
IF (exist? "no-length")         =]
#include <ctype.h>
#include <string.h>[=
ELIF (exist? "no-case")         =]
#include <ctype.h>[=
ENDIF                           =]
[= INVOKE run-gperf             =]

/**
 * Convert a keyword to a [= (. enum-name) =] enumeration value.[=
   IF (exist? "no-length")      =]
 * The length of the keyword is computed by calling \a strspn() 
 * on the input argument.[=
   ENDIF                        =][=
   IF (exist? "equate")         =]
 * Within the keyword, the following characters are considered equivalent:
 *   \a [=(. keyword-chars)     =][=
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
find_[=(. base-name)=]_id(char const * str[=(. len-arg)=])
{[=

  IF (exist? "no-length")       =]
    static char const accept[] =
        [= (kr-string keyword-chars) =];
    unsigned int len = strspn(str, accept);[=
  ENDIF no-length

=]
    [= (. base-name) =]_map_t const * map;[=
  IF (or (exist? "no-case") (exist? "equate"))   =][=
    INVOKE cvt-chars            =][=

  ENDIF                         =]
    map = find_[=(. base-name)=]_name(str, len);
    return (map == NULL) ? [=(. invalid-kwd)=] : map->[=(. pfx-str)=]_id;
}
[=

  IF (exist? "dispatch")        =][=
    INVOKE mk-dispatch          =][=
  ENDIF dispatch                =][=

  IF (not (exist? "no-name"))   =][=
    INVOKE mk-enum2name         =][=
  ENDIF dispatch                =][=

ESAC  suffix c/h

;;; = = = = = = = = = = = = = = = = = = =
;;;
;;; Convert one character to canonical form.  If there are equated characters
;;; or case mappings to do, do it here.
;;;
;;;=][=

DEFINE cvt-chars

=]
    char name_buf[[=(. kwd-max-len)=]];
    unsigned int ix;

    if (  (len < [=(. kwd-min-len)=])
       || (len > [=(. kwd-max-len)=])
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
 * Dispatch a [=(. base-name)=] function, based on the keyword.
 *
 * @param[in] str  a string that should start with a known key word.[=
   IF (not (exist? "no-length"))=]
 * @param[in] len  the length of the keyword at \a str.[=
   ENDIF =]
 * @returns [= dispatch.d-ret =], returned by the dispatched function.
 */
[= dispatch.d-ret =]
disp_[=(. base-name)=](char * str[=
     (if (exist? "dispatch.d-arg")
         (string-append len-arg ", " (get "dispatch.d-arg"))
         len-arg) =])
{
    static [=(. base-name)=]_handler_t * const dispatch[] = {
[= (. proc-list) =] };
    [= (. enum-name) =] id = find_[=(. base-name)=]_id(str[=
        (if (exist? "no-length") "" ", len") =]);
    static unsigned int keywd_len[] = {
[= (shell (string-append
   "{ echo 0 ; for f in " (join " " (stack "keyword")) 
   "  ; do echo ${#f} ; done } | columns -I8 -S, --end=' };'"
   )) =]

    [=
    (if (not (exist? "dispatch.d-ret"))
        (error "dispatch needs callout procedure return type ('d-ret')"))

    (if (= (get "dispatch.d-ret") "void") "" "return ")
    =]dispatch[id](id, str + keywd_len[id][=

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
[=(. base-name)=]_name([= (. enum-name) =] id)
{
    [=(. base-name)=]_map_t const * tbl = [=(. base-name)=]_table;

    if (id >= [=(. keyword-count)=])
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
DEFINE run-gperf     =][=

(define base-out-name (string-substitute base-name "_" "-"))
(define table-fmt (sprintf "%u" (+ 1 kwd-max-len)))
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
%define hash-function-name      [=(. base-name)=]_hash
%define lookup-function-name    find_[=(. base-name)=]_name
%define word-array-name         [=(. base-name)=]_table
%define initializer-suffix      ,[=(. keyword-count)=]
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
} [=(. base-name)=]_map_t;
%}

[= (. gperf-opts) =]

[=(. base-name)=]_map_t;
%%
[=
FOR keyword =][=
 (define kwd (string->c-name! (get "keyword")))
 (define tmp-val (string-append
                 (if (exist? "no-case") (string-downcase kwd) kwd) "," ))
 (sprintf table-fmt tmp-val (string-upcase! kwd)) =][=
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
        -e 's/^\(const [=(. base-name)=]_map_t\)/static inline \1/' \
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
