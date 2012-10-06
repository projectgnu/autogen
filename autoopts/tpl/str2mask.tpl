[= AutoGen5 Template h c -*- Mode: Scheme -*-

=][= #

;;; This file contains the templates used to generate
;;; keyword bitmask code.  It uses the str2enum template to start,
;;; but places some restrictions upon it:
;;;
;;; * names are constrained to alphanumerics and the underscore
;;; * aliases are not allowed
;;; * dispatch procedures are not allowed
;;;
;;; Two procedures are produced for constructing a mask from a string
;;; and to produce a string of bit names from a mask.  The latter,
;;; however, is bypassed if "no-name" is specified.  See str2enum.tpl.

=][=

CASE (suffix)                   =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                            =][=

INVOKE init-header              =][=

(out-push-new)                  =]
hdr_file=[=(. base-file-name)=].h
mask_typ=[= (. mask-name) =]

sed '/_enum_t;$/q;/#define.*_GUARD/a\
#include <sys/types.h>\
#include <inttypes.h>' ${hdr_file}
echo

sfx=U
bits=[= (+ (count "cmd") (count "alias")) =]

if (( bits <= 8 ))
then mask_type='uint8_t'
elif (( bits <= 16 ))
then mask_type='uint16_t'
elif (( bits <= 32 ))
then mask_type='uint32_t'
elif (( bits <= 64 ))
then mask_type='uint64_t'
     sfx=ULL
else
    die "cannot handle a $bits bit mask"
fi

res="\\1_BIT_\\2"
exp='^ *\([A-Z0-9_]*\)_CMD_\([A-Z0-9_]*\),'
sed -n "/_CMD_/{
	s/$exp/$res/
	p
	}" ${hdr_file} > ${tmp_dir}/bits

