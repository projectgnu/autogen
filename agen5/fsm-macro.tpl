[= AutoGen5 Template

#  AutoGen copyright 1992-2006 Bruce Korb

=][=

(define tmp-text   "")
(define offset-names "")
(define next-offset 0)
(define text-offset 0)  =][=

DEFINE add-text-string  =][=
  (set! text-offset next-offset)
  (set! offset-names (string-append offset-names
        (sprintf "#define %s%s_off %d\n" Pfx (get "name") text-offset)  ))
  (set! tmp-text (get "text"))
  (set! next-offset (+ next-offset (string-length tmp-text) 1))
  (c-string tmp-text) =] "\0"[=

ENDDEF add-text-string  =][=

DEFINE emit-invalid-msg =]
tSCC z[=(. Pfx)=]Strings[] =
       [= INVOKE add-text-string  name = Bogus  text = "** OUT-OF-RANGE **" =]
       [= INVOKE add-text-string  name = FsmErr
       text = "FSM Error:  in state %d (%s), event %d (%s) is invalid\n" =]
       [= INVOKE add-text-string  name = EvInvalid  text = invalid =]
       [= INVOKE add-text-string  name = StInit     text = init =][=

  FOR   state   =]
       [= INVOKE add-text-string
       name = (string-append "St" (string-capitalize! (get "state")))
       text = (string-downcase! (get "state")) =][=
  ENDFOR state  =][=

  FOR   event   =]
       [= INVOKE add-text-string
       name = (string-append "Ev" (string-capitalize! (get "event")))
       text = (if (exist? (get "event"))
                     (get (get "event"))
                     (string-downcase! (get "event"))  ) =][=
  ENDFOR event  =];

[=(. offset-names)=]

static const size_t asz[=(. Pfx)=]States[] = {
[=(shellf
"${CLexe-columns} --spread=1 -I4 -S, -f'%sSt%%s_off' <<'_EOF_'
Init
%s
_EOF_"
  Pfx
  (string-capitalize! (join "\n" (stack "state")))  )=] };

static const size_t asz[=(. Pfx)=]Events[] = {
[=(shellf
"${CLexe-columns} --spread=1 -I4 -S, -f'%sEv%%s_off' <<'_EOF_'
%s
Invalid
_EOF_"
  Pfx
  (string-capitalize! (join "\n" (stack "event")))  )=] };

#define [=(. PFX)=]_EVT_NAME(t) ( (((unsigned)(t)) >= [=(. PFX)=]_EV_INVALID) \
    ? z[=(. Pfx)=]Strings : z[=(. Pfx)
    =]Strings + asz[=(. Pfx)=]Events[t])

#define [=(. PFX)=]_STATE_NAME(s) ( (((unsigned)(s)) > [=(. PFX)=]_ST_INVALID) \
    ? z[=(. Pfx)=]Strings : z[=(. Pfx)
    =]Strings + asz[=(. Pfx)=]States[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
int
[=(. pfx)=]_invalid_transition( te_[=(. pfx)=]_state st, te_[=
  (. pfx)=]_event evt )
{
[=(extract fsm-source "    /* %s == INVALID TRANS MSG == %s */" ""
  (sprintf
"    fprintf( stderr, z%2$sStrings + %2$sFsmErr_off, st, %1$s_STATE_NAME(st),
             evt, %1$s_EVT_NAME(evt));"
     PFX Pfx ) )=]

    return EXIT_FAILURE;
}
[=

ENDDEF  emit-invalid-msg  =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE emit-cookie-args   =][=
  FOR cookie              =]
    [=cookie=],[=
  ENDFOR                  =][=
ENDDEF emit-cookie-args   =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-callback     =][=

  CASE cb-name =][=
  =  noop      =][=
  *            =]
static te_[=(. pfx)=]_state
[=cb_prefix=]_[=cb_name=]([= emit-cookie-args =]
    te_[=(. pfx)=]_state initial,
    te_[=(. pfx)=]_state maybe_next,
    te_[=(. pfx)=]_event trans_evt )
{
[= (extract fsm-source (string-append
"/*  %s == " (string-tr! (get "cb_name") "a-z_-" "A-Z  ") " == %s  */" )
""
(if (= (get "cb-name") "invalid")
    (sprintf "    exit( %s_invalid_transition( initial, trans_evt ));" pfx)
    "    return maybe_next;" )) =]
}
[=
  ESAC =][=
ENDDEF build-callback

# # # # # # # =][=

DEFINE run-callback

=]
    if (pT != NULL)
        nxtSt = (*pT)( [=
  FOR cookie =][=
      (shellf "echo '%s'|sed 's,.*[ \t],,'" (get "cookie")) =], [=

  ENDFOR     =][=(. pfx)=]_state, nxtSt, trans_evt );[=

ENDDEF run-callback

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-switch    =]
    case [=cb_prefix=]_[=cb_name=]:[=

  IF (== (get "cb_name") "NOOP") =]  break;[=
  ELSE =]
[=

(set! example-code (if (= (get "cb_name") "invalid") (sprintf
      "        exit( %1$s_invalid_transition( %1$s_state, trans_evt ));" pfx)
      (string-append "        nxtSt = HANDLE_" (get "cb_name") "();")  ))

(extract fsm-source
     (string-append "        /* %s == " (get "cb_name") " == %s */")
     "" example-code )    =]
        break;

