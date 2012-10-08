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
;;;
;;; Additionally, you may specify collections of bits (masks) in terms of
;;; the individual bits.  Define one or more values for "mask" with
;;; contained values for "m-name" and "m-bit" as in:
;;;
;;; mask = {
;;;   m-name = this-is-a-sample;
;;;   m-bit  = flag1, flag4;
;;; };
;;;
;;; and this will result in additional mask(s) in the header containing
;;; bits that are set for the flag1 and flag4 values, e.g.:
;;;
;;;     #define STR2M_2_KWD_FLAG1                 0x0001U
;;;     #define STR2M_2_KWD_FLAG2                 0x0002U
;;;     #define STR2M_2_KWD_FLAG3                 0x0004U
;;;     #define STR2M_2_KWD_FLAG4                 0x0008U
;;;
;;;     /** bits in mask: flag1 flag4 */
;;;     #define STR2M_2_KWD_THIS_IS_A_SAMPLE_MASK 0x0009U
;;;
;;; The produced header will contain a typedef for the mask type.
;;; It will be named after the "base-type-name" with a "_mask_t" suffix.
;;; It will be a fundamental integer type of uint8_t, uint16_t, uint32_t
;;; or uint64_t, depending on what is needed.  This may be overridden
;;; by specifying a value for "mask-type".

=][=

CASE (suffix)                   =][= #

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  H CODE
;;;
;;;=][=
== h                            =][=

INVOKE init-header              =][=

(out-push-new)                  =][=

FOR mask            =][=
  (set! tmp-str (string-append (get "m-name") "_MASK"))
  (set! idx     (string-length tmp-str))
  (if (> idx max-cmd-width) (set! max-cmd-width idx))
  =][=
ENDFOR mask

=]
sfx=U
bits=[=(count "cmd")=]
hex_width=$(( (bits + 3) / 4 ))
(( hex_width < 4 )) && hex_width=4
[=

IF (exist? "mask-type")

=]
mask_type=[= mask-type =]
(( bits > 32 )) && {
    (( bits > 64 )) && die "cannot handle a $bits bit mask"
    sfx=UL
}
[=

ELSE mask type not provided

=]
if (( bits <= 8 ))
then mask_type='uint8_t'
elif (( bits <= 16 ))
then mask_type='uint16_t'
elif (( bits <= 32 ))
then mask_type='uint32_t'
elif (( bits <= 64 ))
then mask_type='uint64_t'
     sfx=UL
else
    die "cannot handle a $bits bit mask"
