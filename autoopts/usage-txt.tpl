[= AutoGen5 Template  -*- Mode: C -*-

  h

=][=

(dne " *  " "/*  ") =]
 *
 *  This file handles all the bookkeeping required for tracking
 *  all the little tiny strings used by the AutoOpts library.
 *  There are [= (count "utxt") =] of them.
 *
[= (lgpl "AutoOpts" "Bruce Korb" " *  ") =]
 */
[= (make-header-guard "autoopts") =]

#undef  cch_t
#define cch_t const char

/*
 *  One structure to hold all the pointers to all the stringlets.
 */
typedef struct {
  int       field_ct;[=
FOR utxt =]
  [=
   ;; A few of these are modifiable, though most are not.  That's why
   ;; we can't use an array
   ;;
   (sprintf "%-9s" (string-append (get "ut-type") "*"))
  =] utpz_[= ut-name =];[=
ENDFOR =]
} usage_text_t;

/*
 *  Declare the global structure with all the pointers to translated
 *  strings.  This is then used by the usage generation procedure.
 */
extern usage_text_t option_usage_text;

#if defined(AUTOOPTS_INTERNAL) /* DEFINE ALL THE STRINGS = = = = = */
/*
 *  Provide a mapping from a short name to fields in this structure.
 */[=
FOR utxt  =]
#define z[= (sprintf "%-20s" (get "ut-name"))
        =] option_usage_text.utpz_[= ut-name =][=
ENDFOR =]

  /*
   *  First, set up the strings.  Some of these are writable.  These are all in
   *  English.  This gets compiled into libopts and is distributed here so that
   *  xgettext (or equivalents) can extract these strings for translation.
   */
[=
FOR utxt =]
  static  [= (sprintf "%-7s" (get "ut-type")) =] eng_z[= ut-name =][] =
          [= (kr-string (get "ut-text")) =];[=
ENDFOR =]

  /*
   *  Now, define (and initialize) the structure that contains
   *  the pointers to all these strings.
   *  Aren't you glad you don't maintain this by hand?
   */
  usage_text_t option_usage_text = {
    [= (count "utxt") =],
[= (shell (string-append
  "${CLexe:-columns} -W84 -I4 --spread=1 -S, -f'eng_z%s' <<_EOF_\n"
     (join "\n" (stack "utxt.ut-name"))
  "\n_EOF_" )) =]
  };

#endif /* DO_TRANSLATIONS */
#endif /* [= (. header-guard) =] */
