[= AutoGen5 Template  -*- Mode: text -*-

  h

=][=
(define cch-ct 0)
(dne " *  " "/*  ") =]
 *
 *  This file handles all the bookkeeping required for tracking all the little
 *  tiny strings used by the AutoOpts library.  There are [= (count "utxt") =]
 *  of them.  This is not versioned because it is entirely internal to the
 *  library and accessed by client code only in a very well-controlled way:
 *  they may substitute translated strings using a procedure that steps through
 *  all the string pointers.
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
FOR utxt        =][=
  (if (exist? "ut-type")
      (emit (sprintf "\n  %-9s utpz_%s;"
                     (string-append (get "ut-type") "*")
                     (get "ut-name")  ))
      (set! cch-ct (+ cch-ct 1))  ) =][=
  
ENDFOR  utxt    =]
  cch_t*    apz_str[ [= (. cch-ct) =] ];
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

(set! cch-ct 0)
(define typed-list "")
(define const-list "")  =][=

FOR utxt        =]
#define z[= (sprintf "%-20s" (get "ut-name"))
        =] (option_usage_text.[=

  IF (exist? "ut-type") =]utpz_[= ut-name =][=
       (set! typed-list (string-append typed-list "\n" (get "ut-name"))) =][=
  ELSE no "ut-type"     =]apz_str[[=
       (emit cch-ct) (set! cch-ct (+ cch-ct 1))
       (set! const-list (string-append const-list "\n" (get "ut-name"))) =]][=
  ENDIF                 =])[=
ENDFOR =]

  /*
   *  First, set up the strings.  Some of these are writable.  These are all in
   *  English.  This gets compiled into libopts and is distributed here so that
   *  xgettext (or equivalents) can extract these strings for translation.
   */
[=
FOR utxt =]
  static [= (sprintf "%-7s"
              (if (exist? "ut-type") (get "ut-type") "cch_t"))
          =] eng_z[= ut-name =][] =
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
  "${CLexe:-columns} -W84 -I4 --spread=1 -f'eng_z%s,' <<_EOF_" typed-list
  "\n_EOF_" )) =]
    {
[= (shell (string-append
  "${CLexe:-columns} -W84 -I6 --spread=1 -S, -f'eng_z%s' <<_EOF_" const-list
  "\n_EOF_" )) =]
    }
  };

#endif /* DO_TRANSLATIONS */
#endif /* [= (. header-guard) =] */
