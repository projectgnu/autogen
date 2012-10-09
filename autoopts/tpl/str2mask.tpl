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
;;;     #define PFX_KWD_FLAG1                 0x0001U
;;;     #define PFX_KWD_FLAG2                 0x0002U
;;;     #define PFX_KWD_FLAG3                 0x0004U
;;;     #define PFX_KWD_FLAG4                 0x0008U
;;;
;;;     /** bits in mask: flag1 flag4 */
;;;     #define PFX_KWD_THIS_IS_A_SAMPLE_MASK 0x0009U
;;;     #define PFX_KWD_MASK_ALL              0x000FU
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

INVOKE sizes-n-formats          =][=

(out-push-new)

=]
hdr_file=[=(string-append tmp-dir "/" base-file-name)=].h
src_file=${hdr_file%.h}.c
mask_name=[= (. mask-name) =]

sed '/^#define .*_GUARD/q' ${hdr_file}
cat <<- _EOF_
	#include <sys/types.h>
	#include <inttypes.h>
[=
FOR add-on-text                 =][=
  IF (= (get "ao-file") "mask-header") =]
[= ao-text                      =][=
  ENDIF correct type            =][=
ENDFOR add-on-text              =]
	_EOF_

echo "/** bits defined for ${mask_name} */"
ix=0

declare -u C
exec 4< ${tmp_dir}/commands
all_mask=0
while read -u4 n C
do
    v=$(( 1 << n ))
    printf "$def_fmt" $C $v
    (( all_mask += v ))
done
exec 4<&-

emit_mask_def() {
    declare v=0
    declare mname=${1}
    shift
    declare which_bits='in'
    $INVERT && which_bits='omitted from'
    printf "\n/** bits $which_bits ${mname%_MASK} mask:\n"
    echo $* | tr ' ' '\n' | columns --spread=1 -I' *  ' --end=' */'

    for f in $*
    do  case "$f" in
        -* )
            eval f=\${val_${f#-}}
            (( v &= ~f ))
            ;;

        *  )
            eval f=\${val_$f}
            (( v |= f ))
            ;;
        esac
    done
    $INVERT && (( v ^= all_mask ))
    printf "$def_fmt" ${mname} $v
    eval $(echo val_${mname}_mask=$v | tr 'A-Z' 'a-z')
}
[=

FOR mask            =][=
   (set! tmp-str (string-append
       (string-upcase! (string->c-name! (get "m-name"))) "_MASK"))
   (string-append "INVERT=" (if (exist? "m-invert") "true" "false")
   " emit_mask_def " tmp-str " "
      (string->c-name! (join " " (stack "m-bit"))) "\n") =][=

ENDFOR mask

\=]
printf "\n/** all bits in ${mask_name} masks */\n"
printf "$def_fmt" MASK_ALL $all_mask
line='[=(join " " (stack "cmd"))=] '
cat <<- _EOF_

	/** buffer size needed to hold all bit names for ${mask_name} masks */
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
	#ifdef UINT64_MASK_INTERFACE
	extern uint64_t
	[=(. base-type-name)
		=]_str2mask64(char const * str, uint64_t old);

	extern size_t
	[=(. base-type-name)
		=]_mask64_2str(uint64_t mask, char * buf, size_t len);
	#endif

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
}
#ifdef UINT64_MASK_INTERFACE
uint64_t
[=(. base-type-name)=]_str2mask64(char const * str, uint64_t old)
{
    return [=(. base-type-name)=]_str2mask(str, ([=(. mask-name)=])old);
}
#endif[=

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
}
#ifdef UINT64_MASK_INTERFACE
size_t
[=(. base-type-name)=]_mask64_2str(uint64_t mask, char * buf, size_t len)
{
    return [=(. base-type-name)=]_mask2str(([=(. mask-name)=])mask, buf, len);
}
#endif[=
ENDIF dispatch=][=

  FOR add-on-text               =][=
    IF (= (get "ao-file") "mask-code") =]
[= ao-text                      =][=
    ENDIF correct type          =][=
  ENDFOR add-on-text            =][=

ESAC  suffix c/h

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;; Create the function that converts the name into an mask value.
;;;
;;;=]
/* end of [= (out-name) =] */
[=

DEFINE sizes-n-formats              =][=
(out-push-new)                      =][=

FOR mask                            =][=
  (set! tmp-str (string-append (get "m-name") "_MASK"))
  (set! idx     (string-length tmp-str))
  (if (> idx max-cmd-width) (set! max-cmd-width idx))
  =][=
ENDFOR mask

=]
sfx=U
bits=[=(. bit-count)=]
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

(shell (out-pop #t))                =][=

ENDDEF sizes-n-formats

;;; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;;;
;;;  Initialize for the header
;;;
;;;=][=

DEFINE init-header                  =][=

(if (exist? "dispatch")
    (error "bit masks do not have dispatching available"))

(if (exist? "alias")
    (error "bit name aliases are not allowed"))

(define enum-type "bit")            =][=

INCLUDE "str2init.tlib"             =][=

(out-move ".Str2Mask-Set-Aside")
(define bit-enum-type
    (string-append (if (== enum-type "bit") "" enum-type) "bnm"))
(define enum-count (string-append PFX-STR "_COUNT_"
       (string-upcase bit-enum-type)))
(define assign-vals "")
(out-push-new (string-append tmp-dir "/" base-file-name ".def"))
=]
AutoGen Definitions str2enum;
prefix  = '[=(. pfx-str)=]';
type    = '[=(string-downcase bit-enum-type)=]';
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
          "val_" tmp-str "=" (number->string idx) "\n"))
            =][=
ENDFOR      =][=

(out-pop)
(shell assign-vals
"{ ${AGexe} -L" (dirname (tpl-file #t)) " ${tmp_dir}/" base-file-name ".def"
" || die 'Could not build enumeration\n'"
      "\"`cat ${tmp_dir}/" base-file-name ".def`\"\n"
"cp " base-file-name ".[ch] ${tmp_dir}/.\n"
"rm -f " base-file-name ".[ch]\n} 1>&2")
(out-move (string-append base-file-name ".h"))

=][=

ENDDEF init-header

\=]