exec 4< ${tmp_dir}/bits
width=0
while read -u4 line
do
    (( ${#line} > width )) && width=${#line}
done

def_fmt="#define %-${width}s (1${sfx} << %s)\\n"
ix=0

exec 4< ${tmp_dir}/bits
while read -u4 line
do
    printf "$def_fmt" $line $ix
    (( ix += 1 ))
done
exec 4<&-
line='[=(join " " (stack "cmd"))=] '

sed '1,/_enum_t;$/d;/^#endif.*GUARD/,$d' ${hdr_file}

cat <<- _EOF_

	#define [=(. PFX-STR)=]_MAX_MASK_NAME_SIZE ${#line}
	typedef $mask_type ${mask_typ};

	extern ${mask_typ}
	[=(. base-mask-name) =]_str2mask(char const * str, ${mask_typ} old);
[=IF (not (exist? "no-name"))   =]
	extern size_t
	[=(. base-mask-name)=]_mask2str([=(. mask-name)
	=] mask, char * buf, size_t len);
[=ENDIF dispatch=]
	_EOF_

grep -E '^#endif .*_GUARD ' ${hdr_file}
mv -f [=(. base-file-name)=].c ${tmp_dir}/TMP-C
[=
(emit (shell (out-pop #t)))
(out-move (string-append base-file-name ".h"))
=][=

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  C CODE
;;;
;;;=][=
== c

=][=
(out-move (string-append base-file-name ".c"))
(shell "sed '$d' ${tmp_dir}/TMP-C")
=]

#include <sys/types.h>
#include <string.h>
#ifndef NUL
#define NUL '\0'
#endif

/**
 * Convert a string to a [= (. mask-name) =] mask.
 * Bit names prefixed with a hyphen have the bit removed from the mask.
 * If the string starts with a '-', '+' or '|' character, then
 * the old value is used as a base, otherwise the result mask
 * is initialized to zero.  Separating bit names with '+' or '|'
 * characters is optional.  By default, the bits are "or"-ed into the
 * result.
 *
 * @param[in] str string with a list of bit names
 * @param[in] old previous value, used if \a str starts with a '+' or '-'.
 *
 * @returns an unsigned integer with the bits set.
 */
[= (string-append mask-name "\n" base-mask-name)
=]_str2mask(char const * str, [=(. mask-name)=] old)
{
    static char const white[] = " \t\f";
    static char const name_chars[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789_";

    [=(. mask-name)=] res = 0;
    int have_data = 0;

    for (;;) {
        [=(. enum-name)=] val;
        unsigned int val_len;
        unsigned int invert = 0;

        str += strspn(str, white);
        switch (*str) {
        case NUL: return res;
        case '-':
            invert = 1;
            /* FALLTHROUGH */

        case '+': case '|':
            if (have_data == 0)
                res = old;

            str += 1 + strspn(str + 1, white);
            if (*str == NUL)
                return 0;
        }

        val_len = strspn(str, name_chars);
        if (val_len == 0)
            return 0;
        val = find_[=(. base-mask-name)=]_id(str, val_len);
        if (val-- == [=(string-append PFX-STR "_INVALID_CMD")=])
            return 0;
        if (invert)
            res &= ~(([=(. mask-name)=])1 << val);
        else
            res |= ([=(. mask-name)=])1 << val;
        have_data = 1;
        str += val_len;
    }
}[=

IF (not (exist? "no-name"))   =]

/**
 * Convert a [=(. mask-name)=] mask to a string.
 *
 * @param[in]  mask  the mask with the bits to be named
 * @param[out] buf   where to store the result.  This may be NULL.
 * @param[in]  len   size of the output buffer
 * @results    The full length of the space needed for the result,
 * including the terminating NUL byte.  The actual result will not
 * overwrite \a len bytes at \a buf.  This value will also never
 * exceed [=(. PFX-STR)=]_MAX_MASK_NAME_SIZE.
 */
size_t
[=(. base-mask-name)=]_mask2str([=(. mask-name)=] mask, char * buf, size_t len)
{
    [=(. enum-name)=] val = ([=(. enum-name)=])0;
    size_t res = 0;
    if (buf == NULL) len = 0;

    for (; mask != 0; mask >>= 1) {
        char const * p;
        size_t l;

        if (++val >= [=(string-append PFX-STR "_COUNT_CMD")=])
            break;

        if ((mask & 1) == 0)
            continue;

        p = [=(. base-mask-name)=]_name(val);
        if (*p == '*')
            continue; /* ignore invalid bits */

        l = strlen(p) + 1; /* includes NUL byte or spacer byte */
        if (l <= len) {
            if (res > 0)
                *(buf++) = ' ';
            memcpy(buf, p, l);
            buf += l - 1;
            len -= l;
        }
        res += l;
    }
    return (res == 0) ? 1 : res;
}[=
ENDIF dispatch=][=

ESAC  suffix c/h

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;; Create the function that converts the name into an mask value.
;;;
;;;=]
/* end of [= (out-name)  =] */
[=

DEFINE init-header   =][=

(if (exist? "dispatch")
    (error "bit masks do not have dispatching available"))

(if (exist? "alias")
    (error "bit name aliases are not allowed"))

(make-tmp-dir)
(out-move ".STR2MASK-SET-ASIDE")

(define base-file-name (if (exist? "base-name") (get "base-name") (base-name)))
(define base-mask-name (string->c-name! (string-downcase base-file-name)))
(define pfx-str    "")
(define idx         0)
(define tmp-str    "")

(shell "${AGexe} -L `dirname " (tpl-file #t) "` -Tstr2enum " (def-file))

(if (exist? "prefix")
    (set! pfx-str (string->c-name! (get "prefix")))
    (begin
       (set! idx (string-index base-mask-name (string->char-set "_-^")))
       (if (number? idx)
           (set! pfx-str (substring/copy base-mask-name 0 idx))
           (set! pfx-str base-mask-name)
)   )  )
(define PFX-STR (string-upcase pfx-str))
(define mask-name (string-append base-mask-name "_mask_t"))
(define enum-name (string-append base-mask-name "_enum_t"))

=][=

ENDDEF init-header

\=]