fi
[=

ENDIF mask type provided/not

=]
hex_fmt=0x%0${hex_width}X${sfx}
def_fmt="#define [=(string-append PFX-STR "_" ENUM-TYPE "_")
=]%-[=(. max-cmd-width)=]s ${hex_fmt}\\n"
[=

;;; * * * * * * * *
;;; Compute above before proceeding with actual output
;;;
(shell (out-pop #t)) (out-push-new)

=]
hdr_file=[=(string-append tmp-dir "/" base-file-name)=].h
src_file=${hdr_file%.h}.c
mask_name=[= (. mask-name) =]

sed '/^#define .*_GUARD/q' ${hdr_file}
printf '#include <sys/types.h>\n#include <inttypes.h>\n\n'

ix=0

declare -u C
exec 4< ${tmp_dir}/commands
while read -u4 n C
do
    printf "$def_fmt" $C $(( 1 << n ))
done
exec 4<&-
[=

FOR mask            =][=
   (set! tmp-str (string-append
       (string-upcase! (string->c-name! (get "m-name"))) "_MASK"))
   (shell "v=0;list='" (string->c-name! (join " " (stack "m-bit"))) "'\n"
         "for f in $list"
         "\ndo eval f=\\${$f} ; (( v |= f )) ; done\n"
         "echo echo\n"
         "echo \"echo '/** bits in mask:' $list '*/'\"\n"
         "printf \"echo '$def_fmt'\" " tmp-str " $v") =][=

ENDFOR mask

=]
line='[=(join " " (stack "cmd"))=] '

cat <<- _EOF_
	#define [=(. PFX-STR)=]_MAX_MASK_NAME_SIZE ${#line}
	typedef $mask_type ${mask_name};

	extern ${mask_name}
	[=(. base-type-name) =]_str2mask(char const * str, ${mask_name} old);
[=

IF (not (exist? "no-name"))   =]
	extern size_t
	[=(. base-type-name)=]_mask2str([=(. mask-name)
	=] mask, char * buf, size_t len);
[=

ENDIF no name

=]
	_EOF_

grep -E '^#endif .*_GUARD ' ${hdr_file}
[=
(emit (shell (out-pop #t)))
=][=

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  C CODE
;;;
;;;=][=
== c

=][=
(out-move (string-append base-file-name ".c"))
(out-push-new) \=]
exec 4< ${tmp_dir}/[=(. base-file-name)=].c
while IFS='' read -u4 line
do
    echo "$line"
    case "$line" in
    '#include '* ) break ;;
    esac
done

sed '1,/^#define.*_GUARD/d;s/^extern /static /;/^#endif.*_GUARD/,$d' \
    ${tmp_dir}/[=(. base-file-name)=].h

sed 's/^[=(. enum-name)=]$/static [=(. enum-name)=]/
     s/^char const \*$/static char const */
     /end of .*\.c/d' <&4
exec 4<&-

[=
(shell (out-pop #t))
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
[= (string-append mask-name "\n" base-type-name)
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
        val = find_[=(. base-type-name)=]_id(str, val_len);
        if (val == [=(. enum-count)=])
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
[=(. base-type-name)=]_mask2str([=(. mask-name)=] mask, char * buf, size_t len)
{
    [=(. enum-name)=] val = ([=(. enum-name)=])0;
    size_t res = 0;
    if (buf == NULL) len = 0;

    for (; mask != 0; val++, mask >>= 1) {
        char const * p;
        size_t l;

        if (val >= [=(. enum-count)=])
            break;

        if ((mask & 1) == 0)
            continue;

        p = [=(. base-type-name)=]_name(val);
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
/* end of [= (out-name) =] */
[=

DEFINE init-header                  =][=

(if (exist? "dispatch")
    (error "bit masks do not have dispatching available"))

(if (exist? "alias")
    (error "bit name aliases are not allowed"))

(define enum-type "bit")            =][=

INCLUDE "str2init.tlib"             =][=

(out-move ".STR2MASK-SET-ASIDE")
(out-push-new (string-append tmp-dir "/" base-file-name ".def"))
(define bit-enum-type
    (string-append (if (== enum-type "bit") "" enum-type) "BNM"))
(define enum-count (string-append PFX-STR "_COUNT_"
       (string-upcase bit-enum-type)))
(define assign-vals "")
=]
AutoGen Definitions str2enum;
prefix  = '[=(. pfx-str)=]';
type    = '[=(. bit-enum-type)=]';
invalid-name = '[=(. invalid-name)=]';
invalid-val  = '';
[=

FOR cmd     =][=
    (set! tmp-str (get "cmd"))
    (set! idx (for-index))
    (ag-fprintf 0 "cmd[%u] = '%s';\n" idx tmp-str)
    (set! tmp-str (string-downcase! (string->c-name! tmp-str)))
    (set! idx (ash 1 idx))
    (set! assign-vals (string-append assign-vals
          tmp-str "=" (number->string idx) "\n"))
            =][=
ENDFOR      =][=

(out-pop)
(shell assign-vals
"{ ${AGexe} -L" (dirname (tpl-file #t)) " ${tmp_dir}/" base-file-name ".def"
" || die 'Could not build enumeration'\n"
"cp " base-file-name ".[ch] ${tmp_dir}/.\n"
"rm -f " base-file-name ".[ch]\n} 1>&2")
(out-move (string-append base-file-name ".h"))

=][=

ENDDEF init-header

\=]