[=ENDIF=][=

ENDDEF build-switch

# # # # # # # =][=

DEFINE run-switch         =][=

(define example-code "")  =]

    switch (trans) {[=
  `set -- \`sed 's/,//' .fsm.xlist\`` =][=

  WHILE `echo $#`         =][=

    invoke build-switch
      cb_prefix = (string-append PFX "_TR")
      cb_name   = (shell "echo $1 ; shift") =][=

  ENDWHILE  echo $#  =]
    default:
[=(extract fsm-source "        /* %s == BROKEN MACHINE == %s */" ""
           (string-append "        exit( " pfx "_invalid_transition( "
           pfx "_state, trans_evt ));" ))=]
    }[=

ENDDEF run-switch

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE preamble

=][=

  (if (== (suffix) "c")
      (begin
        (set! fsm-source ".fsm.code")
        (set-writable) )
      (begin
        (define pfx     (string->c-name! (string-downcase!
                        (if (exist? "prefix") (get "prefix") (base-name))  )))
        (define PFX     (string-upcase pfx))
        (define Pfx     (string-capitalize pfx))
        (define t-trans (string-append "t_" pfx "_transition"))
        (define fsm-source ".fsm.head") )
  )
  (dne " *  " "/*  ")  =]
 *
 *  Automated Finite State Machine
 *
 *  Copyright (c) 2001-2006  by  Bruce Korb
 *
[=(bsd "AutoFSM" "Bruce Korb" " *  ")=]
 */[=

ENDDEF  preamble

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE compute-transitions     =][=

;;; Initialize every possible transition as invalid
;;;
(define tr_name (if (=* (get "method") "call")
                (string-append "&" pfx "_do_invalid")
                (string-append PFX "_TR_INVALID")  ))
(shellf
  "ev_list='%s' ; st_list='INIT %s'
   for f in $ev_list ; do for g in $st_list
   do  eval FSM_TRANS_${g}_${f}=\"'{ %s_ST_INVALID, %s }'\"
       export FSM_TRANS_${g}_${f}
   done ; done"

   (string-upcase! (join " " (stack "event")))
   (string-upcase! (join " " (stack "state")))
   PFX tr_name )

(define tev  "")
(define tst   "")
(define ttype "")
(define next  "")
(define proc-ptr-type
  (lambda (tp)
    (if (= tp "noop") "NULL"
        (string-downcase! (string-append "&" pfx "_do_" tp))  )))
=][=#
;;; Now replace the initial values with proper ones gotten from
;;; the trasition definitions.
;;;
;;; It is actually possible to have multiple specifications for a
;;; single state/event pair, however the last state/event assignment
;;; will supply the value for the transition table.  Different
;;; transitions may also specify the same transition method.  For
;;; that, we unique sort the list and eliminate dups that way.  The
;;; unique-ified list is used to produce the callout table.
;;;
=][=
FOR   transition              =][=

   IF (== (get "tst") "*")    =][=

     ;;  This transition applies for all states
     ;;
     (set! tev (string-upcase! (get "tev")))
     (set! ttype (if (exist? "ttype") (get "ttype")
                     (string-append "${f}_" tev)  ))

     (set! tr_name (if (=* (get "method") "call")
                   (proc-ptr-type ttype)
                   (string-upcase!   (string-append PFX "_TR_" ttype))  ))
     (set! next (if (exist? "next") (string-upcase! (get "next")) "${f}"))

     (shellf
       "for f in ${st_list} ; do F=${f}
       eval FSM_TRANS_${f}_%s=\"'{ %s_ST_%s, %s }'\"
       done"
       tev PFX next tr_name ) =][=

   ELIF (== (get "tev") "*")  =][=

     ;;  This transition applies for all transitions in a certain state
     ;;
     (set! tst  (string-upcase! (get "tst")))
     (set! ttype (if (exist? "ttype")
           (string-upcase! (get "ttype")) (string-append tst "_${f}")  ))

     (set! tr_name (if (=* (get "method") "call")
                   (proc-ptr-type ttype)
                   (string-append PFX "_TR_" ttype)  ))

     (set! next (if (exist? "next") (string-upcase! (get "next")) tst))

     (shellf
       "for f in ${ev_list} ; do
       eval FSM_TRANS_%s_${f}=\"'{ %s_ST_%s, %s }'\"
       done"
       tst PFX next tr_name)  =][=

   ELSE                       =][=

     FOR tst                  =][=

       (set! tst  (string-upcase! (get "tst")))
       (set! next (if (exist? "next") (string-upcase! (get "next")) tst))

                              =][=
       FOR tev                =][=

         (set! tev   (string-upcase! (get "tev")))
         (set! ttype (string-downcase! (if (exist? "ttype") (get "ttype")
                     (string-append tst "_" tev)  )))

         (set! tr_name (if (=* (get "method") "call")
                   (proc-ptr-type ttype)
                   (string-upcase!   (string-append PFX "_TR_" ttype))  ))

         (shellf "FSM_TRANS_%s_%s=\"{ %s_ST_%s, %s }\""
          tst tev PFX next tr_name) =][=

       ENDFOR  tev            =][=
     ENDFOR    tst            =][=
   ENDIF tst or ttkn as '*'   =][=
ENDFOR   transition           =][=

(define trans-ct
  (shellf
    "env | egrep '^FSM_TRANS_' | \
    sed '/, NULL }/d;s/^.*%s//;s/ .*$/,/' | \
    sort -u > .fsm.xlist
    echo `wc -l < .fsm.xlist` "
    (if (=* (get "method") "call")
        (string-append pfx "_do_")
        (string-append PFX "_TR_"))
) )                           =][=

ENDDEF compute-transitions    =]
